/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "fight.h"

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int pk_allowed;		/* see config.c */
extern int auto_save;		/* see config.c */
extern int max_exp_gain;	/* see config.c */
extern char *dirs[];		/* see constants.c */
extern char *dirs2[];		/* see constants.c */
extern int rev_dir[];		/* see constants.c */
extern struct char_data *mob_proto;

/* External procedures */
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data * ch);

ACMD(do_flee);
void hit(struct char_data * ch, struct char_data * victim, int type);
void forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
void mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void mprog_fight_trigger(struct char_data * mob, struct char_data * ch);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},		/* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	/* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	/* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"},
  {"attack", "attack"}		/* 15 */
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

/* The Fight related routines */

void appear(struct char_data * ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);

  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}



void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
    perror(buf2);
    exit(1);
  }
  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }


  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}


void update_pos(struct char_data * victim)
{

  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0)
    GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data * ch, struct char_data * vict)
{
  char buf[256];

  if (PLR_FLAGGED(ch, PLR_KILLER) || CAN_MURDER(ch, vict))
	return;
  else {
    SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
    sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
	    GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
  }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data * ch, struct char_data * vict)
{
  if (ch == vict)
    return;

  assert(!FIGHTING(ch));

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);
}


void make_corpse(struct char_data * ch)
{
  struct obj_data *corpse, *o, *next_obj;
  struct obj_data *money;
  int i;
  extern int max_npc_corpse_time, max_pc_corpse_time;

  struct obj_data *create_money(int amount);

  corpse = create_obj();
  corpse->item_number = 0;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");

  sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = str_dup(buf2);

  sprintf(buf2, "the corpse of %s", GET_NAME(ch));
  corpse->short_description = str_dup(buf2);

  corpse->orig_zone = world[ch->in_room].zone;
  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  GET_OBJ_WEAR(corpse) = 0;
  GET_OBJ_EXTRA(corpse) = ITEM_NODONATE;
  GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_RENT(corpse) = 100000;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
  else
    GET_OBJ_TIMER(corpse) = max_pc_corpse_time;

  /* transfer character's inventory to the corpse */
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content)
    o->in_obj = corpse;
  object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      obj_to_obj(unequip_char(ch, i), corpse);

  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_NO_CORPSE)) {
      for (o = corpse->contains; o; o = next_obj) {
	next_obj = o->next_content;
        obj_from_obj(o);
        obj_to_room(o, ch->in_room);
      }
      act("You see the body of $n disintegrate!", FALSE, ch, 0, 0, TO_ROOM);
      extract_obj(corpse);
  } else {
      obj_to_room(corpse, ch->in_room);
  }
}

/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 4;
}



void death_cry(struct char_data * ch)
{
  int door, was_in;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}



void raw_kill(struct char_data * ch, struct char_data *killer)
{
  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

/*  death_cry(ch); */

  if (killer) {
     mprog_death_trigger(ch, killer);
     if (IS_MOB(ch) && PRF_FLAGGED(killer, PRF_AUTOSAC) && (ch->in_room == killer->in_room)) {
	act("You begin the ritual of corpse sacrifice...", FALSE, killer, 0, 0, TO_CHAR);
	act("$n begins the ritual of corpse sacrifice...", FALSE, killer, 0, 0, TO_ROOM);
	SET_BIT(MOB_FLAGS(ch), MOB_NO_CORPSE);
     }
  }
  make_corpse(ch);
  extract_char(ch);
}

void die(struct char_data * ch, struct char_data *killer)
{
  gain_exp(ch, -(GET_EXP(ch) >> 1));
  raw_kill(ch, killer);
}

void perform_group_gain(struct char_data * ch, int base,
			     struct char_data * victim)
{
  int share;

  share = MIN(max_exp_gain, MAX(1, base));

  if (share > 1) {
    sprintf(buf2, "You receive your share of experience -- %d points.\r\n", share);
    send_to_char(buf2, ch);
  } else
    send_to_char("You receive your share of experience -- one measly little point!\r\n", ch);

