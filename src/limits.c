/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"

#define READ_TITLE(ch) 	titles[(int)GET_CLASS_NUM(ch)][(int)GET_LEVEL(ch)].title

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[NUM_CLASSES][LVL_IMPL + 1];
extern struct room_data *world;
extern int max_exp_gain;
extern int max_exp_loss;


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = graf(age(ch).year, 4, 8, 12, 16, 12, 10, 8);

    /* Class calculations */
    gain += lvD3(ch);    
    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain <<= 1;
      break;
    case POS_RESTING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    }

    if (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_AVATAR(ch) || IS_WIZARD(ch))
      gain <<= 1;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_MANA))
    gain <<= 1;

  return (gain);
}


int hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
    if (MOB_FLAGGED(ch, MOB_FASTREGEN))
	gain <<=2;
    /* Neat and fast */
  } else {

    gain = graf(age(ch).year, 8, 12, 20, 32, 16, 10, 4);

    /* Class/Level calculations */
    gain += lvD3(ch);    
    /* Skill/Spell calculations */

    /* Position calculations    */

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);	/* Divide by 8 */
      break;
    }

    if (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_WIZARD(ch) || IS_AVATAR(ch))
      gain >>= 1;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_HEAL))
    gain <<= 1;

  if (IS_SET(AFF2_FLAGS(ch), AFF2_CRIT_HIT))
    gain >>= 1;

  return (gain);
}

int move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    return (GET_LEVEL(ch));
    /* Neat and fast */
  } else {
    gain = graf(age(ch).year, 16, 20, 24, 20, 16, 12, 10);

    /* Class/Level calculations */

    /* Skill/Spell calculations */


    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);	/* Divide by 8 */
      break;
    }
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_MOVE))
    gain <<= 1;

  if (IS_SET(AFF2_FLAGS(ch), AFF2_CRIT_HIT))
    gain >>= 1;

  return (gain);
}


void set_title(struct char_data * ch, char *title)
{
  if (title == NULL)
    title = READ_TITLE(ch);

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}


void check_autowiz(struct char_data * ch)
{
  char buf[100];
  extern int use_autowiz;
  extern int min_wizlist_lev;
  pid_t getpid(void);

  if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}

void gain_exp(struct char_data * ch, int gain) {
  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (!IS_NPC(ch)) {
	if (gain > 0) {
		GET_EXP(ch) += MIN(max_exp_gain, (int) (gain / GET_NUM_OF_CLASS(ch)));
	} else {/* gain < 0 */
	        GET_EXP(ch) += MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
	        if (GET_EXP(ch) < 0)
		        GET_EXP(ch) = 0;
        }		
  } else {
	GET_EXP(ch) += gain;
  }
  return;
}

void advance_char(struct char_data * ch) {
  int target_xp;

  if (GET_LEVEL(ch) < (LVL_IMMORT -1)) {  /* able to gain */
	target_xp = titles[(int) GET_CLASS_NUM_FULL(ch)][GET_LEVEL(ch) +1].exp;
	if (GET_EXP(ch) >= target_xp) { /* advance one level */
	      GET_LEVEL(ch) += 1;
	      advance_level(ch);
              send_to_char("Congratulations, you've gain a level!\r\n", ch);
	      target_xp = titles[(int) GET_CLASS_NUM_FULL(ch)][GET_LEVEL(ch) +1].exp;
	      if (GET_EXP(ch) >= target_xp) 
		GET_EXP(ch) = target_xp - 1;
	      if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
		set_title(ch, NULL);
        } else
	      send_to_char("You are not ready to gain a level yet.\r\n", ch);
  } else {/* immort */
	target_xp = titles[(int) GET_CLASS_NUM_FULL(ch)][GET_LEVEL(ch) +1].exp;    

        if ((GET_REINCARN(ch) >= 8) && (GET_EXP(ch) >= target_xp)) {
		GET_LEVEL(ch) += 1;
		advance_level(ch);
		send_to_char("Welcome to Immortality!\r\n", ch);
   	        if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
			set_title(ch, NULL);		
	        check_autowiz(ch);
        } else {
	      send_to_char("You must progress further along the Golden Path\r\n", ch);
       	      send_to_char("before reaching Immortality.\r\n", ch);
       }
  } 
}

void gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  GET_EXP(ch) += (int) (gain / GET_NUM_OF_CLASS(ch));
  if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;

  if (!IS_NPC(ch)) {
    while (GET_LEVEL(ch) < LVL_IMPL &&
	GET_EXP(ch) >= titles[(int) GET_CLASS_NUM_FULL(ch)][GET_LEVEL(ch) + 1].exp) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }
    if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
      set_title(ch, NULL);
    check_autowiz(ch);
    }
  }
}


void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1)	/* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(72, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING | PLR_EDITING))
    return;

  switch (condition) {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default:
    break;
  }

}

void check_idling(struct char_data * ch)
{
  extern int free_rent;
  void Crash_rentsave(struct char_data *ch, int cost);

  if (++(ch->char_specials.timer) > 12)
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    } else if (ch->char_specials.timer > 60) {
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc)
	close_socket(ch->desc);
      ch->desc = NULL;
      if (free_rent)
	Crash_rentsave(ch, 0);
      else
	Crash_idlesave(ch);
      sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
      mudlog(buf, CMP, LVL_GOD, TRUE);
      extract_char(ch);
    }
}



/* Update PCs, NPCs, and objects */
void point_update(void)
{
  void update_char_objects(struct char_data * ch);	/* handler.c */
  void extract_obj(struct obj_data * obj);	/* handler.c */
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    if (GET_POS(i) >= POS_STUNNED) {
      if (!AFF_FLAGGED(i, AFF_DEATHDANCE)) {
	GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
      	GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      	GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      }
      if (IS_AFFECTED(i, AFF_POISON))
	damage(i, i, 2, SPELL_POISON, 0);
      if (GET_POS(i) <= POS_STUNNED)
	update_pos(i);
    } else if (GET_POS(i) == POS_INCAP)
      damage(i, i, 1, TYPE_SUFFERING, 0);
    else if (GET_POS(i) == POS_MORTALLYW)
      damage(i, i, 2, TYPE_SUFFERING, 0);
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) < LVL_GOD)
	check_idling(i);
    }
    if (AFF2_FLAGGED(i, AFF2_BURNING) &&
	SECT(i->in_room) == SECT_UNDERWATER) {
      REMOVE_BIT(AFF2_FLAGS(i), AFF2_BURNING);
    }
    if (AFF2_FLAGGED(i, AFF2_ACIDED) &&
	SECT(i->in_room) == SECT_UNDERWATER) {
      REMOVE_BIT(AFF2_FLAGS(i), AFF2_ACIDED);
    }
    if (AFF2_FLAGGED(i, AFF2_FREEZING) &&
	SECT(i->in_room) == SECT_LAVA) {
      REMOVE_BIT(AFF2_FLAGS(i), AFF2_FREEZING);
    }

    if (AFF2_FLAGGED(i, AFF2_BURNING) && !(AFF_FLAGGED(i, AFF_PROT_FIRE))) {
      send_to_char("You're burning!!\r\n", i);
      GET_HIT(i) -= LAVA_DAMAGE;
    }
    if (AFF2_FLAGGED(i, AFF2_FREEZING) && !(AFF2_FLAGGED(i, AFF2_PROT_COLD))) {
      send_to_char("You're freezing!!\r\n", i);
      GET_HIT(i) -= LAVA_DAMAGE;
    }
    if (AFF2_FLAGGED(i, AFF2_ACIDED) && !(AFF2_FLAGGED(i, AFF2_STONESKIN))) {
      send_to_char("You're dissolving!!\r\n", i);
      GET_HIT(i) -= LAVA_DAMAGE;
    }
    if ((SECT(i->in_room) == SECT_UNDERWATER) &&
	     !AFF_FLAGGED(i, AFF_WATERBREATH)) {
	send_to_char("You're drowning!!\r\n", i);
        GET_HIT(i) -= UNWAT_DAMAGE;
    }
    if (AFF2_FLAGGED(i, AFF2_CRIT_HIT)) {
	send_to_char("You're bleeding critically!!!\r\n", i);
	GET_HIT(i) -= CRIT_DAMAGE;
    }
    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;	/* Next in object list */

    /* If this is a corpse */
    if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3)) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
	GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {

	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	}
	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content;	/* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj, j->in_room);
	  else
	    assert(FALSE);
	}
	extract_obj(j);
      }
    }
  }
}
