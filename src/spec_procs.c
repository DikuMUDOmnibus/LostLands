/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
void advance_char(struct char_data * ch);

struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

extern char *spells[];

void sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
	tmp = spell_sort_info[a];
	spell_sort_info[a] = spell_sort_info[b];
	spell_sort_info[b] = tmp;
      }
}


char *how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 15)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 25)
    strcpy(buf, " (poor)");
  else if (percent <= 35)
    strcpy(buf, " (below average)");
  else if (percent <= 45)
    strcpy(buf, " (average)");
  else if (percent <= 55)
    strcpy(buf, " (fair)");
  else if (percent <= 65)
    strcpy(buf, " (good)");
  else if (percent <= 75)
    strcpy(buf, " (very good)");
  else if (percent <= 85)
    strcpy(buf, " (excellent)");
  else if (percent <= 90)
    strcpy(buf, " (learned)");
  else
    strcpy(buf, " (superb)");

  return (buf);
}

char *prac_types[] = {
  "spell",
  "skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define PRAC_TYPE	1	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[2][NUM_CLASSES];

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS_NUM_FULL(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS_NUM_FULL(ch)]])

int mag_manacost(struct char_data * ch, int spellnum);

void list_skills(struct char_data * ch)
{
  extern char *spells[];
  extern struct spell_info_type spell_info[];
  int i, j, sortpos, temp;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following %ss:\r\n", buf, SPLSKL(ch));

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    for (j = 0; j < NUM_CLASSES; j++) {
	if ( IS_SET(GET_CLASS(ch), (1 << j)) && (GET_LEVEL(ch) >= spell_info[i].min_level[j]) ) {
  	    sprintf(buf, "%-24s %-10.10s LV: %-2d  CST: ", spells[i], how_good(GET_SKILL(ch, i)), spell_info[i].min_level[j]);
	    strcat(buf2, buf);
	    if ((temp = mag_manacost(ch, i)))
		sprintf(buf,"%d\r\n", temp);
	    else
		sprintf(buf,"n/a\r\n");
	    strcat(buf2, buf);
	    break;
        }
    }
  }

  page_string(ch->desc, buf2, 1);
}

int do_guild(struct char_data *ch, void *me, int cmd, char *argument, int skilltype);

SPECIAL(mageguild) {
	return do_guild(ch, me, cmd, argument, SKILL_TYPE_MAGIC);
}

SPECIAL(clericguild) {
	return do_guild(ch, me, cmd, argument, SKILL_TYPE_G_CLERIC);
}

SPECIAL(thiefguild) {
	return do_guild(ch, me, cmd, argument, SKILL_TYPE_THIEF);
}

SPECIAL(fighterguild) {
	return do_guild(ch, me, cmd, argument, SKILL_TYPE_FIGHTER);
}

SPECIAL(othersguild) {
	return do_guild(ch, me, cmd, argument, SKILL_TYPE_ALL);
}

int do_guild(struct char_data *ch, void *me, int cmd, char *argument, int skilltype)
{
  int skill_num, percent, i, maxgain, mingain;

  extern struct spell_info_type spell_info[];
  extern struct int_app_type int_app[36];

  if (IS_NPC(ch) || !(CMD_IS("practice") || CMD_IS("advance")))
    return 0;

  skip_spaces(&argument);

  if (CMD_IS("advance")) {
    advance_char(ch);
    return 1;
  }

  if (!*argument) {
    list_skills(ch);
    return 1;
  }

  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return 1;
  }

  skill_num = find_skill_num(argument);

  if (!IS_SET(skilltype, spell_info[skill_num].type)) {
	send_to_char("Sorry, we do not teach that here...\r\n", ch);
	return 1;
  }
  if (IS_SET(SKILL_TYPE_MUNDANE, spell_info[skill_num].type)) {
	mingain = 5;
	maxgain = 25;
  }
  else if (IS_SET(SKILL_TYPE_MAGIC, spell_info[skill_num].type)) {
	mingain = 10;
	maxgain = 50;
  }
  else {
	mingain = 15;
	maxgain = 60;
  }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) {
    send_to_char("You are already learned in that area.\r\n", ch);
  } 
  else if (skill_num > 0) {
	for (i = 0; i < NUM_CLASSES; i++) {
		if (IS_SET(GET_CLASS(ch), (1 << i)) && GET_LEVEL(ch) >= spell_info[skill_num].min_level[i]) {
		  send_to_char("You practice for a while...\r\n", ch);
		  GET_PRACTICES(ch)--;

		  percent = GET_SKILL(ch, skill_num);
		  percent += MAX(mingain, 
			(MIN(maxgain, 
			 MAX(1, lvD6(ch) * (int_app[GET_INT(ch)].learn / 
			     spell_info[skill_num].min_level[i])))));

		  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

		  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
		    send_to_char("You are now learned in that area.\r\n", ch);

		  return 1;
		}
	}
  } else {
  sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
  send_to_char(buf, ch);
  }
  return 1;
}