  gain_exp(ch, share);
  change_alignment(ch, victim);
}


void group_gain(struct char_data * ch, struct char_data * victim)
{
  int tot_members, base;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))
    k = ch;

  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
    tot_members = 1;
  else
    tot_members = 0;

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      tot_members++;

  /* round up to the next highest tot_members */
  base = (GET_EXP(victim) / 3) + tot_members - 1;

  if (tot_members >= 1)
    base = MAX(1, GET_EXP(victim) / (3 * tot_members));
  else
    base = 0;

  if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
    perform_group_gain(k, base, victim);

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      perform_group_gain(f->follower, base, victim);
}



char *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
  static char buf[256];
  char *cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }				/* For */

  return (buf);
}


/* message for doing damage with a weapon */
void dam_message(int dam, struct char_data * ch, struct char_data * victim,
		      int w_type, struct obj_data *obj)
{
  char *buf;
  int msgnum;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

    {
      "$n tries to #w $N with $p, but misses.",	/* 0: 0     */
      "You try to #w $N with $p, but miss.",
      "$n tries to #w you with $p, but misses."
    },

    {
      "$n tickles $N as $e #W $M with $p.",	/* 1: 1..2  */
      "You tickle $N as you #w $M with $p.",
      "$n tickles you as $e #W you with $p."
    },

    {
      "$n barely #W $N with $p.",		/* 2: 3..4  */
      "You barely #w $N with $p.",
      "$n barely #W you with $p."
    },

    {
      "$n #W $N with $p.",			/* 3: 5..6  */
      "You #w $N with $p.",
      "$n #W you with $p."
    },

    {
      "$n #W $N hard with $p.",			/* 4: 7..10  */
      "You #w $N hard with $p.",
      "$n #W you hard with $p."
    },

    {
      "$n #W $N very hard with $p.",		/* 5: 11..14  */
      "You #w $N very hard with $p.",
      "$n #W you very hard with $p."
    },

    {
      "$n #W $N extremely hard with $p.",	/* 6: 15..19  */
      "You #w $N extremely hard with $p.",
      "$n #W you extremely hard with $p."
    },

    {
      "With $p, $n mauls $N to smithereens with $s #w.",	/* 7: 19..23 */
      "With $p, you maul $N to smithereens with your #w.",
      "With $p, $n mauls you to smithereens with $s #w."
    },

    {
      "With $p, $n decimates $N with a brutal #w.",	/* 8: 23..28 */
      "With $p, you decimate $N with a brutal #w.",
      "With $p, $n decimates you with a brutal #w."
    },

    {
      "With a mighty #w, $n devastates $N with $p.",	/* 9: 28..32 */
      "You totally devastate $N with $p.",
      "$n devastates you fully with $p."
    },

    {
      "$n #W $N with $p and maims $M.",	/* 10: 32..36 */
      "You #w $N with $p and maim $M.",
      "$n #W you with $p and maims you."
    },

    {
      "$n #W and mutilates $N with $p.",	/* 11: 36..44 */
      "You #w and mutilate $N with $p.",
      "$n #W and mutilates you with $p."
    },

    {
      "#wing with $p, $n massacres $N to small fragments.",	/* 12: 44..50 */
      "#wing with $p, you massacre $N to small fragments.",
      "#wing with $p, $n massacres you to small fragments."
    },

    {
      "$n's #w with $p critically DEMOLISHES $N.",	/* 13: 50..100 */
      "Your #w with $p critically DEMOLISHES $N.",
      "$n's #w with $p critically DEMOLISHES you."
    },

    {
      "With $p, $n's #w ANNIHILATES $N's vitals.",	/* 14: 100..150 */
      "With $p, your #w ANNIHILATES $N's vitals.",
      "With $p, $n's #w ANNIHILATES your vitals."
    },
 
    {
      "$n OBLITERATES $N with a deadly #w, using $p!!",	/* 15: 150..250   */
      "You OBLITERATE $N with a deadly #w, using $p!!",
      "$n OBLITERATES you with a deadly #w, using $p!!"
    },

    {
      "$n uses $p to DISINTERATE $N with a LETHAL #w!!",	/* 16: > 250   */
      "You use $p to DISINTEGRATE $N with a LETHAL #w!!",
      "$n uses $p to DISINTEGRATE you with a LETHAL #w!!"
    }

  };


  w_type -= TYPE_HIT;		/* Change to base of table with text */

  if (dam == 0)		msgnum = 0;
  else if (dam <= 2)    msgnum = 1;
  else if (dam <= 4)    msgnum = 2;
  else if (dam <= 6)    msgnum = 3;
  else if (dam <= 10)   msgnum = 4;
  else if (dam <= 14)   msgnum = 5;
  else if (dam <= 19)   msgnum = 6;
  else if (dam <= 23)   msgnum = 7;
  else if (dam <= 28)   msgnum = 8;
  else if (dam <= 32)   msgnum = 9;
  else if (dam <= 36)   msgnum = 10;
  else if (dam <= 44)   msgnum = 11;
  else if (dam <= 54)   msgnum = 12;
  else if (dam <= 99)   msgnum = 13;
  else if (dam <= 150)  msgnum = 14;
  else if (dam <= 250) 	msgnum = 15;
  else 			msgnum = 16;

  /* damage message to onlookers */
  buf = replace_string(dam_weapons[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, obj, victim, TO_NOTVICT);

  /* damage message to damager */
  send_to_char(CCYEL(ch, C_CMP), ch);
  buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  if (GET_LEVEL(ch) >= 22) 
	  sprintf(buf, "%s  You do %s%d%s points of damage.", buf, CCRED(ch, C_CMP), dam, CCYEL(ch, C_CMP));
  act(buf, FALSE, ch, obj, victim, TO_CHAR);
  send_to_char(CCNRM(ch, C_CMP), ch);

  /* damage message to damagee */
  send_to_char(CCRED(victim, C_CMP), victim);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  if (GET_LEVEL(ch) >= 18) 
	  sprintf(buf, "%s  You got hit for %s%d%s points of damage.", buf, CCYEL(ch, C_CMP), dam, CCRED(ch, C_CMP));
  act(buf, FALSE, ch, obj, victim, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, struct char_data * ch, struct char_data * vict,
		      int attacktype, struct obj_data *weap)
{
  char buf[256];
  int i, j, nr;
  struct message_type *msg;

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
	if (msg->god_msg.attacker_msg)
		act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	if (msg->god_msg.victim_msg)
		act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	if (msg->god_msg.room_msg)
		act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  if (msg->die_msg.attacker_msg) {
		  send_to_char(CCYEL(ch, C_CMP), ch);
		  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
		  send_to_char(CCNRM(ch, C_CMP), ch);
	  }
	  if (msg->die_msg.victim_msg) {
		  send_to_char(CCRED(vict, C_CMP), vict);
		  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
		  send_to_char(CCNRM(vict, C_CMP), vict);
	  }
	  if (msg->die_msg.room_msg) 
		  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  if (msg->hit_msg.attacker_msg) {
		  send_to_char(CCYEL(ch, C_CMP), ch);
	  	  strcpy(buf, msg->hit_msg.attacker_msg);
	          if (GET_LEVEL(ch) >= 20 && attacktype != 399)
	   	  	sprintf(buf, "%s  You do %s%d%s points of damage.", buf, CCRED(ch, C_CMP), dam, CCYEL(ch, C_CMP));
		  act(buf, FALSE, ch, weap, vict, TO_CHAR);
		  send_to_char(CCNRM(ch, C_CMP), ch);
	  }
	  if (msg->hit_msg.victim_msg) {
		  send_to_char(CCRED(vict, C_CMP), vict);
		  strcpy(buf, msg->hit_msg.victim_msg);
		  if (GET_LEVEL(ch) >= 20 && attacktype != 399) 
			  sprintf(buf, "%s  You take %s%d%s points of damage.", buf, CCYEL(ch, C_CMP), dam, CCRED(ch, C_CMP));
		  act(buf, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
		  send_to_char(CCNRM(vict, C_CMP), vict);
	  }
	  if (msg->hit_msg.room_msg)
		  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	/* Dam == 0 */
	  if (msg->miss_msg.attacker_msg) {
		send_to_char(CCYEL(ch, C_CMP), ch);
		act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
		send_to_char(CCNRM(ch, C_CMP), ch);
	  }
	  if (msg->miss_msg.victim_msg) {
		send_to_char(CCRED(vict, C_CMP), vict);
		act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
		send_to_char(CCNRM(vict, C_CMP), vict);
	  }
	  if (msg->miss_msg.room_msg)
		act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return 1;
    }
  }
  return 0;
}


