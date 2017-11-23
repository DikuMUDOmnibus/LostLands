#define __TELEPORT_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "rooms.h"
#include "objs.h"
#include "class.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "comm.h"

extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct room_data *world;

void TeleportPulseStuff(int pulse)
{
  ACMD(do_look);
   int real_room(int virtual);
  /*
    check_mobile_activity(pulse);
    Teleport(pulse);
    */

  bool flag = FALSE;
  register struct char_data *ch;
  struct char_data *next, *tmp, *bk;
  int troom = 0, sroom = 0;
  struct obj_data *obj_object, *temp_obj;

/*  tm = pulse % PULSE_MOBILE;    this is dependent on P_M = 3*P_T */

/*
  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT*2) {
    tick = 2;
  }
*/

  for (ch = character_list; ch; ch = next) {
    next = ch->next;
    /* if (IS_MOB(ch)) {
     * if (ch->specials.tick == tick) {
     *   mobile_activity(ch);
     * }
     * } else */
    if (!IS_NPC(ch)) {
      if (!world[ch->in_room].number || world[ch->in_room].tele == NULL) 
	return;
      if (world[ch->in_room].number == world[ch->in_room].tele->targ)
	return;
      if (IS_SET(world[ch->in_room].tele->mask, TELE_OBJ)) {
	flag = TRUE;
	temp_obj = ch->carrying;
	while (flag && temp_obj) {
		if (GET_OBJ_VNUM(temp_obj) == world[ch->in_room].tele->obj)
			flag = FALSE;
		temp_obj = temp_obj->next;
	}
      } else if (IS_SET(world[ch->in_room].tele->mask, TELE_NOOBJ)) {
	flag = FALSE;
	temp_obj = ch->carrying;
	while (!flag && temp_obj) {
		if (GET_OBJ_VNUM(temp_obj) == world[ch->in_room].tele->obj)
			flag = TRUE;
		temp_obj = temp_obj->next;
	}
      }
      temp_obj = NULL;
      if (flag || world[ch->in_room].tele->time <= 0 ||
          (pulse % world[ch->in_room].tele->time) !=0) {
	return;
      }
      if (real_room(world[ch->in_room].tele->targ) == -1) {
          log("invalid tele->targ");
          continue;
      }
      troom = real_room(world[ch->in_room].tele->targ);
      sroom = real_room(world[ch->in_room].number);
      obj_object = world[troom].contents;
      while (obj_object) {
          temp_obj = obj_object->next_content;
          obj_from_room(obj_object);
          obj_to_room(obj_object, troom);
          obj_object = temp_obj;
      }

      bk = 0;

      while(world[sroom].people/* should never fail */) {

          tmp = world[sroom].people;   /* work through the list of people */
          if (!tmp) break;

          if (tmp == bk) break;

          bk = tmp;

	  if (!IS_SET(world[sroom].tele->mask, TELE_NOMSG)) {
		act("$n disappears.", FALSE, ch, 0, 0, TO_ROOM);
	  }
          char_from_room(tmp); /* the list of people in the room has changed */
          char_to_room(tmp, troom);
	  if (!IS_SET(world[sroom].tele->mask, TELE_NOMSG)) {
		act("$n appears from out of nowhere.", FALSE, ch, 0, 0, TO_ROOM);
	  }

          if (IS_SET(world[sroom].tele->mask, TELE_LOOK) && !IS_NPC(tmp)) {
            do_look(tmp, "\0",15, 0);
	     /*look_at_room(ch, 0);*/
          }

          if (IS_SET(world[troom].room_flags, ROOM_DEATH) && 
		(GET_LEVEL(tmp))  < LVL_IMMORT) {
            if (tmp == next)
              next = tmp->next;
	      log_death_trap(tmp);
	     /*death_cry(tmp);*/
	     extract_char(tmp);
	     /*            NailThisSucker(tmp);*/
            continue;
          }
          /* if (dest->sector_type == SECT_AIR) {
           * n2 = tmp->next;
           * if (check_falling(tmp)) {
           *   if (tmp == next)
           *     next = n2;
           * }
           * } hmm.... maybe I should add this as welll*/
        }

        if (IS_SET(world[sroom].tele->mask, TELE_COUNT)) {
          world[sroom].tele->time = 0;   /* reset it for next count */
        }
        if (IS_SET(world[sroom].tele->mask, TELE_RANDOM)) {
          world[sroom].tele->time = number(1,10)*100;
      }
    }
  }
}
