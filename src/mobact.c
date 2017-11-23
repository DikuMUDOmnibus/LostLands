/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];

void hunt_victim(struct char_data * ch);

void mprog_random_trigger(struct char_data * mob);
void mprog_wordlist_check(char *arg, struct char_data * mob, struct char_data * actor,
                          struct obj_data * obj, void *vo, int type);
extern int is_empty(int zone_nr);

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

void mobile_activity(void)
{
  register struct char_data *ch, *next_ch, *vict;
  struct obj_data *obj, *best_obj;
  int door, found, max;
  memory_rec *names;

  extern int no_specials;

  ACMD(do_get);
  ACMD(do_say);

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (ch->desc || !IS_MOB(ch) || FIGHTING(ch) || !AWAKE(ch) || AFF_FLAGGED(ch, AFF_CHARM))
      continue;

    /* Examine call for special procedure */
    if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
      if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
	sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
		GET_NAME(ch), GET_MOB_VNUM(ch));
	log(buf);
	REMOVE_BIT(MOB_FLAGS(ch), MOB_SPEC);
      } else {
	if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
	  continue;		/* go to next char */
      }
    }

    if(!is_empty(world[ch->in_room].zone)) mprog_random_trigger(ch);

    /* Scavenger (picking up objects) */
    if (MOB_FLAGGED(ch, MOB_SCAVENGER)) {
      if (world[ch->in_room].contents && !number(0, 10)) {
	max = 1;
	best_obj = NULL;
	for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
	  if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
	    best_obj = obj;
	    max = GET_OBJ_COST(obj);
	  }
	if (best_obj != NULL) {
	  obj_from_room(best_obj);
	  obj_to_char(best_obj, ch);
	  do_say(ch, "So much junk around here!!!", 0, 0);
	  act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	}
      }
    }
    /* Mob Movement */
    if (!MOB_FLAGGED(ch, MOB_SENTINEL) && (GET_POS(ch) == POS_STANDING) &&
	((door = number(0, 18)) < NUM_OF_DIRS) && CAN_GO(ch, door) &&
	!ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
	(!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
	 (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
      perform_move(ch, door, 1);
    }

   /* MOB Prog foo */
   if(IS_NPC(ch) && ch->mpactnum > 0 && !is_empty(world[ch->in_room].zone)) {
      MPROG_ACT_LIST *tmp_act, *tmp2_act;
      for(tmp_act = ch->mpact; tmp_act != NULL; tmp_act=tmp_act->next) {
             mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch,
                           tmp_act->obj, tmp_act->vo, ACT_PROG);
             free(tmp_act->buf);
      }
      for(tmp_act = ch->mpact; tmp_act != NULL; tmp_act=tmp2_act) {
             tmp2_act = tmp_act->next;
             free(tmp_act);
      }
      ch->mpactnum = 0;
      ch->mpact = NULL;
    }

    /* Aggressive Mobs */
    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	  continue;
/*	if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
	  continue;
*/	if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
	  if (GET_RACE(ch) != GET_RACE(vict)) {
		  hit(ch, vict, TYPE_HIT);
		  found = TRUE;
          } else {
		do_say(ch, "Ah, you're a friend.", 0, 0);		
	}
      }
    }
    /* Hunting Mobs */
    if (HUNTING(ch))
	hunt_victim(ch);

    /* Mob Memory */
    if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	  continue;
	for (names = MEMORY(ch); names && !found; names = names->next)
	  if (names->id == GET_IDNUM(vict)) {
	    found = TRUE;
	    if (MOB3_FLAGGED(ch, MOB3_CANTALK))
		    do_say(ch, "Hey!  You're the fiend that attacked me!!!", 0, 0);
	    else {
		    act("$n growls at $N and prepares to attack!!", 
			FALSE, ch, 0, vict, TO_ROOM);
		    act("$n growls at you and lunges for your throat!!!",
			FALSE, ch, 0, vict, TO_VICT);
	    }
	    hit(ch, vict, TYPE_HIT);
	  }
        }
      }
    }

    /* Helper Mobs */
    if (MOB_FLAGGED(ch, MOB_HELPER)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	if (ch != vict && IS_NPC(vict) && FIGHTING(vict) &&
            !IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict) && 
	    GET_RACE(ch) != GET_RACE(FIGHTING(vict)) ) {
	  act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
	  hit(ch, FIGHTING(vict), TYPE_HIT);
	  found = TRUE;
	}
      }
    }
    /* Add new mobile actions here */

  }				/* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if (!IS_NPC(ch) || IS_NPC(victim))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
void forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev = NULL;

  if (!(curr = MEMORY(ch)))
    return;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return;			/* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  free(curr);
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}
