/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include <time.h>
#include <errno.h>
#include <sys/time.h>

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
#include "screen.h"
#include "constants.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[NUM_CLASSES][LVL_IMPL + 1];
extern struct command_info cmd_info[];

/*
extern struct str_app_type *str_app;
extern struct dex_app_type *dex_app;
*/

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *buglist;
extern char *idealist;
extern char *typolist;

long find_class_bitvector(char arg);

void show_obj_to_char(struct obj_data * object, struct char_data * ch,
			int mode)
{
  bool found, showflag, showflag2;

  *buf = '\0';
  if ((mode == 0) && object->description)
    strcpy(buf, object->description);
  else if (object->short_description && ((mode == 1) ||
				 (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buf, object->short_description);
  else if (mode == 5) {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
      if (object->action_description) {
	strcpy(buf, "There is something written upon it:\r\n\r\n");
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
      } else
	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {
      strcpy(buf, "You see nothing special..");
      mode = 3;
    } else {			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcpy(buf, "It looks like a drink container.");
      mode = 3;
    }
  }
  showflag = FALSE;
  showflag2 = FALSE;
  if (mode != 3) {
    found = FALSE;
    if (IS_OBJ_STAT2(object, ITEM2_ENGRAVED)) {
      strcat(buf, " of ");
      strcat(buf, object->owner_name);
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, " (hum)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, " (invis)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, " (soft");
      found = TRUE;
      showflag = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      if (!showflag) 
	sprintf(buf, "%s (%sbluish", buf, CCBLU(ch, C_SPR));
      else
	sprintf(buf, "%s %sbluish", buf, CCBLU(ch, C_SPR));
      showflag = TRUE;
      showflag2 = TRUE;
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && CAN_SEE_MAGIC(ch)) {
      if (!showflag) 
	sprintf(buf, "%s (%sgreen", buf, CCGRN(ch, C_SPR));
      else if (!showflag2)
	sprintf(buf, "%s %sgreen", buf, CCGRN(ch, C_SPR));
      else
	sprintf(buf, "%s-%sgreen", buf, CCGRN(ch, C_SPR));
      showflag = TRUE;
      found = TRUE;
    }
    if (showflag)
	sprintf(buf, "%s %sglow)", buf, CCNRM(ch, C_SPR));
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}

void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
		           bool show)
{
  struct obj_data *i;
  bool found;

  found = FALSE;
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}


void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		/* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, " is in excellent condition.\r\n");
  else if (percent >= 90)
    strcat(buf, " has a few scratches.\r\n");
  else if (percent >= 75)
    strcat(buf, " has some small wounds and bruises.\r\n");
  else if (percent >= 50)
    strcat(buf, " has quite a few wounds.\r\n");
  else if (percent >= 30)
    strcat(buf, " has some big nasty wounds and scratches.\r\n");
  else if (percent >= 15)
    strcat(buf, " looks pretty hurt.\r\n");
  else if (percent >= 0)
    strcat(buf, " is in awful condition.\r\n");
  else
    strcat(buf, " is bleeding awfully from big wounds.\r\n");

  send_to_char(buf, ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;
  struct obj_data *tmp_obj;

  if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(where[j], ch);
	show_obj_to_char(GET_EQ(i, j), ch, 1);
      }
  }
  if (ch != i && (IS_THIEF(ch) || GET_LEVEL(ch) >= LVL_IMMORT)) {
    found = FALSE;
    act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 25) < GET_LEVEL(ch))) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
    }

    if (!found)
      send_to_char("You can't see anything.\r\n", ch);
  }
}


void list_one_char(struct char_data * i, struct char_data * ch)
{
  bool showflag;
  char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here."
  };

  showflag = FALSE;
  strcpy(buf, "");

  /* determine aura(s) */

  if (IS_AFFECTED(i, AFF_SANCTUARY)) {
      strcat(buf, "[bright");
      showflag = TRUE;
      }

  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      if (showflag)
        strcat(buf, "-");
      else
        strcpy(buf, "[");
      if (IS_EVIL(i))
	strcat(buf, "red");
      else if (IS_GOOD(i))
	strcat(buf, "blue");
      else
        strcat(buf, "green");
      showflag = TRUE;
    }
  if (showflag)
	strcat(buf, " aura] ");

  showflag = TRUE;
  if (AFF2_FLAGGED(i, AFF2_MIRRORIMAGE))
	strcat(buf, "(Multiple) ");

  if (AFF3_FLAGGED(i, AFF3_PASSDOOR))
	strcat(buf, "(Translucent) ");

  if (MOB_FLAGGED(i, MOB_ETHEREAL))
	strcat(buf, "<ETHEREAL> ");

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    strcat(buf, i->player.long_descr);
    showflag = FALSE;
  } else if (IS_NPC(i)) {
    strcat(buf, i->player.short_descr);
    CAP(buf);
  } else
    sprintf(buf, "%s%s %s", buf, i->player.name, GET_TITLE(i));  

  if (showflag && GET_POS(i) != POS_FIGHTING) {
    if (GET_POS(i) != POS_STANDING) 
	strcat(buf, positions[(int) GET_POS(i)]);
    else {
	if (IS_AFFECTED(i, AFF_FLYING))
	       strcat(buf, " is flying in the air.");
        else if (IS_AFFECTED(i, AFF_WATERWALK)) 
	       strcat(buf, " is floating in the air.");
        else strcat(buf, positions[(int) GET_POS(i)]);
        }
  } else {
    if (FIGHTING(i)) {
      strcat(buf, " is here, fighting ");
      if (FIGHTING(i) == ch)
	strcat(buf, "YOU!");
      else {
	if (i->in_room == FIGHTING(i)->in_room)
	  strcat(buf, PERS(FIGHTING(i), ch));
	else
	  strcat(buf, "someone who has already left");
	strcat(buf, "!");
      }
    } /* else */			/* NIL fighting pointer */
/*      strcat(buf, " is here struggling with thin air."); */
  }
  if (showflag)
	strcat(buf, "\r\n");
  send_to_char(buf, ch);

  showflag = FALSE;
  strcpy(buf, "  ...$e is ");
  if (IS_AFFECTED(i, AFF_BLIND)) {
    strcat(buf, "groping blindly");
    showflag = TRUE;
    }
  if (IS_AFFECTED(i, AFF_INVISIBLE)) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "invisible");
    }
  if (IS_AFFECTED(i, AFF_HIDE)) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "hidden");
    }
  if (AFF2_FLAGGED(i, AFF2_BLINK)) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "displaced");
    }
  if (!IS_NPC(i) && !i->desc) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "linkless");
    }
  if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_WRITING)) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "writing");
    }
  if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_EDITING)) {
    if (showflag) 
	strcat(buf, " and ");
    else 
	showflag = TRUE;
    strcat(buf, "editing");
    }
  if (showflag) {
	strcat(buf, ".");
	act(buf, TRUE, i, 0, ch, TO_VICT);
  }
}