int do_train(struct char_data *ch, void *me, int cmd, char *argument)
{
  char buf[256];
  bool str, dex, con, intel, wis, cha;
  
  if (IS_NPC(ch) || !CMD_IS("train"))
    return 0;

  skip_spaces(&argument);

  str = (ch->real_abils.str < 25);
  dex = (ch->real_abils.dex < 25);
  con = (ch->real_abils.con < 25);
  intel = (ch->real_abils.intel < 25);
  wis = (ch->real_abils.wis < 25);
  cha = (ch->real_abils.cha < 25);

  if (!*argument) {
    send_to_char("It costs 5 practices to train up one of your attributes,\r\n", ch);
    send_to_char("   or you can train 3 points of health or mana, or 5 points of movement.\r\n", ch);
    send_to_char("You can train in the following:\r\n",ch);
    strcpy(buf, "    health mana move");
    if (str)
	strcat(buf, " str");
    if (dex)
	strcat(buf, " dex");
    if (con)
	strcat(buf, " con");
    if (intel)
	strcat(buf, " int");
    if (wis)
	strcat(buf, " wis");
    if (cha)
	strcat(buf, " cha");
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return 1;
  }

  if (GET_PRACTICES(ch) <= 4) {
    send_to_char("You do not have enough practices to train right now.\r\n", ch);
    return 1;
  }

  if (!strcmp(argument, "health"))
	GET_MAX_HIT(ch) += 3;
  else if (!strcmp(argument, "mana"))
	GET_MAX_MANA(ch) += 3;
  else if (!strcmp(argument, "move"))
	GET_MAX_MOVE(ch) += 5;
  else  if (!strcmp(argument, "str") && str) {
	if (ch->real_abils.str == 18 && ( IS_WARRIOR(ch) || IS_BARBARIAN(ch) ||
		IS_MONK(ch) || IS_SAMURAI(ch) )) {
		if (ch->real_abils.str_add >= 90) {
			ch->real_abils.str_add = 0;
			ch->real_abils.str = 19;
		} else ch->real_abils.str_add += 10;
	} else ch->real_abils.str += 1;
  }
  else  if (!strcmp(argument, "dex") && dex)
	ch->real_abils.dex += 1;	
  else  if (!strcmp(argument, "con") && con)
	ch->real_abils.con += 1;	
  else  if (!strcmp(argument, "int") && intel)
	ch->real_abils.intel += 1;	
  else  if (!strcmp(argument, "wis") && wis)
	ch->real_abils.wis += 1;	
  else  if (!strcmp(argument, "cha") && cha)
	ch->real_abils.cha += 1;	
  else {
	send_to_char("You can't train that right now.\r\n", ch);
	return 1;
  }
  GET_PRACTICES(ch) -= 5;
  affect_total(ch);        
  send_to_char("Congratulations, you have successfully trained yourself.\r\n", ch);
  return 1;
}

SPECIAL(trainer) {
	return do_train(ch, me, cmd, argument);
}

SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  ACMD(do_drop);
  char *fname(char *namelist);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return 1;
}


SPECIAL(mayor)
{
  ACMD(do_gen_door);

  static char open_path[] =
  "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
  "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
    do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */

void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal some coins from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4))) {
      npc_steal(ch, cons);
      return TRUE;
    }
  return FALSE;
}


SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0))
    cast_spell(ch, vict, NULL, SPELL_SLEEP);

  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
    cast_spell(ch, vict, NULL, SPELL_BLINDNESS);

  if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }
  if (number(0, 4))
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
    break;
  case 6:
  case 7:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 8:
  case 9:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
    break;
  case 10:
  case 11:
    cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
    break;
  case 12:
  case 13:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  }
  return TRUE;

}