void damage(struct char_data * ch, struct char_data * victim, int dam,
	    int attacktype, struct obj_data *obj)
{
  int exp;
  bool freeflag = FALSE;

  if (GET_POS(victim) <= POS_DEAD) {
    log("SYSERR: Attempt to damage a corpse.");
    return;			/* -je, 7/7/92 */
  }
  /* You can't damage an immortal! */
  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
    dam = 0;

  /* shopkeeper protection */
  if (!ok_damage_shopkeeper(ch, victim))
    return;

  if (victim != ch) {
    if (GET_POS(ch) > POS_STUNNED) {
      if ((attacktype >= 0) && !(FIGHTING(ch)))
	set_fighting(ch, victim);

      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (FIGHTING(ch))
	  stop_fighting(ch);
	hit(ch, victim->master, TYPE_UNDEFINED);
	return;
      }
    }
    if (attacktype >= 0 && GET_POS(victim) > POS_STUNNED && !FIGHTING(victim)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
	  (GET_LEVEL(ch) < LVL_IMMORT))
	remember(victim, ch);
    }
  }
  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
    appear(ch);

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam >>= 1;		/* 1/2 damage when sanctuary */

/*  if (!pk_allowed) {
    check_killer(ch, victim);
*/

  if ((attacktype >= 0) && (IS_WEAPON(attacktype) || IS_MONK(ch))) {
	if (number(1, 101) <= GET_SKILL(ch, SKILL_ENH_DAMAGE)) {
		dam += lvD5(ch);
	}
  }
  if (AFF_FLAGGED(ch, AFF_DEATHDANCE))
	dam += number(1, dam / 4);
  dam = MAX(MIN(dam, 500), 0);

  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   * 
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */

   if (!obj) {
	exp = MAX(39, MIN(56, attacktype + 40));
	obj = read_object(exp, VIRTUAL, 0);
	obj_to_room(obj, 0);
	exp = 0;
	freeflag = TRUE;
   }
   if (!IS_WEAPON(attacktype))
      skill_message(dam, ch, victim, attacktype, obj);
    else {
      if (GET_POS(victim) == POS_DEAD || dam == 0) {
        if (!skill_message(dam, ch, victim, attacktype, obj))
  	  dam_message(dam, ch, victim, attacktype, obj);
      } else
         dam_message(dam, ch, victim, attacktype, obj);
      if (!CAN_DAMAGE(ch, victim))
	dam = (GET_HIT(victim) == GET_MAX_HIT(victim));
    }
  GET_HIT(victim) -= dam;
  if (ch != victim)
	  gain_exp(ch, MAX(0, (GET_LEVEL(victim) - lvD2(ch)) * dam));

  if (freeflag) {
	extract_obj(obj);
	freeflag = FALSE;
  }

  update_pos(victim);

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You are dead!  Sorry...\r\n", victim);
    break;

  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) >> 2))
      act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim))
	do_flee(victim, "", 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	GET_HIT(victim) < GET_WIMP_LEV(victim)) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
    }
    break;
  }

  if (!IS_NPC(victim) && !(victim->desc)) {
    do_flee(victim, "", 0, 0);
    if (!FIGHTING(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }
  if (!AWAKE(victim))
    if (FIGHTING(victim))
      stop_fighting(victim);

  if (GET_POS(victim) == POS_DEAD) {
    if (IS_NPC(victim) || victim->desc)
      if (IS_AFFECTED(ch, AFF_GROUP))
	group_gain(ch, victim);
      else {
	exp = MIN(max_exp_gain, GET_EXP(victim) / 3);

	/* Calculate level-difference bonus */
	if (IS_NPC(ch))
	  exp += MAX(1, (exp * MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch)))) >> 3);
	else
	  exp += MAX(1, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) >> 3);
	exp = MAX(exp, 1);
	if (exp > 1) {
	  sprintf(buf2, "Horray, you receive %d experience points.\r\n", exp);
	  send_to_char(buf2, ch);
	} else
	  send_to_char("You receive one lousy experience point.\r\n", ch);
	gain_exp(ch, exp);
	change_alignment(ch, victim);
      }
    if (!IS_NPC(victim)) {
      sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
	      world[victim->in_room].name);
      if (IS_NPC(ch))
	victim->player_specials->saved.killed_by_mob++;
      else {
	victim->player_specials->saved.killed_by_player++;
	ch->player_specials->saved.killed_player++;
      }
      mudlog(buf2, BRF, LVL_IMMORT, TRUE);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }
    die(victim, ch);
  }
}