void list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;

  for (i = list; i; i = i->next_in_room)
    if (ch != i) {
      if (CAN_SEE(ch, i) && (!MOB_FLAGGED(i, MOB_ETHEREAL) || GET_LEVEL(ch) > LVL_IMMORT))
	list_one_char(i, ch);
      else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) && !MOB_FLAGGED(i, MOB_ETHEREAL))
	send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
    }
}


ACMD(do_affects)
{
char buf[256], buf2[256];
struct affected_type *aff;
bool showed = FALSE;

if (IS_IMMORT(ch) || AFF_FLAGGED(ch, AFF_DETECT_MAGIC)) {
	send_to_char("Affected by: ", ch);
        if (AFF_FLAGS(ch)) {
		sprintbit(AFF_FLAGS(ch), affected_bits, buf);
		send_to_char(buf, ch);
		showed = TRUE;
        }
	if (AFF2_FLAGS(ch)) {
		send_to_char("\r\n             ", ch);
		sprintbit(AFF2_FLAGS(ch), affected_bits2, buf);
		send_to_char(buf, ch);
		showed = TRUE;
        }
	if (AFF3_FLAGS(ch)) {
		send_to_char("\r\n             ", ch);
		sprintbit(AFF3_FLAGS(ch), affected_bits3, buf);
		send_to_char(buf, ch);
		showed = TRUE;
        }
	if (showed) 
		send_to_char("\r\n", ch);
	else 
		send_to_char("Nothing\r\n", ch);
}
if (ch->affected) {
    for (aff = ch->affected; aff; aff = aff->next) {
	*buf2 = '\0';
	if (GET_LEVEL(ch) >= 10) {
		sprintf(buf2, "(%3dhr)", aff->duration + 1);
        }
	sprintf(buf, "Spells: %s %s%-21s%s ", buf2, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
	if (aff->modifier && (GET_LEVEL(ch) >= 20)) {
		sprintf(buf2, "%+d to %s\r\n", aff->modifier, apply_types[(int) aff->location]);
	}
	else	
		sprintf(buf2, "\r\n");
	strcat(buf, buf2);
        send_to_char(buf, ch);
        }
     }
else
     send_to_char("You are not affected by any spells.\r\n", ch);
}


void do_auto_dir(struct char_data * ch)
{
  int door;

  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      sprintf(buf, "%s%c ", buf, LOWER(*dirs[door]));;

  sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
	  *buf ? buf : "None! ", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
}

ACMD(do_exits)
{
  int door;
  bool hidden;

  *buf = '\0';

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++) {
    hidden = FALSE;
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
      if (GET_LEVEL(ch) >= LVL_IMMORT) {
	sprintf(buf2, "   %-5s - [%5d] %-15s", dirs[door],
		world[EXIT(ch, door)->to_room].number,
		world[EXIT(ch, door)->to_room].name);
        if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
   	  sprintf(buf2, "%-30s [CLOSED]", buf2);
        if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
	  strcat(buf2, "[HIDDEN]");
      } else {
	sprintf(buf2, "  %-5s - ", dirs[door]);
	if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
	  strcat(buf2, "Too dark to tell");
	else {
	  if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
  	    strcat(buf2, "(hidden) ");
	    hidden = TRUE;
            }
          if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
  	    strcat(buf2, "(closed)");
	  else 
  	    strcat(buf2, world[EXIT(ch, door)->to_room].name);
  	  }
      }
      if ( (hidden && !IS_AFFECTED(ch, AFF_SENSE_LIFE)) && !IS_IMMORT(ch))
	  strcpy(buf2, "");
      else
	  strcat(buf2, "\r\n");
      strcat(buf, CAP(buf2));
    }
  }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}