SPECIAL(cleric)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  if ((GET_LEVEL(ch) > 17) && (number(0, 10) == 0))
    cast_spell(ch, vict, NULL, SPELL_HARM);

  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
    if (IS_EVIL(ch))
	cast_spell(ch, vict, NULL, SPELL_CURSE);	
    else
	cast_spell(ch, vict, NULL, SPELL_CALL_LIGHTNING);

  if ((GET_LEVEL(ch) > 23) && (number(0, 8) == 0))
    cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE);

  if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }
  if (number(0, 4))
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT);
    break;
  case 6:
  case 7:
    cast_spell(ch, ch, NULL, SPELL_CURE_SERIOUS);
    break;
  case 8:
  case 9:
    cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC);
    break;
  case 10:
  case 11:
    cast_spell(ch, vict, NULL, SPELL_FLAME_ARROW);
    break;
  case 12:
  case 13:
    cast_spell(ch, vict, NULL, SPELL_CALL_LIGHTNING);
    break;
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_BLADEBARRIER);
    break;
  case 23:
    cast_spell(ch, vict, NULL, SPELL_PEACE);
  default:
    cast_spell(ch, ch, NULL, SPELL_HEAL);
    break;
  }
  return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(guild_guard)
{
  int i;
  extern int guild_info[][5];
  struct char_data *guard = (struct char_data *) me;
  char *denybuf = "$N tells you, 'Sorry, you do not belong there.' and blocks your way.\r\n";
  char *denybuf2 = "$N tells $n, 'Sorry, you do not belong there.' and blocks $s way";
  char *immortbuf = "$N announces your arrival and commands everyone to bow.\r\n";
  char *immortbuf2 = "$N announces the arrival of $n and commands everyone to bow.";
  char *racebuf[] = {
        "$N tells you that only Humans are allowed beyond this point.\r\n",
        "$N tells you that only Elves are allowed beyond this point.\r\n",
        "$N tells you that only HalfElves are allowed beyond this point.\r\n",
        "$N tells you that only Dwarves are allowed beyond this point.\r\n",
        "$N tells you that only Halflings are allowed beyond this point.\r\n",
        "$N tells you that only Gnomes are allowed beyond this point.\r\n",
        "$N tells you that only Hemnov are allowed beyond this point.\r\n"
        "$N tells you that only Llyran are allowed beyond this point.\r\n"
        "$N tells you that only Minotaurs are allowed beyond this point.\r\n"
        "$N tells you that only Pixies are allowed beyond this point.\r\n"
	"\n"
        };

  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND)) {
    return FALSE;
  }

  for (i = 0; guild_info[i][0] != -1; i++) {
    /* Check RACE */
    if (GET_LEVEL(ch) >= LVL_IMMORT && cmd == guild_info[i][4] &&
		world[ch->in_room].number == guild_info[i][3]) {
	act(immortbuf, FALSE, ch, 0, guard, TO_CHAR);
	act(immortbuf2, FALSE, ch, 0, guard, TO_ROOM);
	return FALSE;
    }
    if ((IS_NPC(ch)) && (guild_info[i][1] != ANYRACE) &&
	(guild_info[i][0] != ANYCLASS) &&
        (guild_info[i][2] != ANYALIGN) &&
        (cmd == guild_info[i][4])) {
      return TRUE; 
      }
  
    if (guild_info[i][1] != ANYRACE) {
      if ( !(GET_RACE(ch) == guild_info[i][1]) &&
  	   (world[ch->in_room].number == guild_info[i][3]) &&
	   (cmd == guild_info[i][4]) ) {
        act(racebuf[(guild_info[i][1])], FALSE, ch, 0, guard, TO_CHAR);
        act(denybuf2, FALSE, ch, 0, guard, TO_ROOM);
        return TRUE;
      }
    }
    /* Check CLASS */
    if (guild_info[i][0] != ANYCLASS) {
      if ( ( !IS_SET(GET_CLASS(ch), guild_info[i][0]) &&
    	   (world[ch->in_room].number == guild_info[i][3]) &&
	   (cmd == guild_info[i][4]) ) ) {
        act(denybuf, FALSE, ch, 0, guard, TO_CHAR);
        act(denybuf2, FALSE, ch, 0, guard, TO_ROOM);
        return TRUE;
      }
    }
    /* Check Align */

    /* Check Level */
    if (guild_info[i][5] > GET_LEVEL(ch) &&
    	   (world[ch->in_room].number == guild_info[i][3]) &&
	   (cmd == guild_info[i][4]) ) {
	act(denybuf2, FALSE, ch, 0, guard, TO_CHAR);
	return TRUE;
    }
    if (guild_info[i][6] < GET_LEVEL(ch) &&
    	   (world[ch->in_room].number == guild_info[i][3]) &&
	   (cmd == guild_info[i][4]) ) {
	act(denybuf2, FALSE, ch, 0, guard, TO_CHAR);
	return TRUE;
    }
  }
  return FALSE;
}


