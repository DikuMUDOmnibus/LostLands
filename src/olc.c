#include <stdio.h>
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "interpreter.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include <ctype.h>
#include <stdlib.h>
#include "olc.h"
#include "zedit.h"

extern struct obj_data *obj_proto;
extern struct room_data *world;
extern int top_of_zone_table;
extern struct zone_data *zone_table;

int get_zone_perms(struct char_data * ch, int z_num)
{
  int allowed = FALSE;
  FILE *old_file;
  char *old_fname, line[256];
  char arg3[40], arg4[40], arg5[40], arg6[40];

  if(GET_LEVEL(ch) < LVL_IMPL) {
    sprintf(buf1, "%s/%i.zon", ZON_PREFIX, z_num);
    old_fname = buf1;

    if(!(old_file = fopen(old_fname, "r"))) {
      sprintf(buf, "Error: Could not open %s for read.\r\n", old_fname);
      send_to_char(buf, ch);
      return(-1);
    }
    do {
      get_line2(old_file, line);
      if (sscanf(line, " %s %s %s %s", arg3, arg4, arg5, arg6) == 4) {
        if (*arg3 == '#') {			/* creator */
	  if(isname(ch->player.name, arg4)) {
		  allowed = TRUE;
          }
        }            
      } else if (*arg3 == '*') {		/* builder */
	  if(isname(ch->player.name, arg5)) {
		  allowed = TRUE;
	  }
      }
    } while(*line != 'S' && !allowed);
    fclose(old_file);

    if(allowed == FALSE) {
      sprintf(buf, "You don't have builder rights in zone #%i\r\n",
        z_num);
      send_to_char(buf, ch);
      return(-1);
    }
  }
  return(z_num);  
}


/*
 * the redit ACMD function 
 */