void hit(struct char_data * ch, struct char_data * victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int w_type, victim_ac, calc_thaco, dam = 0, diceroll;

  extern int thaco[NUM_CLASSES][LVL_IMPL+1];
  extern byte backstab_mult[];
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];

  if (ch->in_room != victim->in_room) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  mprog_hitprcnt_trigger(ch, FIGHTING(ch));
  mprog_fight_trigger(ch, FIGHTING(ch));

  if (type == SKILL_DUAL_WIELD) {
	wielded = GET_EQ(ch, WEAR_DUALWIELD);
	type = TYPE_UNDEFINED;
  }
  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else if (wielded && GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON)
    w_type = 5 + TYPE_HIT; /* fireweapons bludgeon in hand-to-hand */
  else {
    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }

  /* Calculate the raw armor including magic armor.  Lower AC is better. */

  if (!IS_NPC(ch))
    calc_thaco = thaco[(int) GET_CLASS_NUM(ch)][(int) GET_LEVEL(ch)];
  else		/* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;

  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5);	/* Intelligence helps! */
  calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);	/* So does wisdom */
  diceroll = number(1, 20);

  victim_ac = GET_AC(victim) / 10;

  if (AWAKE(victim))
    victim_ac += dex_app[GET_DEX(victim)].defensive;

  victim_ac = MAX(-20, victim_ac);	/* -20 is lowest */

  /* decide whether this is a hit or a miss */
  if ((((diceroll < 20) && AWAKE(victim)) &&
       ((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac)))) {
    if (type == SKILL_BACKSTAB)
      damage(ch, victim, 0, SKILL_BACKSTAB, wielded);
    else
      damage(ch, victim, 0, w_type, wielded);
  } else {
    if (number(1, 121) <= GET_SKILL(victim, SKILL_PARRY)) {
	act("$N parries your attack.", FALSE, ch, 0, victim, TO_CHAR);
	act("You parry $n's attack.", FALSE, ch, 0, victim, TO_VICT);
    } else if (AFF2_FLAGGED(victim, AFF2_BLINK) && 
	       (number(1, 50) <= GET_LEVEL(victim))) {
		act("$N blinks away from your attack.", FALSE, ch, 0, victim, TO_CHAR);
		act("$N blinks out of $n's way.", FALSE, ch, 0, victim, TO_NOTVICT);
	        act("You blink out of $n's way.", FALSE, ch, 0, victim, TO_VICT);
    } else if (AFF2_FLAGGED(victim, AFF2_MIRRORIMAGE) &&
	       (number(1, 40) > GET_INT(ch)) && (number(1, 40) <= GET_INT(victim))) {
		act("One of $N's false images dissipates and is instantly replaced!", FALSE, ch, 0, victim, TO_CHAR);
		act("One of $N's false images takes the blow from $n and is instantly replaced!", FALSE, ch, 0, victim, TO_NOTVICT);
	        act("One of your images takes the blow from $n and is replaced by another image.", FALSE, ch, 0, victim, TO_VICT);
    } else {
    /* okay, we know the guy has been hit.  now calculate damage. */
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);

    if (wielded && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON && GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON))
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0);
    else {
      if (IS_NPC(ch)) {
	dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      } else if (IS_MONK(ch)) {
 	dam += number(0, lvD2(ch));
      } else 
	dam += number(0, 2);	/* Max. 2 dam with bare hands */
    }
  
    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    dam = MAX(0, dam);		/* at least 0 hp damage min per hit */

    if (type == SKILL_BACKSTAB) {
      dam *= backstab_mult[(int) GET_LEVEL(ch)];
      damage(ch, victim, dam, SKILL_BACKSTAB, wielded);
    } else
      damage(ch, victim, dam, w_type, wielded);
    }
  }
}