ACMD(do_say);

SPECIAL(puff)
{

  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  default:
    return (0);
  }
}



SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}



SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (FIGHTING(ch)) {
      if (dice(1, 10) == 1)
	      act("$n screams 'You're gonna get it now!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      else if (dice(1, 10) == 1)
	      act("$n screams 'How dare you attack a guard of Capitol!!!!!!'", FALSE, ch, 0, 0, TO_ROOM); 
      return FALSE;
  }

  max_evil = 300;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those wanted killers!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)){
      act("$n screams 'HEY!!!  You're one of the wanted outlaws!!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\r\n", 100 * GET_EXP(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < (GET_EXP(pet) * 100)) {
      send_to_char("You don't have enough coins!\r\n", ch);
      return (TRUE);
    }
    if (GET_LEVEL(ch) < GET_LEVEL(pet)) {
	send_to_char("You are not qualified to be that pet's master!\r\n", ch);
	return (TRUE);
    }
    GET_GOLD(ch) -= GET_EXP(pet) * 100;

    pet = read_mobile(GET_MOB_RNUM(pet), REAL, world[pet_room].zone);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);
    SET_BIT(MOB_FLAGS(pet), MOB_PET);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

SPECIAL(engraver)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;

  if (CMD_IS("list")) {
    send_to_char("Engraving an item makes that item permanently yours.\r\n", ch);
    send_to_char("It costs 100000 coins to engrave an item.\r\n", ch);
    send_to_char("Of course, you can unengrave an item, but that costs five times more.\r\n", ch);
    return (TRUE);
  } else if (CMD_IS("engrave")) {

    argument = one_argument(argument, buf);

    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
      send_to_char("You don't have that!\r\n", ch);
      return (TRUE);
    }
    if (IS_OBJ_STAT2(obj, ITEM2_ENGRAVED)) {
      send_to_char("That item's already engraved!\r\n", ch);
      return (TRUE);
    }
    if (IS_OBJ_STAT2(obj, ITEM2_AUTOENGRAVE)) {
      send_to_char("You can't engrave that!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < 100000) {
      send_to_char("You don't have enough coins!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= 100000;

    SET_BIT(GET_OBJ_EXTRA2(obj), ITEM2_ENGRAVED);
    strcpy(obj->owner_name, GET_NAME(ch));
    send_to_char("There you go, enjoy!!\r\n", ch);
    act("$n engraves $p.", FALSE, ch, obj, 0, TO_ROOM);
    return 1;
  } else if (CMD_IS("unengrave")) {

    argument = one_argument(argument, buf);

    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
      send_to_char("You don't have that!\r\n", ch);
      return (TRUE);
    }
    if (!IS_OBJ_STAT2(obj, ITEM2_ENGRAVED)) {
      send_to_char("That item isn't engraved!\r\n", ch);
      return (TRUE);
    }
    if (IS_OBJ_STAT2(obj, ITEM2_AUTOENGRAVE)) {
      send_to_char("You can't unengrave that!\r\n", ch);
      return (TRUE);
    }
    if (strcmp(obj->owner_name, GET_NAME(ch)) != 0) {
      send_to_char("Excuse me, but that item isn't yours to unengrave!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < 500000) {
      send_to_char("You don't have enough coins!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= 500000;

    REMOVE_BIT(GET_OBJ_EXTRA2(obj), ITEM2_ENGRAVED);
    strcpy(obj->owner_name, "");
    send_to_char("There you go, it's unengraved now!!\r\n", ch);
    act("$n unengraves $p.", FALSE, ch, obj, 0, TO_ROOM);
    return 1;
  }

  /* All commands except list and engrave and unengrave */
  return 0;
}

SPECIAL(war_reg)
{
  if (CMD_IS("register")) {
	if (PRF2_FLAGGED(ch, PRF2_RETIRED)) {
		send_to_char("Welcome back, sir.  How goes retirement?\r\n", ch);
		return 1;
        }
	if (PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI) || PRF2_FLAGGED(ch, PRF2_WAR_YLLANTRA)) {
		send_to_char("Yes sir!  But, you are already registered!\r\n", ch);
		return 1;
	}
	if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
		send_to_char("We do not want your kind here.  GO AWAY!\r\n", ch);
		return 1;
    	}
	if (PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT)) {
		send_to_char("We can not enlist bounty hunters, sorry.\r\n", ch);
		return 1;
	}
	send_to_char("Yes sir!  Now, which side of the war do you want to register for?\r\n", ch);
	send_to_char("Type 'enlist-druhari' or 'enlist-yllantra'.\r\n",ch);
	return 1;
  }
  if (CMD_IS("enlist-druhari")) {
	if (PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI) || PRF2_FLAGGED(ch, PRF2_WAR_YLLANTRA)) {
		send_to_char("Yes sir!  But, you are already registered!\r\n", ch);
		return 1;
	}
	if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
		send_to_char("We do not want your kind here.  GO AWAY!\r\n", ch);
		return 1;
    	}
	if (PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT)) {
		send_to_char("We can not enlist bounty hunters, sorry.\r\n", ch);
		return 1;
	}
	send_to_char("Yes sir!  Welcome to the Druhari Coalition, and may your kill/killed ratio be always positive.\r\n", ch);
	SET_BIT(PRF2_FLAGS(ch), PRF2_WAR_DRUHARI);	
	return 1;
  }
  if (CMD_IS("enlist-yllantra")) {
	if (PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI) || PRF2_FLAGGED(ch, PRF2_WAR_YLLANTRA)) {
		send_to_char("Yes sir!  But, you are already registered!\r\n", ch);
		return 1;
	}
	if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
		send_to_char("We do not want your kind here.  GO AWAY!\r\n", ch);
		return 1;
    	}
	if (PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT)) {
		send_to_char("We can not enlist bounty hunters, sorry.\r\n", ch);
		return 1;
	}
	send_to_char("Yes sir!  Welcome to the Yllantra Collective, and may your kill/killed ratio be always positive.\r\n", ch);
	SET_BIT(PRF2_FLAGS(ch), PRF2_WAR_YLLANTRA);	
	return 1;
  }
  return 0;
}

