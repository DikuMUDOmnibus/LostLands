/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int pk_allowed;

/* extern functions */
void check_killer(struct char_data * ch, struct char_data * victim);
void raw_kill(struct char_data * ch, struct char_data * killer);
bool CAN_MURDER(struct char_data *ch, struct char_data * victim);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    for (opponent = world[ch->in_room].people;
	 opponent && (FIGHTING(opponent) != helpee);
	 opponent = opponent->next_in_room)
		;
    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pk_allowed && !IS_NPC(opponent) && !CAN_MURDER(ch, opponent))	/* prevent accidental pkill */
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!pk_allowed) {
      if ( !IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict) && (subcmd != SCMD_MURDER)) {
	send_to_char("Use 'murder' to hit another person.  Be forewarned, you will be flagged a Killer.\r\n", ch);
	return;
        }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict)) {
	send_to_char("Wimp, you can't order a charmed pet to attack a player.\r\n", ch);
	return;			/* you can't order a charmed pet to attack a
				 * player */
      }
    }  
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
      send_to_char("This room has a nice peaceful feeling.\r\n", ch);
      return;
    }
    if (SCMD_MURDER) {
	check_killer(ch, vict);
    }
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}

ACMD(do_kill)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else
      do_hit(ch, argument, cmd, subcmd);
  }
  return;
}

ACMD(do_disarm)
{
  struct obj_data *obj;
  struct char_data *vict;

  one_argument(argument, buf);

  if (!*buf) {
        send_to_char("Whom do you want to disarm?\r\n", ch);
	return;
  }
  else if (!(vict = get_char_room_vis(ch, buf))) {
        send_to_char(NOPERSON, ch);
	return;
  }
  else if (vict == ch) {
        send_to_char("Try removing your weapon instead.\r\n", ch);
	return;
  }
  else if (!pk_allowed && !IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)) {
        send_to_char("That would be seen as an act of aggression!\r\n", ch);
	return;
  }
  else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict)) {
        send_to_char("The thought of disarming your master seems revolting to you.\r\n", ch);
	return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room has a nice peaceful feeling.\r\n", ch);
    return;
  }
  else if (!(obj = GET_EQ(vict, WEAR_WIELD)))
        act("$N is unarmed!", FALSE, ch, 0, vict, TO_CHAR);
  else if ((GET_OBJ_VAL(obj, 0) == 2) ||
           (number(1, 101) > GET_SKILL(ch, SKILL_DISARM)))
        act("You failed to disarm $N!", FALSE, ch, 0, vict, TO_CHAR);
  else if (dice(2, GET_STR(ch)) + GET_LEVEL(ch) <= dice(2, GET_STR(vict)) + GET_LEVEL(vict)) {
        act("You almost succeed in disarming $N", FALSE, ch, 0, vict, TO_CHAR);
        act("You were almost disarmed by $N!", FALSE, vict, 0, ch, TO_CHAR);
  } else {
        obj_to_room(unequip_char(vict, WEAR_WIELD), vict->in_room);
        act("You succeed in disarming your enemy!", FALSE, ch, 0, 0, TO_CHAR);
        act("Your $p flies from your hands!", FALSE, vict, obj, 0, TO_CHAR);
        act("$n disarms $N, $p drops to the ground.", FALSE, ch, obj, vict, TO_ROOM);
  }
  hit(vict , ch, TYPE_UNDEFINED);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_slay)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
      send_to_char("This room has a nice peaceful feeling.\r\n", ch);
      return;
    }
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}


ACMD(do_backstab)
{
  struct char_data *vict;
  struct obj_data *obj;
  int percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!(obj = GET_EQ(ch, WEAR_WIELD))) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room has a nice peaceful feeling.\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    return;
  }

  if (!CAN_MURDER(ch, vict)) {
	send_to_char("You can't backstab a person under the protection of Neutrality.\r\n", ch);
	return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BACKSTAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB, obj);
  else
    hit(ch, vict, SKILL_BACKSTAB);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_order)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(OK, ch);
	command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(OK, ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}


ACMD(do_flee)
{
  int i, attempt, loss;

  loss = GET_NUM_OF_CLASS(ch) * GET_LEVEL(ch) * 20;
  gain_exp(ch, -loss);

  if (FIGHTING(ch) && MOB3_FLAGGED(FIGHTING(ch), MOB3_CANT_FLEE)) {
	act("$N prevents you from fleeing!!!", FALSE, ch, 0, FIGHTING(ch), TO_CHAR);
	act("$N prevents $n from fleeing!!!", FALSE, ch, 0, FIGHTING(ch), TO_ROOM);
 	return;
  }
  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char("You flee head over heels.\r\n", ch);
	if (FIGHTING(ch)) {
	  if (FIGHTING(FIGHTING(ch)) == ch)
	    stop_fighting(FIGHTING(ch));
	  stop_fighting(ch);
	}
      } else {
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}




ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!(IS_WARRIOR(ch) || IS_BARBARIAN(ch))) {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!(obj = GET_EQ(ch, WEAR_SHIELD))) {
    send_to_char("You need to wear a shield to make it a success.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room has a nice peaceful feeling.\r\n", ch);
    return;
  }
  if (!IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)) {
	send_to_char("Type 'murder' if you want to attack that person.\r\n", ch);
	return;
        }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BASH);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH, obj);
    GET_POS(ch) = POS_SITTING;
  } else {
    damage(ch, vict, 1, SKILL_BASH, obj);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (!(IS_WARRIOR(ch) || IS_BARBARIAN(ch) || IS_SAMURAI(ch)))
    send_to_char("But only true warriors can do this!", ch);
  else {
    percent = number(1, 101);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_RESCUE);

    if (percent > prob) {
      send_to_char("You fail the rescue!\r\n", ch);
      return;
    }
    send_to_char("Banzai!  To the rescue...\r\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

    if (FIGHTING(vict) == tmp_ch)
      stop_fighting(vict);
    if (FIGHTING(tmp_ch))
      stop_fighting(tmp_ch);
    if (FIGHTING(ch))
      stop_fighting(ch);

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
  }

}

ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;
  struct obj_data *obj;

  if (!(IS_WARRIOR(ch) || IS_BARBARIAN(ch) || IS_SAMURAI(ch))) {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room has a nice peaceful feeling.\r\n", ch);
    return;
  }
  if (!(obj = GET_EQ(ch, WEAR_FEET))) {
    send_to_char("You need some kind of footwear to make it a success.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)) {
    send_to_char("Type 'murder' if you want to attack that person.\r\n", ch);
    return;
  }

  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 101);	/* 101% is a complete
								 * failure */
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK, obj);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_KICK, obj);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