void look_at_room(struct char_data * ch, int ignore_brief)
{
  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    return;
  } else if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }

  /* autodir */
  if (PRF_FLAGGED(ch, PRF_AUTODIR))
    do_auto_dir(ch);

  send_to_char(CCCYN(ch, C_NRM), ch);
  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
    sprintbit((long) ROOM_FLAGS(ch->in_room), room_bits, buf);
    sprintf(buf2, "[%5d] %s [%s: %s]", world[ch->in_room].number,
	    world[ch->in_room].name, sector_types[world[ch->in_room].sector_type], buf);
    send_to_char(buf2, ch);
    if (world[ch->in_room].tele != NULL)
	send_to_char(" Teleport", ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  /* autoexits */
  if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_exits(ch, 0, 0, 0);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}

void look_in_direction(struct char_data * ch, int dir)
{
  room_num temp_room;  
  int distance = 0, counter = 0;
  char buf2[127];
  bool show = TRUE;

  if (IS_HEMNOV(ch))
	distance = 4;
  else if (IS_ELF(ch) || IS_HALFLING(ch))
	distance = 3;
  else
	distance = 2;

  if (number(1, 101) <= GET_SKILL(ch, SKILL_ENH_SIGHT)) {
 	distance++;
	if (number(1, 111) <= GET_SKILL(ch, SKILL_ENH_SIGHT)) {
	 	distance++;
   	        if (number(1, 121) <= GET_SKILL(ch, SKILL_ENH_SIGHT))
		 	distance++;
  		}
	}

  if (AFF2_FLAGGED(ch, AFF2_FARSEE)) {
	distance = 8;
  }
  distance = MIN(distance, 8);

  if (!EXIT(ch, dir))
    send_to_char("Nothing special there...\r\n", ch);
  else  {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("You see nothing special.\r\n", ch);

    temp_room = (ch)->in_room;

    do {
      show = TRUE;
      if (IS_SET(EXITN(temp_room, dir)->exit_info, EX_HIDDEN))
	return;
      if (IS_SET(EXITN(temp_room, dir)->exit_info, EX_CLOSED) && EXITN(temp_room, dir)->keyword) {
        sprintf(buf, "The %s is closed.\r\n", fname(EXITN(temp_room, dir)->keyword));
        send_to_char(buf, ch);
        return;
      } else if (show && IS_SET(EXITN(temp_room, dir)->exit_info, EX_ISDOOR) && EXITN(temp_room, dir)->keyword) {
        sprintf(buf, "The %s is open.\r\n", fname(EXITN(temp_room, dir)->keyword));
        send_to_char(buf, ch);
      }
      if (EXITN(temp_room, dir)->to_room == NOWHERE) {
/*	sprintf(buf, "Incorrect room number found in #%d -> #%d", world[temp_room].number, EXITN(temp_room, dir)->to_room_vnum);
	mudlog(buf, BRF, LVL_IMMORT, TRUE);
	send_to_char("There's something wrong with this room, please report to an IMP.", ch);
*/
	return;
      }
      temp_room = EXITN(temp_room, dir)->to_room;
      counter++;
	if (IS_MURKY(temp_room) && !IS_IMMORT(ch)) {
		send_to_char("Something prevents you from seeing further.\r\n", ch);
	        return;
	}
	if (IS_DARK(temp_room) && !CAN_SEE_IN_DARK(ch)) {
		strcpy(buf2, "You see nothing but darkness");
	 	show = FALSE;
        }
	else
		strcpy(buf2, world[temp_room].name);
      sprintf(buf, " Dist: %d   %-15s\r\n", counter, buf2);
      send_to_char(buf, ch);
      if (show && temp_room) {
		send_to_char(CCCYN(ch, C_NRM), ch);
		list_char_to_char(world[temp_room].people, ch);
		send_to_char(CCNRM(ch, C_NRM), ch);
      }
  } 
  while (EXITN(temp_room, dir) && (counter < distance));
  }
}