/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  struct char_data *ch;
  extern struct index_data *mob_index;
  unsigned int attacks, dual_wield_flag;

  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;

    attacks = 1;
    dual_wield_flag = 0;

    if (IS_NPC(ch)) 
        attacks = dice(1, MAX(1, ch->mob_specials.attack_num));
    else { /* PC */
	if (number(1, 101) <= GET_SKILL(ch, SKILL_SECOND_ATTACK))
		attacks++;
	if (number(1, 101) <= GET_SKILL(ch, SKILL_THIRD_ATTACK))
		attacks++;
	if ((IS_WARRIOR(ch) && GET_NUM_OF_CLASS(ch) == 1) || IS_AVATAR(ch) ||
	     IS_SAMURAI(ch) || IS_BARBARIAN(ch) || IS_MONK(ch))
		attacks += MAX(0, (lvD8(ch) - 2));
	if (IS_MONK(ch))
		attacks++;
	if (AFF_FLAGGED(ch, AFF_DEATHDANCE) && !AFF2_FLAGGED(ch, AFF2_HASTE))
		attacks += dice(1,lvD3(ch)) + dice(1, lvD3(ch));
        if (AFF2_FLAGGED(ch, AFF2_HASTE))
		attacks *= 2;
	if ( (number(1, 101) <= GET_SKILL(ch, SKILL_DUAL_WIELD)) &&
		(GET_EQ(ch, WEAR_DUALWIELD)) )
		dual_wield_flag = 1;
    }
    attacks = MIN(10, attacks);

    while(attacks-- > 0) {
      if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
        stop_fighting(ch);
        continue;
      }
      if (IS_NPC(ch)) {
        if (GET_MOB_WAIT(ch) > 0) {
  	  GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
  	  continue;
        }
        GET_MOB_WAIT(ch) = 0;
        if (GET_POS(ch) < POS_FIGHTING) {
   	  GET_POS(ch) = POS_FIGHTING;
 	  act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        }
      }
      if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("You can't fight while sitting!!\r\n", ch);
        continue;
      }
      if ((dual_wield_flag == 1) && (attacks < 2)) {
		hit(ch, FIGHTING(ch), SKILL_DUAL_WIELD);
		dual_wield_flag = 0;
      } else
	      hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }
    if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
      (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
  }
}