ACMD (do_redit)
{
  int number;
  int room_num;
  struct descriptor_data *d;
  char arg1[MAX_INPUT_LENGTH];
  struct room_data *room;
  struct teleport_data *new_tele;
  struct broadcast_data *new_broad;
  int counter, found = 0;

  
  one_argument (argument, arg1);
  if (!arg1)
    {
      send_to_char ("Specify a room number to edit.\r\n", ch);
      return;
    }
  if (!isdigit (*arg1))
    {
      send_to_char ("Please supply a valid number.\r\n", ch);
      return;
    }
  number = atoi (arg1);
  room_num = real_room (number);

  for (counter = 0; counter <= top_of_zone_table; counter++)
    {
      if ((number >= (zone_table[counter].number * 100)) &&
	  (number <= (zone_table[counter].top)))
	{
	  ch->desc->edit_zone = counter;
	  found = 1;
	  break;
	}
    }
  if (!found)
    {
      send_to_char ("Sorry, that number is not part of any zone!\r\n", ch);
      return;
    }
  if (get_zone_perms(ch, zone_table[counter].number) == -1)
	return;

  SET_BIT (PLR_FLAGS (ch), PLR_EDITING);
  d = ch->desc;
  STATE (d) = CON_REDIT;
  d->edit_number = number;
  if (room_num >= 0)
    {
      send_to_char ("A room already exists at that number. Do you wish to edit it?\r\n", ch);
      CREATE (room, struct room_data, 1);
      CREATE (new_tele, struct teleport_data, 1);
      CREATE (new_broad, struct broadcast_data, 1);
      *room = world[room_num];
      room->tele = new_tele;
      room->broad = new_broad;
      if (world[room_num].tele != NULL) {
	      room->tele->targ = world[room_num].tele->targ;
	      room->tele->time = world[room_num].tele->time;
	      room->tele->mask = world[room_num].tele->mask;
	      room->tele->cnt = world[room_num].tele->cnt;
	      room->tele->obj = world[room_num].tele->obj;
      } else {
	      room->tele->targ = 0;
	      room->tele->time = 0;
              room->tele->mask = 0;
              room->tele->cnt  = 0;
              room->tele->obj  = 0;
      }
      if (world[room_num].broad != NULL) {
		room->broad->channel = world[room_num].broad->channel;
		room->broad->targ1 = world[room_num].broad->targ1;
		room->broad->targ2 = world[room_num].broad->targ2;
      } else {
		room->broad->channel = 0;
		room->broad->targ1 = 0;
		room->broad->targ2 = 0;
      }
      /* allocate space for all strings  */
      if (world[room_num].name)
	room->name = str_dup (world[room_num].name);
      if (world[room_num].description)
	room->description = str_dup (world[room_num].description);
      /* exits - alloc only if necessary */
      for (counter = 0; counter < NUM_OF_DIRS; counter++)
	{
	  if (world[room_num].dir_option[counter])
	    {
	      CREATE(room->dir_option[counter], struct room_direction_data, 1);
	      /* copy numbers over */
	      *room->dir_option[counter] = *world[room_num].dir_option[counter];
	      /* malloc strings */
	      if (world[room_num].dir_option[counter]->general_description)
		room->dir_option[counter]->general_description =
		  str_dup(world[room_num].dir_option[counter]->general_description);
	      if (world[room_num].dir_option[counter]->keyword)
		room->dir_option[counter]->keyword =
		  str_dup(world[room_num].dir_option[counter]->keyword);
	    }
	}
      if (world[room_num].ex_description) {
	struct extra_descr_data *this, *temp, *temp2;
	
	CREATE (temp, struct extra_descr_data, 1);
	room->ex_description = temp;
	for (this = world[room_num].ex_description;
	     this; this = this->next)
	  {
	    if (this->keyword)
	      temp->keyword = str_dup (this->keyword);
	    if (this->description)
	      temp->description = str_dup (this->description);
	    if (this->next)
	      {
		CREATE (temp2, struct extra_descr_data, 1);
		temp->next = temp2;
		temp = temp2;
	      }
	    else
	      temp->next = NULL;
	  }
      }
      d->edit_room = room;
      d->edit_mode = REDIT_CONFIRM_EDIT;
      return;
    }
  else
    {
      send_to_char ("That room does not exist, create it?\r\n", ch);
      /*
       * create dummy room 
       */
      CREATE(d->edit_room, struct room_data, 1);
      d->edit_room->name = str_dup("An unfinished room");
      d->edit_room->description = str_dup("You are in an unfinished room.\r\n");

      d->edit_mode = REDIT_CONFIRM_EDIT;
      return;
    }
}

/*
 * the iedit ACMD function 
 */