void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char("There's nothing inside that!\r\n", ch);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	send_to_char("It is closed.\r\n", ch);
      else {
	send_to_char(fname(obj->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char(" (carried): \r\n", ch);
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char(" (here): \r\n", ch);
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char(" (used): \r\n", ch);
	  break;
	}

	list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {		/* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char("It is empty.\r\n", ch);
      else {
	amt = ((GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0));
	sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt],
		color_liquid[GET_OBJ_VAL(obj, 2)]);
	send_to_char(buf, ch);
      }
    }
  }
}


char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return NULL;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = 0, j;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
	act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
    page_string(ch->desc, desc, 0);
    return;
  }
  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  if (bits) {			/* If an object was found back in
				 * generic_find */
    if (!found)
      show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
  } else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("Read what?\r\n", ch);
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)			/* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("You're broke!\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("You have one miserable little coin.\r\n", ch);
  else {
    sprintf(buf, "You have %d coins.\r\n", GET_GOLD(ch));
    send_to_char(buf, ch);
  }
}


ACMD(do_score)
{
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];
  extern char *pc_class_types[];
  extern char *pc_race_types[];
  int i;
  char colorbuf[30];

 sprintf(buf, "Your name is  %s %s (level %d, Incarnation %d).\r\n",
	  GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch), GET_REINCARN(ch));

  sprintf(buf, "%sYou are a %d year old %s.", buf, 
	GET_AGE(ch), pc_race_types[(int)GET_RACE(ch)]);

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, "  It's your birthday today.\r\n");
  else
    strcat(buf, "\r\n");

  if (GET_NUM_OF_CLASS(ch) > 1) {
	  strcat(buf, "You have decided to join the ways of the:\r\n");
	  for (i = 0; i < NUM_CLASSES; i++) {
	  	if (IS_SET(GET_CLASS(ch), (1 << i)))
			sprintf(buf, "%s	%s\r\n", buf, pc_class_types[i]);
	  }
  }
  else 
    sprintf(buf, "%sYou have decided to join the ways of the %s.\r\n",
	 buf, pc_class_types[GET_CLASS_NUM(ch)]);
	
  i = 100 * GET_HIT(ch) / GET_MAX_HIT(ch);
  STATUS_COLOR(i, colorbuf, ch, C_SPR);
  sprintf(buf,
       "%sYou have %s%d%s/%d hit ",
	  buf, colorbuf, GET_HIT(ch), CCNRM(ch, C_SPR), GET_MAX_HIT(ch));

  i = 100 * GET_MANA(ch) / GET_MAX_MANA(ch);
  STATUS_COLOR(i, colorbuf, ch, C_SPR);
  sprintf(buf, "%s%s%d%s/%d mana ",
	  buf, colorbuf, GET_MANA(ch), CCNRM(ch, C_SPR), GET_MAX_MANA(ch));

  i = 100 * GET_MOVE(ch) / GET_MAX_MOVE(ch);
  STATUS_COLOR(i, colorbuf, ch, C_SPR);
  sprintf(buf, "%sand %s%d%s/%d movement points.\r\n",
	  buf, colorbuf, GET_MOVE(ch), CCNRM(ch, C_SPR), GET_MAX_MOVE(ch));
  send_to_char(buf, ch);

  sprintf(buf,
        "Str: %d/%d  Dex: %d  Con: %d  Int: %d  Wis: %d  Cha: %d\r\n", 
	GET_STR(ch), GET_ADD(ch), GET_DEX(ch), GET_CON(ch), GET_INT(ch),
        GET_WIS(ch), GET_CHA(ch));

  sprintf(buf,
        "%sYour armor class is %.1f, and your alignment is %s",
	 buf, (float) GET_AC(ch) + (10 *  dex_app[GET_DEX(ch)].defensive) / 10,
	 align_types[get_alignment_type(GET_ALIGNMENT(ch))]);

  if (GET_LEVEL(ch) >= 20)
	sprintf(buf,"%s (%d)\r\n", buf, GET_ALIGNMENT(ch));
  else
	strcat(buf, ".\r\n");

  if (GET_LEVEL(ch) >= 16) 
     sprintf(buf,
        "%sHitroll: %2d   Damroll:  %2d    Saving throws: [%d/%d/%d/%d/%d]\r\n", buf,
	  ch->points.hitroll + str_app[STRENGTH_APPLY_INDEX(ch)].tohit,
          ch->points.damroll + str_app[STRENGTH_APPLY_INDEX(ch)].todam,
          GET_SAVE(ch, 0), GET_SAVE(ch, 1), GET_SAVE(ch, 2), GET_SAVE(ch, 3),
	  GET_SAVE(ch, 4));

  sprintf(buf, "%s\r\nYou have scored %d exp, and have %d coins.\r\n",
	  buf, GET_EXP(ch), GET_GOLD(ch));

  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) < (LVL_IMMORT - 1))
      sprintf(buf, "%sYou need %d exp to reach your next level.\r\n\r\n", buf,
	(titles[(int) GET_CLASS_NUM_FULL(ch)][GET_LEVEL(ch) + 1].exp) - GET_EXP(ch));

    sprintf(buf, "%sYou have been killed %s%d%s times by monsters and %s%d%s times by other players.\r\n", buf,
	    CCRED(ch, C_SPR),
	    ch->player_specials->saved.killed_by_mob,
	    CCNRM(ch, C_SPR),
	    CCRED(ch, C_SPR),
	    ch->player_specials->saved.killed_by_player,
	    CCNRM(ch, C_SPR));

    sprintf(buf, "%sYou have killed %s%d%s other players.\r\n\r\n", buf,
	    CCCYN(ch, C_SPR),
	    ch->player_specials->saved.killed_player,
	    CCNRM(ch, C_SPR));
    playing_time = real_time_passed((time(0) - ch->player.time.logon) +
				  ch->player.time.played, 0);
    sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	  buf, playing_time.day, playing_time.hours);
  }

  strcat(buf, "You are ");
  switch (GET_POS(ch)) {
  case POS_DEAD:
    sprintf(buf, "%s%sDEAD!%s\r\n", buf, CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
    break;
  case POS_MORTALLYW:
    sprintf(buf, "%s%smortally wounded!%s  You should seek help!\r\n",
	   buf, CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
    break;
  case POS_INCAP:
    strcat(buf, "incapacitated, slowly fading away...\r\n");
    break;
  case POS_STUNNED:
    strcat(buf, "stunned!  You can't move!\r\n");
    break;
  case POS_SLEEPING:
    strcat(buf, "sleeping.\r\n");
    break;
  case POS_RESTING:
    strcat(buf, "resting.\r\n");
    break;
  case POS_SITTING:
    strcat(buf, "sitting.\r\n");
    break;
  case POS_FIGHTING:
    if (FIGHTING(ch))
      sprintf(buf, "%sfighting %s.\r\n", buf, PERS(FIGHTING(ch), ch));
    else
      strcat(buf, "fighting thin air.\r\n");
    break;
  case POS_STANDING:
    if (IS_AFFECTED(ch, AFF_FLYING))
	strcat(buf, "flying.\r\n");
    else
        strcat(buf, "standing.\r\n");
    break;
  default:
    strcat(buf, "floating.\r\n");
    break;
  }

  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "    You are intoxicated.\r\n");

  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "    You are hungry.\r\n");

  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "    You are thirsty.\r\n");

  if (IS_AFFECTED(ch, AFF_BLIND))
    strcat(buf, "You have been blinded!\r\n");

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    strcat(buf, "You are invisible.\r\n");

/*  if (IS_AFFECTED(ch, AFF_DETECT_INVIS))
    strcat(buf, "You are sensitive to the presence of invisible things.\r\n");

  if (IS_AFFECTED(ch, AFF_SANCTUARY))
    strcat(buf, "You are protected by Sanctuary.\r\n");
*/

  if (IS_AFFECTED(ch, AFF_POISON))
    strcat(buf, "You are poisoned!\r\n");

  if (IS_AFFECTED(ch, AFF_CHARM))
    strcat(buf, "You have been charmed!\r\n");

  if (affected_by_spell(ch, SPELL_ARMOR))
    strcat(buf, "You feel protected.\r\n");

/*  if (IS_AFFECTED(ch, AFF_INFRAVISION))
    strcat(buf, "Your eyes are glowing red.\r\n");
*/
  if (!PRF_FLAGGED(ch, PRF_SUMMONABLE))
    strcat(buf, "You are not summonable by other players.\r\n");

  send_to_char(buf, ch);
}