static char *fire_msg[] = {
  "You are not holding something to throw.\r\n",
  "You must wield the correct weapon.\r\n",
  "You are not holding the right missile type for that weapon.\r\n",
  "You must specify a correct direction.\r\n",
  "You pull your arm back and throw $p.",
  "You take aim and fire $p.",
  "With herculean might, $n throws $p.",
  "With skill equal to that of William Tell, $n fires $p.",
  "Losing its momentum, $p falls to the ground.",
  "Miraculously, $p returns to $n.",
  "Miraculously, $p returns to you."
  "\n"
};

extern char *dirs[];

void fire_in_direction(struct char_data *ch, struct obj_data *obj, int look_type, int distance);

ACMD(do_fire)
{
  static char arg2[MAX_INPUT_LENGTH];
  struct obj_data * obj = GET_EQ(ch, WEAR_WIELD);
  int look_type, distance;

  half_chop(argument, arg, arg2);

  if (!*arg || (look_type = search_block(arg, dirs, FALSE)) < 0) {
	send_to_char(fire_msg[3], ch);
	return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
	send_to_char("Divine intervention prevents you from doing that.\r\n", ch);
	return;
  }
  if (subcmd == SCMD_THROW) {
	obj = GET_EQ(ch, WEAR_DUALWIELD);
	if (!obj && !GET_EQ(ch, WEAR_WIELD)) {
		if (!GET_EQ(ch, WEAR_HOLD)) {
			send_to_char(fire_msg[0], ch);
			return;
		}
		else
		  obj = unequip_char(ch, WEAR_HOLD);
	} else if (!obj) {
		obj = unequip_char(ch, WEAR_WIELD);
	} else {
		obj = unequip_char(ch, WEAR_DUALWIELD);
	}
	if (GET_OBJ_TYPE(obj) == ITEM_MISSILE)
		distance = 2;
	else 
		distance = 1;
  } else { 	/* fire, not throw */
	if (!obj || GET_OBJ_TYPE(obj) != ITEM_FIREWEAPON) {
		send_to_char(fire_msg[1], ch);
		return;
	}
	if (!GET_EQ(ch, WEAR_HOLD) || GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) != ITEM_MISSILE ||
            GET_OBJ_VAL(obj, 3) != GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3)) {
		send_to_char(fire_msg[2], ch);
		return;
	}
	obj = unequip_char(ch, WEAR_HOLD);
	distance = GET_OBJ_VAL(obj, 0);
	if (*arg2)
		distance = MAX(1, MIN(atoi(arg2), distance));
	distance = MIN(distance, 10);	/* Max firing distance is 8 */
  }
  act(fire_msg[4 + subcmd], FALSE, ch, obj, 0, TO_CHAR);
  act(fire_msg[6 + subcmd], FALSE, ch, obj, 0, TO_ROOM);
  fire_in_direction(ch, obj, look_type, distance);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

bool range_hit(struct char_data *ch, struct char_data * victim, struct obj_data *obj);

bool fire_at_char(struct char_data *ch, struct char_data * list, struct obj_data * obj, int dir);

void fire_in_direction(struct char_data *ch, struct obj_data *obj, int dir, int distance)
{
  room_num temp_room = ch->in_room;
  struct char_data *vict;
  bool contin = TRUE;		/* Has missile hit someone yet? (true = no) */

  while (contin && EXITN(temp_room, dir) && (distance-- > 0) && 
	 !IS_SET(EXITN(temp_room, dir)->exit_info, EX_CLOSED)) {
	temp_room = EXITN(temp_room, dir)->to_room;
	if (ROOM_FLAGGED(temp_room, ROOM_MURKY))
	   distance--;
	else
	   contin = fire_at_char(ch, world[temp_room].people, obj, dir);
  }
  if (IS_OBJ_STAT2(obj, ITEM2_RETURNING)) {
	  act(fire_msg[10], FALSE, ch, obj, 0, TO_CHAR);
	  act(fire_msg[9], FALSE, ch, obj, 0, TO_ROOM);
  	  if ((vict = world[temp_room].people)) {
	    act(fire_msg[9], FALSE, ch, obj, vict, TO_VICT);
	    act(fire_msg[9], FALSE, ch, obj, vict, TO_NOTVICT);
	  }
	  obj_to_char(obj, ch);
  } else {
  	obj_to_room(obj, temp_room);
  	if ((vict = world[temp_room].people)) {
	  act(fire_msg[8], FALSE, vict, obj, 0, TO_CHAR);
	  act(fire_msg[8], FALSE, vict, obj, 0, TO_ROOM);
  	}
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 4);
}