SPECIAL(recruiter)
{
  if (cmd)
    return (0);

  switch (number(0, 50)) {
  case 0:
  case 1:
  case 2:
    do_say(ch, "Join the war!", 0, 0);
    return (1);
  case 10:
  case 11:
  case 12:
    do_say(ch, "Register for the war here!", 0, 0);
    return (1);
  case 20:
  case 21:
  case 22:
    do_say(ch, "Sign up now!  Join the cause!", 0, 0);
    return (1);
  case 30:
  case 31:
  case 32:
    do_say(ch, "Fight, fight, fight!  Register now!", 0, 0);
    return (1);
  case 40:
  case 41:
  case 42:
    do_say(ch, "Be, all that you can be... now, how does that go again?", 0, 0);
    return (1);
  default:
    return (0);
  }
}

SPECIAL(bounty_reg)
{
  if (CMD_IS("register")) {
	if (PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT)) {
		send_to_char("But, you are already a registered bounty hunter!\r\n", ch);
		return 1;
 	}
	if (GET_NUM_OF_CLASS(ch) != 1) {
		send_to_char("Sorry, you can not register to be a bounty hunter.\r\n", ch);
		return 1;
	}
	if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
		send_to_char("HA! You are more a fool than I once thought.\r\n", ch);
		return 1;
	}
	send_to_char("Welcome, Bounty Hunter.  Now go get some Assassins!\r\n", ch);
	SET_BIT(PRF2_FLAGS(ch), PRF2_BOUNTYHUNT);	
	return 1;
  }
  return 0;
}

SPECIAL(assass_reg)
{
  if (CMD_IS("register")) {
	if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
		send_to_char("You are already an assassin!\r\n", ch);
		return 1;
 	}
	if (GET_NUM_OF_CLASS(ch) != 1) {
		send_to_char("Sorry, I don't know what you mean, multi-class slime!\r\n", ch);
		return 1;
	}
	if (!(IS_THIEF(ch) || IS_NINJA(ch))) {
		send_to_char("HA! You are more a fool than I once thought.\r\n", ch);
		return 1;
	}
	send_to_char("Welcome to the Assassin's Circle!\r\n", ch);
	SET_BIT(PRF2_FLAGS(ch), PRF2_ASSASSIN);
	return 1;
  }
  return 0;
}

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %d coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}