ACMD(do_inventory)
{
  send_to_char("You are carrying:\r\n", ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}

ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("You are using:\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
	send_to_char(where[i], ch);
	show_obj_to_char(GET_EQ(ch, i), ch, 1);
	found = TRUE;
      } else {
	send_to_char(where[i], ch);
	send_to_char("Something.\r\n", ch);
	found = TRUE;
      }
    }
  }
  if (!found) {
    send_to_char(" Nothing.\r\n", ch);
  }
}


ACMD(do_time)
{
  char *suf;
  int weekday, day;
  extern struct time_info_data time_info;

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
}


ACMD(do_weather)
{
  static char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
  "lit by flashes of lightning"};

  if (OUTSIDE(ch)) {
    sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	    (weather_info.change >= 0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
    send_to_char(buf, ch);
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


ACMD(do_help)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_index;
  extern FILE *help_fl;
  extern char *help;

  int chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_index) {
    send_to_char("No help available.\r\n", ch);
    return;
  }
  bot = 0;
  top = top_of_helpt;

  for (;;) {
    mid = (bot + top) >> 1;
    minlen = strlen(argument);

    if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {

      /* trace backwards to find first matching entry. Thanks Jeff Fink! */
      while ((mid > 0) &&
	 (!(chk = strn_cmp(argument, help_index[mid - 1].keyword, minlen))))
	mid--;
      fseek(help_fl, help_index[mid].pos, SEEK_SET);
      *buf2 = '\0';
      for (;;) {
	fgets(buf, 128, help_fl);
	if (*buf == '#')
	  break;
	buf[strlen(buf) - 1] = '\0';	/* cleave off the trailing \n */
	strcat(buf2, strcat(buf, "\r\n"));
      }
      page_string(ch->desc, buf2, 1);
      return;
    } else if (bot >= top) {
      send_to_char("There is no help on that word.\r\n", ch);
      return;
    } else if (chk > 0)
      bot = ++mid;
    else
      top = --mid;
  }
}