ACMD (do_iedit)
{
  int number;
  int obj_num;
  struct descriptor_data *d;
  char arg1[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  int counter, found = 0;

  one_argument (argument, arg1);

  /*
   * if no argument 
   */
  if (!arg1)
    {
      send_to_char ("Specify an object number to edit.\r\n", ch);
      return;
    }
  /*
   * is argument a number? 
   */
  if (!isdigit (*arg1))
    {
      send_to_char ("Please supply a valid number.\r\n", ch);
      return;
    }
  /*
   * check if number already exists 
   */
  number = atoi (arg1);
  obj_num = real_object (number);

  /*
   * check zone numbers 
   */
  for (counter = 0; counter <= top_of_zone_table; counter++)
    {
      if ((number >= (zone_table[counter].number * 100)) &&
	  (number <= (zone_table[counter].top)))
	{
	  ch->desc->edit_zone = counter;
	  found = 1;
	  break;
	}
    }
  if (!found)
    {
      send_to_char ("Sorry, that number is not part of any zone.\r\n", ch);
      return;
    }
  if (get_zone_perms(ch, zone_table[counter].number) == -1)
	return;
  /*
   * put guy into editing mode 
   */
  SET_BIT (PLR_FLAGS (ch), PLR_EDITING);
  /*
   * now start playing with the descriptor 
   */
  d = ch->desc;
  STATE (d) = CON_IEDIT;

  d->edit_number = number;	/*
				 * the VNUM not the REAL NUMBER 
				 */

  if (obj_num >= 0)
    {
      struct extra_descr_data *this, *temp, *temp2;
      send_to_char ("An object already exists at that number. Do you wish to edit it?\r\n", ch);
      /*
       * allocate object 
       */
      CREATE (obj, struct obj_data, 1);
      clear_object (obj);
      *obj = obj_proto[obj_num];	/*
					 * the RNUM 
					 */
      /*
       * copy all strings over 
       */
      if (obj_proto[obj_num].name)
	obj->name = str_dup (obj_proto[obj_num].name);
      if (obj_proto[obj_num].short_description)
	obj->short_description = str_dup (obj_proto[obj_num].short_description);
      if (obj_proto[obj_num].description)
	obj->description = str_dup (obj_proto[obj_num].description);
      if (obj_proto[obj_num].action_description)
	obj->action_description = str_dup (obj_proto[obj_num].description);
      if (obj_proto[obj_num].ex_description)
	{
	  /*
	   * temp is for obj being edited 
	   */
	  CREATE (temp, struct extra_descr_data, 1);
	  obj->ex_description = temp;
	  for (this = obj_proto[obj_num].ex_description;
	       this; this = this->next)
	    {
	      if (this->keyword)
		temp->keyword = str_dup (this->keyword);
	      if (this->description)
		temp->description = str_dup (this->description);
	      if (this->next)
		{
		  CREATE (temp2, struct extra_descr_data, 1);
		  temp->next = temp2;
		  temp = temp2;
		}
	      else
		temp->next = NULL;
	    }
	}
      d->edit_obj = obj;
      d->edit_mode = IEDIT_CONFIRM_EDIT;
      return;
    }
  else
    {
      send_to_char ("That object does not exist, create it?\r\n", ch);
      /*
       * create dummy object! 
       */
      CREATE (d->edit_obj, struct obj_data, 1);
      clear_object (d->edit_obj);
      d->edit_obj->name = str_dup ("unfinished object");
      d->edit_obj->description = str_dup ("An unfinished object is lying here.");
      d->edit_obj->short_description = str_dup ("an unfinished object");
      GET_OBJ_WEAR (d->edit_obj) = ITEM_WEAR_TAKE;
      d->edit_mode = IEDIT_CONFIRM_EDIT;
      return;
    }
}
/*
 * the zedit ACMD function 
 */
ACMD (do_zedit)
{
  int rznum = 0;
  int vznum = 0;
  struct descriptor_data *d;
  char arg1[MAX_INPUT_LENGTH];
  bool found = FALSE;

  one_argument (argument, arg1);

  /*
   * if no argument 
   */
  if (!arg1)
    {
      send_to_char ("Specify a zone number to edit.\r\n", ch);
      return;
    }
  /*
   * is argument a number? 
   */
  if (!isdigit (*arg1))
    {
      send_to_char ("Please supply a valid number.\r\n", ch);
      return;
    }
  /*
   * check if number already exists 
   */
  vznum = atoi (arg1);

  for (rznum = 0; rznum <= top_of_zone_table; rznum++)
    {
      if (vznum == zone_table[rznum].number)
	{
	  ch->desc->edit_zone = rznum;
	  found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      sprintf(arg1, "Sorry, %d is not a valid zone number.\r\n", vznum);
      send_to_char (arg1, ch);
      return;
    }
  if (get_zone_perms(ch, zone_table[rznum].number) == -1)
	return;
  /*
   * put guy into editing mode 
   */
  SET_BIT (PLR_FLAGS (ch), PLR_EDITING);
  /*
   * now start playing with the descriptor 
   */
  send_to_char("Are you sure you want to do this zedit this zone? ", ch);
  d = ch->desc;
  STATE (d) = CON_ZEDIT;

  d->edit_number = vznum;	/*
				 * the VNUM not the REAL NUMBER 
				 */

  d->edit_mode = ZEDIT_CONFIRM_EDIT;
}