bool range_hit(struct char_data *ch, struct char_data * victim, struct obj_data *obj)
{
  int w_type = 315, victim_ac, calc_thaco, dam = 0, diceroll;
  extern int thaco[NUM_CLASSES][LVL_IMPL+1];
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];


  if (!IS_NPC(ch))
    calc_thaco = thaco[(int) GET_CLASS_NUM(ch)][(int) GET_LEVEL(ch)];
  else          /* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;

  calc_thaco -= dex_app[GET_DEX(ch)].miss_att;
  calc_thaco -= GET_HITROLL(ch);
  calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5);       /* Intelligence helps! */
  calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);       /* So does wisdom */
  diceroll = number(1, 20);

  victim_ac = GET_AC(victim) / 10;

  if (AWAKE(victim))
    victim_ac += dex_app[GET_DEX(victim)].defensive;
  if (GET_POS(victim) == POS_SITTING)
    victim_ac -= 1;
  if (GET_POS(victim) <= POS_RESTING)
    victim_ac -= 2;
  victim_ac = MAX(-20, victim_ac);      /* -20 is lowest */

  if ((((diceroll < 20) && AWAKE(victim)) &&
       ((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac)))) {
      return FALSE;
  } else {
    if (AFF_FLAGGED(victim, AFF_SHIELD) || number(1, 121) <= GET_SKILL(victim, SKILL_DODGE)) {
        act("$N dodges your attack.", FALSE, ch, 0, victim, TO_CHAR);
        act("You dodge $p.", FALSE, ch, obj, victim, TO_VICT);
        return FALSE;
    } else if (AFF2_FLAGGED(victim, AFF2_BLINK) && 
	       (number(1, 50) <= GET_LEVEL(victim))) {
		act("$N blinks away from your $p.", FALSE, ch, obj, victim, TO_CHAR);
	        act("You blink away from the $p $n launched at you.", FALSE, ch, obj, victim, TO_VICT);
    } else if (AFF2_FLAGGED(victim, AFF2_MIRRORIMAGE) &&
	       (number(1, 40) > GET_INT(ch)) && (number(1, 40) <= GET_INT(victim))) {
		act("One of $N's false images dissipate and is instantly replaced!", FALSE, ch, 0, victim, TO_CHAR);
	        act("One of your images takes the $p and is replaced by another image.", FALSE, ch, 0, victim, TO_VICT);
    } else {
    /* okay, we know the guy has been hit.  now calculate damage. */
        dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
        if (GET_OBJ_TYPE(obj) == ITEM_MISSILE) {
	    dam += GET_DAMROLL(ch);
            dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
        }
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) 
            dam += (GET_OBJ_VAL(obj, 0) + dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)) / 2);
        else
            dam += number(0, (GET_OBJ_WEIGHT(obj) / 5));    
    }
    dam = MAX(1, dam);          /* at least 1 hp damage min per hit */
    damage(ch, victim, dam, w_type, obj);
    return TRUE;
  }
  return FALSE;
}

bool fire_at_char(struct char_data *ch, struct char_data * list, struct obj_data * obj, int dir)
{
  struct char_data *vict = list; 
  char msgbuf[256];

  while (vict) {	/* while there's a victim */
    /* check to hit */
    sprintf(msgbuf, "$p just flew in from %s.", dirs2[rev_dir[dir]]);
    act(msgbuf, FALSE, vict, obj, 0, TO_CHAR);
    if (ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL) || 
	(!pk_allowed && !IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)))
		send_to_char("A strange wall of force protects you!!!\r\n", vict);
    else if (range_hit(ch, vict, obj)) {
	if (vict) {
	  if (IS_NPC(vict) && !IS_NPC(ch) && GET_POS(ch) > POS_STUNNED) {
		  SET_BIT(MOB_FLAGS(vict), MOB_MEMORY);
		  remember(vict, ch);
		  HUNTING(vict) = ch;
	  }
	}
	return FALSE;
    }
    send_to_char("It flies past you harmlessly.", vict);
    vict = vict->next_in_room;
  }
  return TRUE;	/* missed everyone */
}