#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-q] [-r] [-z]\r\n"

ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  int i, low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);	/* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 's':
	short_list = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(WHO_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */

  send_to_char("Players\r\n-------\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (showclass && !(showclass & GET_CLASS(tch)))
      continue;
    if (short_list) {
      sprintf(buf, "%s[%s %2d %s] %-12.12s%s%s",
	      (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
	      RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch), GET_NAME(tch),
	      (GET_LEVEL(tch) >= LVL_IMMORT ? CCNRM(ch, C_SPR) : ""),
	      ((!(++num_can_see % 4)) ? "\r\n" : ""));
      send_to_char(buf, ch);
    } else {
      num_can_see++;
      sprintf(buf1, "%s %s", GET_NAME(tch), GET_TITLE(tch));
      sprintf(buf, "%s[%s %2d %s] %-38.38s",
	      (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
	      RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch), buf1);

      strcat(buf, "[");
      if (PRF2_FLAGGED(tch, PRF2_BOUNTYHUNT))
	strcat(buf, "B");
      else if (PRF2_FLAGGED(tch, PRF2_ASSASSIN))
	strcat(buf, "A");
      else
	strcat(buf, "-");
      if (PRF2_FLAGGED(tch, PRF2_RETIRED)) {
	      if (PRF2_FLAGGED(tch, PRF2_WAR_DRUHARI))
	        strcat(buf, ".Vet-D");
	      else if (PRF2_FLAGGED(tch, PRF2_WAR_YLLANTRA))
	        strcat(buf, ".Vet-Y");
	      else
		strcat(buf, ".Vet-?");
      } else {
	      if (PRF2_FLAGGED(tch, PRF2_WAR_DRUHARI))
	        strcat(buf, ".Reg-D");
	      else if (PRF2_FLAGGED(tch, PRF2_WAR_YLLANTRA))
	        strcat(buf, ".Reg-Y");
	      else
		strcat(buf, ".Unreg");
      }
      if (PRF2_FLAGGED(tch, PRF2_ARENA_RED))
	strcat(buf, ".Red");
      else if (PRF2_FLAGGED(tch, PRF2_ARENA_BLUE))
	strcat(buf, ".Blu");
      else
	strcat(buf, ".---");

      strcat(buf, "](");
      if (PLR_FLAGGED(tch, PLR_THIEF))
	strcat(buf, "T");
      if (PLR_FLAGGED(tch, PLR_KILLER))
	strcat(buf, "K");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, "Q");
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	strcat(buf, "I");
      if (GET_INVIS_LEV(tch)) {
	sprintf(buf1, "i%d", GET_INVIS_LEV(tch));
	strcat(buf, buf1);
      }
      strcat(buf, ")");

      if (PRF2_FLAGGED(tch, PRF2_AFK))
	strcat(buf, " (AFK)");
      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " (writing)");
      else if (PLR_FLAGGED(tch, PLR_EDITING))
	strcat(buf, " (editing)");
      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " (notell)");
      if (GET_LEVEL(tch) >= LVL_IMMORT)
	strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }				/* endif shortlist */
  }				/* end of for */
  if (short_list && (num_can_see % 4))
    send_to_char("\r\n", ch);
  if (num_can_see == 0)
    sprintf(buf, "\r\nNo-one at all!\r\n");
  else if (num_can_see == 1)
    sprintf(buf, "\r\nOne lonely character displayed.\r\n");
  else
    sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
  send_to_char(buf, ch);
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
  extern char *connected_types[];
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  int low = 0, high = LVL_IMPL, i, num_can_see = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(USERS_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */
  strcpy(line,
	 "Num Class     Name         State          Idl Login@   Site\r\n");
  strcat(line,
	 "--- --------  ------------ -------------- --- -------- ------------------------\r\n");
  send_to_char(line, ch);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && playing)
      continue;
    if (!d->connected && deadweight)
      continue;
    if (!d->connected) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (showclass && !(showclass & GET_CLASS(tch)))
	continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	continue;

      if (d->original)
	sprintf(classname, "[%2d  %s]", GET_LEVEL(d->original),
		CLASS_ABBR(d->original));
      else
	sprintf(classname, "[%2d  %s]", GET_LEVEL(d->character),
		CLASS_ABBR(d->character));
    } else
      strcpy(classname, "     -     ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (!d->connected && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[d->connected]);

    if (d->character && !d->connected && GET_LEVEL(d->character) < LVL_GOD)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "N/A");

    format = "%3d %-7s %-12s %-14s %-3s %-8s ";

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(line, format, d->desc_num, classname,
		d->original->player.name, state, idletime, timeptr);
      else
	sprintf(line, format, d->desc_num, classname,
		d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
	      state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (d->connected) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
      send_to_char(line, ch);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  extern char circlemud_version[];

  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_BUGLIST:
    page_string(ch->desc, buglist, 0);
    break;
  case SCMD_IDEALIST:
    page_string(ch->desc, idealist, 0);
    break;
  case SCMD_TYPOLIST:
    page_string(ch->desc, typolist, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    break;
  case SCMD_VERSION:
    send_to_char(circlemud_version, ch);
    break;
  case SCMD_WHOAMI:
    send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
    break;
  default:
    return;
    break;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct descriptor_data *d;

  if (!*arg) {
    send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
	    (world[ch->in_room].zone == world[i->in_room].zone)) {
	  sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {			/* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next)
      if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
	  (i->in_room != NOWHERE) && isname(arg, i->player.name)) {
	sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	send_to_char(buf, ch);
	return;
      }
    send_to_char("No-one around by that name.\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
			        int recur)
{
  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%5d] %s\n\r",
	    world[obj->in_room].number, world[obj->in_room].name);
    send_to_char(buf, ch);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\n\r",
	    PERS(obj->carried_by, ch));
    send_to_char(buf, ch);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\n\r",
	    PERS(obj->worn_by, ch));
    send_to_char(buf, ch);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\n\r",
	    obj->in_obj->short_description, (recur ? ", which is" : " "));
    send_to_char(buf, ch);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\n\r");
    send_to_char(buf, ch);
  }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char("Players\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	  if (d->original)
	    sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
		    GET_NAME(i), world[d->character->in_room].number,
		 world[d->character->in_room].name, GET_NAME(d->character));
	  else
	    sprintf(buf, "%-20s - [%5d] %s\r\n", GET_NAME(i),
		    world[i->in_room].number, world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	sprintf(buf, "M%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
		world[i->in_room].number, world[i->in_room].name);
	send_to_char(buf, ch);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE);
      }
    if (!found)
      send_to_char("Couldn't find any such thing.\r\n", ch);
  }
}



ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i, j = GET_LEVEL(ch);

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  *buf = '\0';

  for (i = j; i < MIN(LVL_IMMORT, j + 10); i++) {
    sprintf(buf + strlen(buf), "[%2d] %8d-%-8d : ", i,
	    1 + titles[(int) GET_CLASS_NUM_FULL(ch)][i].exp, titles[(int) GET_CLASS_NUM_FULL(ch)][i + 1].exp);
    strcat(buf, titles[(int) GET_CLASS_NUM(ch)][i].title);
    strcat(buf, "\r\n");
  }
  send_to_char(buf, ch);
}


ACMD(do_consider)
{
  struct char_data *victim;
  int diff;

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Consider killing who?\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Easy!  Very easy indeed!\r\n", ch);
    return;
  }
  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
    return;
  }
  diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

  if (diff <= -10)
    send_to_char("Now where did that chicken go?\r\n", ch);
  else if (diff <= -5)
    send_to_char("You could do it with a needle!\r\n", ch);
  else if (diff <= -2)
    send_to_char("Easy.\r\n", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\r\n", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\r\n", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\r\n", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\r\n", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\r\n", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\r\n", ch);
  else if (diff <= 10)
    send_to_char("Are you mad!?\r\n", ch);
  else if (diff <= 100)
    send_to_char("You ARE mad!\r\n", ch);

}



ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    } else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}


static char *ctypes[] = {
"off", "sparse", "normal", "complete", "\n"};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  sprintf(buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  sprintf(buf,
	  "Hit Pnt Display: %-3s    "
	  " Gossip Channel: %-3s    "
	  " Summon Protect: %-3s\r\n"

	  "   Move Display: %-3s    "
	  "Auction Channel: %-3s    "
	  "       On Quest: %-3s\r\n"

	  "   Mana Display: %-3s    "
	  "  Grats Channel: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

          "   Coin Display: %-3s    "
	  " Combat Channel: %-3s    "
          "      AutoTitle: %-3s\r\n"

          "     XP Display: %-3s    "
	  "   Chat Channel: %-3s    "
          "        AutoSac: %-3s\r\n"

          " Damage Display: %-3s    "
	  "  Arena Channel: %-3s    "
          "  Auto Show Dir: %-3s\r\n"

	  "           Deaf: %-3s    "
	  "         NoTell: %-3s    "
	  " Auto Show Exit: %-3s\r\n"

	  "     Brief Mode: %-3s    "
	  "   Compact Mode: %-3s\r\n"

	  "     Wimp Level: %-3s    "
	  "    Color Level: %s\r\n"

	  " Radio Channels:",

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPGOLD)),
          ONOFF(PRF_FLAGGED(ch, PRF_NOWAR)),
	  ONOFF(!PLR_FLAGGED(ch, PLR_NOSETTITLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPXP)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOCHAT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSAC)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPDAM)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOTEAM)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTODIR)),

	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),

	  buf2,
	  ctypes[COLOR_LEV(ch)]);

  send_to_char(buf, ch);
  sprintbit(GET_CHANNEL(ch), channel_bits, buf);
  send_to_char(buf, ch);
}

struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;

void sort_commands(void)
{
  int a, b, tmp;
  char buf[128];

  ACMD(do_action);

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it includes the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  sprintf(buf, "The number of commands counted was: %d", num_of_cmds);
  log(buf);

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if (cmd_info[i].minimum_level >= 0 &&
	GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	(cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
      if (!(no % 7))
	strcat(buf, "\r\n");
      no++;
    }
  }

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}

/* functions and macros for 'scan' command */
void list_scanned_chars(struct char_data * list, struct char_data * ch,
 int distance, int door)
{
  const char *how_far[] = {
    "close by",
    "near by",
    "not too far"
    "a ways off",
    "far off to the"
  };

  struct char_data *i;
  int count = 0;
  *buf = '\0';

/* this loop is a quick, easy way to help make a grammatical sentence
   (i.e., "You see x, x, y, and z." with commas, "and", etc.) */

  for (i = list; i; i = i->next_in_room)

/* put any other conditions for scanning someone in this if statement -
   i.e., if (CAN_SEE(ch, i) && condition2 && condition3) or whatever */

    if (CAN_SEE(ch, i))
     count++;

  if (!count)
    return;

  for (i = list; i; i = i->next_in_room) {

/* make sure to add changes to the if statement above to this one also, using
   or's to join them.. i.e., 
   if (!CAN_SEE(ch, i) || !condition2 || !condition3) */

    if (!CAN_SEE(ch, i))
      continue; 
    if (!*buf)
      sprintf(buf, "You see %s", GET_NAME(i));
    else 
      sprintf(buf, "%s%s", buf, GET_NAME(i));
    if (--count > 1)
      strcat(buf, ", ");
    else if (count == 1)
      strcat(buf, " and ");
    else {
      sprintf(buf2, " %s %s.\r\n", how_far[distance], dirs[door]);
      strcat(buf, buf2);      
    }
    
  }
  send_to_char(buf, ch);
}

ACMD(do_scan)
{
  /* >scan
     You quickly scan the area.
     You see John, a large horse and Frank close by north.
     You see a small rabbit a ways off south.
     You see a huge dragon and a griffon far off to the west.

  */
  int door;
  
  *buf = '\0';
  
  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  /* may want to add more restrictions here, too */
  send_to_char("You quickly scan the area.\r\n", ch);
  for (door = 0; door < NUM_OF_DIRS - 2; door++) /* don't scan up/down */
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (world[EXIT(ch, door)->to_room].people) {
	list_scanned_chars(world[EXIT(ch, door)->to_room].people, ch, 0, door);
      } else if (_2ND_EXIT(ch, door) && _2ND_EXIT(ch, door)->to_room != 
		 NOWHERE && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_CLOSED)) {
   /* check the second room away */
	if (world[_2ND_EXIT(ch, door)->to_room].people) {
	  list_scanned_chars(world[_2ND_EXIT(ch, door)->to_room].people, ch, 1, door);
	} else if (_3RD_EXIT(ch, door) && _3RD_EXIT(ch, door)->to_room !=
		   NOWHERE && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_CLOSED)) {
	  /* check the third room */
	  if (world[_3RD_EXIT(ch, door)->to_room].people) {
	    list_scanned_chars(world[_3RD_EXIT(ch, door)->to_room].people, ch, 2, door);
	  }

	}
      }
    }                
}
