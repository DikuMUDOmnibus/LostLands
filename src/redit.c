#include <stdio.h>
#include <ctype.h>
#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include <stdlib.h>
#include "boards.h"
#include "olc.h"
#include "constants.h"

/* change this depending on the number of flags you have defined */
#define NUM_ROOM_SECTORS 12
#define NUM_ROOM_FLAGS   20
#define TELEPORT_NUM	  7

extern int      top_of_world;
extern struct room_data *world;
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct zone_data *zone_table;
extern sh_int r_mortal_start_room;
extern sh_int r_immort_start_room;
extern sh_int r_frozen_start_room;
extern sh_int mortal_start_room;
extern sh_int immort_start_room;
extern sh_int frozen_start_room;

/* function protos */
void redit_disp_extradesc_menu(struct descriptor_data * d);
void redit_disp_exit_menu(struct descriptor_data * d);
void redit_disp_exit_flag_menu(struct descriptor_data * d);
void redit_disp_teleport_menu(struct descriptor_data * d);
void redit_disp_flag_menu(struct descriptor_data * d);
void redit_disp_sector_menu(struct descriptor_data * d);
void redit_disp_menu(struct descriptor_data * d);
void redit_parse(struct descriptor_data * d, char *arg);

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* For extra descriptions */
void redit_disp_extradesc_menu(struct descriptor_data * d)
{
  struct extra_descr_data *extra_desc =
  (struct extra_descr_data *) * d->misc_data;

  send_to_char("Extra desc menu\r\n", d->character);
  send_to_char("0) Quit\r\n", d->character);
  sprintf(buf, "1) Keyword: %s\r\n", extra_desc->keyword
	  ? extra_desc->keyword : "<NONE>");
  send_to_char(buf, d->character);
  sprintf(buf, "2) Description:\r\n%s\r\n", extra_desc->description ?
	  extra_desc->description : "<NONE>");
  send_to_char(buf, d->character);
  if (!extra_desc->next)
    send_to_char("3) <NOT SET>\r\n", d->character);
  else
    send_to_char("3) Set. <NOT VIEWED>\r\n", d->character);
  send_to_char("Enter choice:\r\n", d->character);
  d->edit_mode = REDIT_EXTRADESC_MENU;
}

/* For exits */
void redit_disp_exit_menu(struct descriptor_data * d)
{
  /* if exit doesn't exist, alloc/create it */
  if(!d->edit_room->dir_option[d->edit_number2])
    CREATE(d->edit_room->dir_option[d->edit_number2],
	   struct room_direction_data, 1);
  sprintf(buf, "1) Exit to :     %d\r\n",
	  d->edit_room->dir_option[d->edit_number2]->to_room_vnum);
  send_to_char(buf, d->character);
  sprintf(buf, "2) Description : %s\r\n",
	  d->edit_room->dir_option[d->edit_number2]->general_description ?
	  d->edit_room->dir_option[d->edit_number2]->general_description :
	  "<NONE>");
  send_to_char(buf, d->character);
  sprintf(buf, "3) Door name :   %s\r\n",
	  d->edit_room->dir_option[d->edit_number2]->keyword ?
	  d->edit_room->dir_option[d->edit_number2]->keyword :
	  "<NONE>");
  send_to_char(buf, d->character);
  sprintf(buf, "4) Key :         %d\r\n",
	  d->edit_room->dir_option[d->edit_number2]->key);
  send_to_char(buf, d->character);
  /* weird door handling! */
  if (IS_SET(d->edit_room->dir_option[d->edit_number2]->exit_info,
	     EX_ISDOOR)) {
    if (IS_SET(d->edit_room->dir_option[d->edit_number2]->exit_info,
	       EX_PICKPROOF))
      strcpy(buf2, "Pickproof door");
    else
      strcpy(buf2, "Normal door");
  } else
    strcpy(buf2, "No door");
  if (IS_SET(d->edit_room->dir_option[d->edit_number2]->exit_info,
		EX_HIDDEN))
	sprintf(buf2, "%s; Hidden", buf2);
  sprintf(buf, "5) Door flags :  %s\r\n",
	  buf2);
  send_to_char(buf, d->character);
  send_to_char("6) Purge exit.\r\n", d->character);
  send_to_char("Enter choice, 0 to quit:", d->character);
  d->edit_mode = REDIT_EXIT_MENU;
}

/* For exit flags */
void redit_disp_exit_flag_menu(struct descriptor_data * d)
{
  send_to_char("0) No door\r\n", d->character);
  send_to_char("1) Closeable door\r\n", d->character);
  send_to_char("2) Pickproof\r\n", d->character);
  send_to_char("3) Hidden passage\r\n", d->character);
  send_to_char("4) Hidden door\r\n", d->character);
  send_to_char("5) Hidden Pickproof door\r\n", d->character);
  send_to_char("Enter choice:", d->character);
}

/* For room flags */
void redit_disp_flag_menu(struct descriptor_data * d)
{
  int             counter;

  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_ROOM_FLAGS; counter += 2) {
    sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
	    counter + 1, room_bits[counter],
	    counter + 2, counter + 1 < NUM_ROOM_FLAGS ?
	    room_bits[counter + 1] : "");
    send_to_char(buf, d->character);
  }
  sprintbit(d->edit_room->room_flags, room_bits, buf1);
  sprintf(buf, "Room flags: %s\r\n", buf1);
  send_to_char(buf, d->character);
  send_to_char("Enter room flags, 0 to quit:", d->character);
  d->edit_mode = REDIT_FLAGS;
}

/* For teleport flags */
void redit_disp_teleport_menu(struct descriptor_data * d)
{
  int             counter;

  send_to_char("[H[J", d->character);
  for (counter = 0; counter < TELEPORT_NUM; counter += 2) {
    sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
	    counter, teleport_bits[counter],
	    counter + 1, counter + 1 < NUM_ROOM_FLAGS ?
	    teleport_bits[counter + 1] : "");
    send_to_char(buf, d->character);
  }
  sprintbit(d->edit_room->tele->mask, teleport_bits, buf1);
  sprintf(buf, "Teleport flags: %s\r\n", buf1);
  send_to_char(buf, d->character);
  send_to_char("Enter teleport flags, 'q' to quit:", d->character);
  d->edit_mode = REDIT_TELEPORT_MENU;
}

/* for sector type */
void redit_disp_sector_menu(struct descriptor_data * d)
{
  int             counter;

  send_to_char("[H[J", d->character);
  for (counter = 0; counter < NUM_ROOM_SECTORS; counter += 2) {
    sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
	    counter, sector_types[counter],
	    counter + 1, counter + 1 < NUM_ROOM_SECTORS ?
	    sector_types[counter + 1] : "");
    send_to_char(buf, d->character);
  }
  send_to_char("Enter sector type:", d->character);
  d->edit_mode = REDIT_SECTOR;
}

/* the main menu */
void redit_disp_menu(struct descriptor_data * d)
{

  send_to_char("[H[J", d->character);
  d->edit_mode = REDIT_MAIN_MENU;
  sprintf(buf, "Room number: %d\r\n", d->edit_number);
  send_to_char(buf, d->character);
  sprintf(buf, "1) Room name: %s\r\n", d->edit_room->name);
  send_to_char(buf, d->character);
  sprintf(buf, "2) Room Desc:\r\n%s\r\n", d->edit_room->description);
  send_to_char(buf, d->character);
  sprintf(buf, "Room zone: %d\r\n",
	  zone_table[d->edit_room->zone].number);
  send_to_char(buf, d->character);
  sprintbit((long) d->edit_room->room_flags, room_bits, buf2);
  sprintf(buf, "3) Room flags: %s\r\n", buf2);
  send_to_char(buf, d->character);
  sprinttype(d->edit_room->sector_type, sector_types, buf2);
  sprintf(buf, "4) Sector type: %s\r\n", buf2);
  send_to_char(buf, d->character);
  if (d->edit_room->dir_option[NORTH])
    sprintf(buf2, "%d", d->edit_room->dir_option[NORTH]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "5) Exit north to: %s\r\n", buf2);
  send_to_char(buf, d->character);
  if (d->edit_room->dir_option[EAST])
    sprintf(buf2, "%d", d->edit_room->dir_option[EAST]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "6) Exit east to:  %s\r\n", buf2);
  send_to_char(buf, d->character);
  if (d->edit_room->dir_option[SOUTH])
    sprintf(buf2, "%d", d->edit_room->dir_option[SOUTH]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "7) Exit south to: %s\r\n", buf2);
  send_to_char(buf, d->character);

  if (d->edit_room->dir_option[WEST])
    sprintf(buf2, "%d", d->edit_room->dir_option[WEST]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "8) Exit west to:  %s\r\n", buf2);
  send_to_char(buf, d->character);
  if (d->edit_room->dir_option[UP])
    sprintf(buf2, "%d", d->edit_room->dir_option[UP]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "9) Exit up to:    %s\r\n", buf2);
  send_to_char(buf, d->character);
  if (d->edit_room->dir_option[DOWN])
    sprintf(buf2, "%d", d->edit_room->dir_option[DOWN]->to_room_vnum);
  else
    strcpy(buf2, "<NONE>");
  sprintf(buf, "a) Exit down to:  %s\r\n", buf2);
  send_to_char(buf, d->character);
  send_to_char("b) Extra descriptions\r\n", d->character);

  if (IS_SET(d->edit_room->room_flags, ROOM_BROADCAST)) {
    send_to_char("  Broadcasting Control \r\n", d->character);
    sprintf(buf, "p) Broadcasting Channel: %s\r\n", channel_bits[(d->edit_room->broad->channel) - 1]);
    send_to_char(buf, d->character);
    sprintf(buf, "r) Broadcasting Target Room1: %s \r\n",
		world[real_room(d->edit_room->broad->targ1)].name);
    send_to_char(buf, d->character);
    sprintf(buf, "s) Broadcasting Target Room2: %s \r\n",
		world[real_room(d->edit_room->broad->targ2)].name);
    send_to_char(buf, d->character);    
  }
  if (d->edit_room->tele != NULL) {
    send_to_char("  Teleportation Control \r\n"
	"   Set target to 0 for non-teleporting rooms\r\n", d->character);
    sprintf(buf, "t) Teleport Target   :  %s\r\n", 
		world[real_room(d->edit_room->tele->targ)].name);
    send_to_char(buf, d->character);
    sprintf(buf, "u) Teleport Frequency:  ~%d*10 seconds\r\n",
		d->edit_room->tele->time);
    send_to_char(buf, d->character);
    sprintbit(d->edit_room->tele->mask, teleport_bits, buf2);
    sprintf(buf, "v) Teleport Flags    : %s\r\n", buf2);
    send_to_char(buf, d->character);
    if (IS_SET(d->edit_room->tele->mask, TELE_OBJ) ||
		IS_SET(d->edit_room->tele->mask, TELE_NOOBJ)) {
	if (d->edit_room->tele->obj == 0)  
	  sprintf(buf, "w) Teleport Obj-VNUM : (NULL) [CHANGE IT]\r\n");
	else
	  sprintf(buf, "w) Teleport Obj-VNUM : %s\r\n",
		obj_proto[real_object(d->edit_room->tele->obj)].short_description);
    } else
	  sprintf(buf, "w) Teleport Obj-VNUM : (NULL) [not used]\r\n");
    send_to_char(buf, d->character);
  }
  send_to_char("q) Quit\r\n", d->character);
  send_to_char("Enter your choice:\r\n", d->character);
}



/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse(struct descriptor_data * d, char *arg)
{
  extern struct room_data *world;
  char buf[33];
  int             number;
  int             room_num;
  switch (d->edit_mode) {
  case REDIT_CONFIRM_EDIT:
    switch (*arg) {
    case 'y':
    case 'Y':
      redit_disp_menu(d);
      break;
    case 'n':
    case 'N':
      /* player doesn't want to edit, free entire temp room */
      STATE(d) = CON_PLAYING;
      if (d->edit_room)
	free_room(d->edit_room);
      d->edit_room = NULL;
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
      break;
    default:
      send_to_char("That's not a valid choice!\r\n", d->character);
      send_to_char("Do you wish to edit it?\r\n", d->character);
      break;
    }
    break; /* end of REDIT_CONFIRM_EDIT */
  case REDIT_CONFIRM_SAVEDB:
    switch (*arg) {
    case 'y':
    case 'Y':
      send_to_char("Writing room to disk.\r\n", d->character);

      {
	/* this code writes the entire zone containing
	   the edited room to disk */
	int             counter, counter2, realcounter;
	char	newname[50], oldname[50];
	FILE           *fp;
	struct room_data *room = NULL;
	struct extra_descr_data *ex_desc;

	sprintf(newname, "%s/%d.wld.back", WLD_PREFIX,
		zone_table[d->edit_zone].number);
	sprintf(oldname, "%s/%d.wld", WLD_PREFIX,
		zone_table[d->edit_zone].number);
	fp = fopen(newname, "w+");
	for (counter = zone_table[d->edit_zone].number * 100;
	     counter <= zone_table[d->edit_zone].top;
	     counter++) {
	  realcounter = real_room(counter);
	  if (realcounter >= 0) {
	    room = &world[realcounter];
	    fprintf(fp, "#%d\n", counter);
	    fprintf(fp, "%s~\n", room->name);
	    fprintf(fp, "%s~\n", room->description);
	    fprintf(fp, "%d %s %d\n",
		    zone_table[room->zone].number,
		    sprintbitascii(room->room_flags, buf),
		    room->sector_type);
	    for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) {
	      if (room->dir_option[counter2]) {
		int             temp_door_flag;
		fprintf(fp, "D%d\n", counter2);
		fprintf(fp, "%s~\n",
			room->dir_option[counter2]->general_description ?
			room->dir_option[counter2]->general_description :
			"");
		fprintf(fp, "%s~\n", room->dir_option[counter2]->keyword ?
			room->dir_option[counter2]->keyword :
			"");
		/* door flags need special handling, unfortunately. argh! */
		if (IS_SET(room->dir_option[counter2]->exit_info, EX_ISDOOR)) {
		  if (IS_SET(room->dir_option[counter2]->exit_info, EX_PICKPROOF))
		    temp_door_flag = 2;
		  else
		    temp_door_flag = 1;
		} else
		  temp_door_flag = 0;
                if (IS_SET(room->dir_option[counter2]->exit_info, EX_HIDDEN))
		  temp_door_flag += 3;
		fprintf(fp, "%d %d %d\n",
			temp_door_flag,
			room->dir_option[counter2]->key,
			room->dir_option[counter2]->to_room_vnum);
	      }
	    }
	    if (room->ex_description) {
	      for (ex_desc = room->ex_description; ex_desc; ex_desc = ex_desc->next) {
		fprintf(fp, "E\n");
		fprintf(fp, "%s~\n", ex_desc->keyword);
		fprintf(fp, "%s~\n", ex_desc->description);
	      }
	    }
	    if (room->tele != NULL) {
		fprintf(fp, "T\n");
		fprintf(fp, "%d %ld %d %d\n",
			room->tele->targ,
			room->tele->mask,
			room->tele->time,
			room->tele->obj);
	    }
            if (IS_SET(room->room_flags, ROOM_BROADCAST) && room->broad != NULL) {
		fprintf(fp, "B\n");
		fprintf(fp, "%d %d %d\n",
			room->broad->channel,
			room->broad->targ1,
			room->broad->targ2);
	    }
	    fprintf(fp, "S\n");
	  }
	}
	/* write final line and close */
	fprintf(fp, "$~\n");
	fclose(fp);
        remove(oldname);
	rename(newname, oldname);
	send_to_char("Saved.\r\n", d->character);
	/* do NOT free strings! just the room structure */
	free(d->edit_room);
	d->edit_room = NULL;
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
	STATE(d) = CON_PLAYING;
	send_to_char("Done.\r\n", d->character);
      }
      break;
    case 'n':
    case 'N':
      send_to_char("Room not saved to disk, available until next reboot.\r\n", d->character);
      /* I can free the room structure, but I cannot free the string / exit
       * pointers! */
      free(d->edit_room);
      d->edit_room = NULL;
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
      STATE(d) = CON_PLAYING;
      send_to_char("Done.\r\n", d->character);
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to write this room to disk?", d->character);
      break;
    }
    break; /* end of REDIT_CONFIRM_SAVEDB */
  case REDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      {
	int             counter;
	room_num = real_room(d->edit_number);
	if (room_num > 0) {
	  /* copy people/object pointers over to the temp room
	     as a temporary measure */
	  d->edit_room->contents = world[room_num].contents;
	  d->edit_room->people = world[room_num].people;
	  /* find the original room, and free all strings/pointers */
	  free_room(&world[room_num]);
	  /* now copy everything over! */
	  world[room_num] = *d->edit_room;
	  if (d->edit_room->tele->targ <= 0) {
		free(world[room_num].tele);
		world[room_num].tele = NULL;
	  }
          if (!IS_SET(d->edit_room->room_flags, ROOM_BROADCAST)) {
		free(world[room_num].broad);
		world[room_num].broad = NULL;
	  }
	  /* resolve all vnum doors to rnum doors in the newly edited room */
	  for (counter = 0; counter < NUM_OF_DIRS; counter++) {
	    if (world[room_num].dir_option[counter]) {
	      world[room_num].dir_option[counter]->to_room =
		real_room(world[room_num].dir_option[counter]->to_room_vnum);
	    }
	  }

	} else {  /* SHOULDN"T GET HERE... BAD, MEANS WE"RE NOT USING STUBS */
	  /* hm, we can't just copy.. gotta insert a new room */
	  int             counter;
	  int             counter2;
	  int             found = 0;
	  /* make a new world table with one more spot */
	  struct room_data *new_world;
	  CREATE(new_world, struct room_data, top_of_world + 2);
	  /* count thru world tables */
	  for (counter = 0; counter < top_of_world + 1; counter++) {
	    /* if we haven't found place to insert  */
	    if (!found) {
	      struct char_data *temp_ch;
	      struct obj_data *temp_obj;
	      /* check if current virtual is bigger than our virtual */
	      if (world[counter].number > d->edit_number) {
		/* eureka! insert now */
		new_world[counter] = *(d->edit_room);
		new_world[counter].number = d->edit_number;
		new_world[counter].func = NULL;
		found = 1;
		/* people in this room must have their numbers moved */
		for (temp_ch = world[counter].people; temp_ch;
		     temp_ch = temp_ch->next_in_room)
		  if (temp_ch->in_room != -1)
		    temp_ch->in_room = counter + 1;
		/* move objects */
		for (temp_obj = world[counter].contents; temp_obj;
		     temp_obj = temp_obj->next_content)
		  if (temp_obj->in_room != -1)
		    temp_obj->in_room = counter + 1;
		/* copy from world to new_world + 1 */
		new_world[counter + 1] = world[counter];
	      } else {
		/* just copy everything over if current virtual
		   is not bigger */
		new_world[counter] = world[counter];
	      }
	    } else {		/* we have found it */
	      /* people in this room must have their in_rooms moved */
	      struct char_data *temp_ch;
	      struct obj_data *temp_obj;
	      for (temp_ch = world[counter].people; temp_ch;
		   temp_ch = temp_ch->next_in_room)
		if (temp_ch->in_room != -1)
		  temp_ch->in_room = counter + 1;
	      /* move objects */
	      for (temp_obj = world[counter].contents; temp_obj;
		   temp_obj = temp_obj->next_content)
		if (temp_obj->in_room != -1)
		  temp_obj->in_room = counter + 1;

	      new_world[counter + 1] = world[counter];
	    }
	  }
	  top_of_world++;
	  /* copy world table over */
	  free(world);
	  world = new_world;
	  /* we have to update the start rooms */
	  r_mortal_start_room = real_room(mortal_start_room);
	  r_immort_start_room = real_room(immort_start_room);
	  r_frozen_start_room = real_room(frozen_start_room);
	  /* now this is the *real* room_num */
	  room_num = real_room(d->edit_number);
	  /* go through the world. if any of the old rooms indicated an exit
	   * to our new room, we have to change it */
	  for (counter = 0; counter < top_of_world + 1; counter++)
	    for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) {
	      /* if exit exists */
	      if (world[counter].dir_option[counter2]) {
		/* increment r_nums for rooms bigger than or equal to new one
		 * because we inserted room */
		if (world[counter].dir_option[counter2]->to_room >= room_num)
		  world[counter].dir_option[counter2]->to_room =
		    world[counter].dir_option[counter2]->to_room + 1;
		/* if an exit to the new room is indicated, change to_room */
		if (world[counter].dir_option[counter2]->to_room_vnum
		    == d->edit_number)
		  world[counter].dir_option[counter2]->to_room =
		    room_num;
	      }
	    }
	  /* resolve all vnum doors to rnum doors in the newly edited room */
	  for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) {
	    if (world[room_num].dir_option[counter2]) {
	      world[room_num].dir_option[counter2]->to_room =
		real_room(world[room_num].dir_option[counter2]->to_room_vnum);
	    }
	  }
	}
	send_to_char("Do you want to write this room to disk?\r\n",
		     d->character);
	d->edit_mode = REDIT_CONFIRM_SAVEDB;
      }
      break;
    case 'n':
    case 'N':
      send_to_char("Room not saved, aborting.\r\n", d->character);
      STATE(d) = CON_PLAYING;
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
      /* free everything up, including strings etc */
      if (d->edit_room)
	free_room(d->edit_room);
      d->edit_room = NULL;
      d->edit_number = 0;
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save this room internally?", d->character);
      break;
    }
    break; /* end of REDIT_CONFIRM_SAVESTRING */
  case REDIT_MAIN_MENU:
    switch (*arg) {
    case 'q':
    case 'Q':
      send_to_char("Do you wish to save this room internally?\r\n", d->character);
      d->edit_mode = REDIT_CONFIRM_SAVESTRING;
      break;
    case '1':
      send_to_char("Enter room name:", d->character);
      d->edit_mode = REDIT_NAME;
      break;
    case '2':
      send_to_char("Enter room description:\r\n", d->character);
      d->edit_mode = REDIT_DESC;
      d->str = (char **) malloc(sizeof(char *));
      *(d->str) = NULL;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      break;
    case '3':
      redit_disp_flag_menu(d);
      break;
    case '4':
      redit_disp_sector_menu(d);
      break;
    case '5':
      d->edit_number2 = NORTH;
      redit_disp_exit_menu(d);
      break;
    case '6':
      d->edit_number2 = EAST;
      redit_disp_exit_menu(d);
      break;
    case '7':
      d->edit_number2 = SOUTH;
      redit_disp_exit_menu(d);
      break;
    case '8':
      d->edit_number2 = WEST;
      redit_disp_exit_menu(d);
      break;
    case '9':
      d->edit_number2 = UP;
      redit_disp_exit_menu(d);
      break;
    case 'a':
    case 'A':
      d->edit_number2 = DOWN;
      redit_disp_exit_menu(d);
      break;
    case 'b':
    case 'B':
      /* if extra desc doesn't exist . */
      if (!d->edit_room->ex_description) {
	CREATE(d->edit_room->ex_description, struct extra_descr_data, 1);
	d->edit_room->ex_description->next = NULL;
      }
      d->misc_data = (void **) &(d->edit_room->ex_description);
      redit_disp_extradesc_menu(d);
      break;
    case 'p':
    case 'P':
      if (IS_SET(d->edit_room->room_flags, ROOM_BROADCAST)) {
	send_to_char("Enter channel number: ", d->character);
        d->edit_mode = REDIT_BROADCAST_CHANNEL;
      } else {
	send_to_char("But this room isn't flagged as a Broadcasting room", d->character);
      }
      break;
    case 'r':
    case 'R':
      if (IS_SET(d->edit_room->room_flags, ROOM_BROADCAST)) {
        send_to_char("Enter target vnum: ", d->character);
        d->edit_mode = REDIT_BROADCAST_TARG1;
      } else {
	send_to_char("But this room isn't flagged as a Broadcasting room", d->character);
      }
      break;
    case 's':
    case 'S':
      if (IS_SET(d->edit_room->room_flags, ROOM_BROADCAST)) {
	send_to_char("Enter target vnum: ", d->character);
        d->edit_mode = REDIT_BROADCAST_TARG2;
      } else {
	send_to_char("But this room isn't flagged as a Broadcasting room", d->character);
      }
      break;
    case 't':
    case 'T':
      send_to_char("Enter target vnum: ", d->character);
      d->edit_mode = REDIT_TELEPORT_TARGET;
      break;
    case 'u':
    case 'U':
      send_to_char("Enter teleport frequency: ", d->character);
      d->edit_mode = REDIT_TELEPORT_FREQ;
      break;
    case 'v':
    case 'V':
      redit_disp_teleport_menu(d);
      break;
    case 'w':
    case 'W':
      send_to_char("Enter Object's Vnum: ", d->character);
      d->edit_mode = REDIT_TELEPORT_OBJ;
      break;
    default:
      send_to_char("Invalid choice!", d->character);
      redit_disp_menu(d);
      break;
    }
    break;
  case REDIT_BROADCAST_TARG1:
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (real_room(number) != -1) {
		d->edit_room->broad->targ1 = number;
		redit_disp_menu(d);
		break;
            }
    }
    send_to_char("That was not a valid room, please try again or enter -1: ", d->character);
    break;
  case REDIT_BROADCAST_TARG2:
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (real_room(number) != -1) {
		d->edit_room->broad->targ2 = number;
		redit_disp_menu(d);
		break;
            }
    }
    send_to_char("That was not a valid room, please try again or enter -1: ", d->character);
    break;
  case REDIT_BROADCAST_CHANNEL:
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (number <= 32 && number >= 0) {
		d->edit_room->broad->channel = number;
		redit_disp_menu(d);
		break;
            }
    }
    send_to_char("That was not a valid number, please enter something between 0 and 32: ", d->character);
    break;	
  case REDIT_TELEPORT_TARGET:
    if (!strcmp("-1", arg)) {
		d->edit_room->tele->targ = -1;
		redit_disp_menu(d);
		break;
    }
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (real_room(number) != -1) {
		d->edit_room->tele->targ = number;
		redit_disp_menu(d);
		break;
            }
    }
    send_to_char("That was not a valid room, please try again or enter -1: ", d->character);
    break;
  case REDIT_TELEPORT_FREQ:
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (number >= 2 && number <= 7200) {
		d->edit_room->tele->time = number;
	        redit_disp_menu(d);
		break;
   	        }
            }
    send_to_char("That was not a valid number, please enter something between 2 and 7200: ", d->character);
    break;	
  case REDIT_TELEPORT_OBJ:
    if (isdigit(*arg)) {
	    number = atoi(arg);
	    if (real_object(number) != -1) {
		d->edit_room->tele->obj = number;
	        redit_disp_menu(d);
		break;
   	        }
            }
    send_to_char("That was not a valid number, please enter the object's VNUM: ", d->character);
    break;	
  case REDIT_TELEPORT_MENU:
    if (isdigit(*arg)) {
	number = atoi(arg);
	if (number >= 0 && number < TELEPORT_NUM) {
		if (IS_SET(d->edit_room->tele->mask, 1 << number))
			REMOVE_BIT(d->edit_room->tele->mask, 1 << number);
		else
			SET_BIT(d->edit_room->tele->mask, 1 << number);
		redit_disp_teleport_menu(d);
		break;
	}
    } 
    switch (*arg) {
    case 'q':
    case 'Q':
	redit_disp_menu(d);
        break;
    default:
        send_to_char("Invalid Choice... please try again: ", d->character);
	break;
    }
    break;
  case REDIT_NAME:
    if (d->edit_room->name)
      free(d->edit_room->name);
    d->edit_room->name = str_dup(arg);
    redit_disp_menu(d);
    break;
  case REDIT_DESC:
    /* we will NEVER get here */
    break;
  case REDIT_FLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ROOM_FLAGS)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      redit_disp_flag_menu(d);
    } else {
      if (number == 0)
	/* back out */
	redit_disp_menu(d);
      else {
	/* toggle bits */
	if (IS_SET(d->edit_room->room_flags, 1 << (number - 1)))
	  REMOVE_BIT(d->edit_room->room_flags, 1 << (number - 1));
	else
	  SET_BIT(d->edit_room->room_flags, 1 << (number - 1));
	redit_disp_flag_menu(d);
      }
    }
    break;
  case REDIT_SECTOR:
    number = atoi(arg);
    if (number < 0 || number >= NUM_ROOM_SECTORS) {
      send_to_char("Invalid choice!", d->character);
      redit_disp_sector_menu(d);
    } else {
      d->edit_room->sector_type = number;
      redit_disp_menu(d);
    }
    break;
  case REDIT_EXIT_MENU:
    switch (*arg) {
    case '0':
      redit_disp_menu(d);
      break;
    case '1':
      d->edit_mode = REDIT_EXIT_NUMBER;
      send_to_char("Exit to room number:", d->character);
      break;
    case '2':
      d->edit_mode = REDIT_EXIT_DESCRIPTION;
      d->str = (char **) malloc(sizeof(char *));
      *(d->str) = NULL;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      send_to_char("Enter exit description:\r\n", d->character);
      break;
    case '3':
      d->edit_mode = REDIT_EXIT_KEYWORD;
      send_to_char("Enter keywords:", d->character);
      break;
    case '4':
      d->edit_mode = REDIT_EXIT_KEY;
      send_to_char("Enter key number:", d->character);
      break;
    case '5':
      redit_disp_exit_flag_menu(d);
      d->edit_mode = REDIT_EXIT_DOORFLAGS;
      break;
    case '6':
      /* delete exit */
      if (d->edit_room->dir_option[d->edit_number2]->keyword)
	free(d->edit_room->dir_option[d->edit_number2]->keyword);
      if (d->edit_room->dir_option[d->edit_number2]->general_description)
	free(d->edit_room->dir_option[d->edit_number2]->general_description);
      free(d->edit_room->dir_option[d->edit_number2]);
      d->edit_room->dir_option[d->edit_number2] = NULL;
      redit_disp_menu(d);
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      break;
    }
    break;
  case REDIT_EXIT_NUMBER:
    number = atoi(arg);
    if (number < 0)
      send_to_char("Invalid choice!\r\nExit to room number:", d->character);
    else {
      d->edit_room->dir_option[d->edit_number2]->to_room_vnum = number;
      redit_disp_exit_menu(d);
    }
    break;
  case REDIT_EXIT_DESCRIPTION:
    /* we should NEVER get here */
    break;
  case REDIT_EXIT_KEYWORD:
    if (d->edit_room->dir_option[d->edit_number2]->keyword)
      free(d->edit_room->dir_option[d->edit_number2]->keyword);
    d->edit_room->dir_option[d->edit_number2]->keyword = str_dup(arg);
    redit_disp_exit_menu(d);
    break;
  case REDIT_EXIT_KEY:
    number = atoi(arg);
    d->edit_room->dir_option[d->edit_number2]->key = number;
    redit_disp_exit_menu(d);
    break;
  case REDIT_EXIT_DOORFLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > 5)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      redit_disp_exit_flag_menu(d);
    } else {
      /* doors are a bit idiotic, don't you think? :) */
      d->edit_room->dir_option[d->edit_number2]->exit_info = 0;
      if (number == 1)
	d->edit_room->dir_option[d->edit_number2]->exit_info = EX_ISDOOR;
      else if (number == 2)
	d->edit_room->dir_option[d->edit_number2]->exit_info =
	  EX_ISDOOR | EX_PICKPROOF;
      else if (number == 3)
	d->edit_room->dir_option[d->edit_number2]->exit_info = EX_HIDDEN;
      else if (number == 4)
	d->edit_room->dir_option[d->edit_number2]->exit_info = EX_ISDOOR | EX_HIDDEN;
      else if (number == 5)
	d->edit_room->dir_option[d->edit_number2]->exit_info =
	  EX_ISDOOR | EX_PICKPROOF | EX_HIDDEN;
      /* jump out to menu */
      redit_disp_exit_menu(d);
    }
    break;
  case REDIT_EXTRADESC_KEY:
    ((struct extra_descr_data *) * d->misc_data)->keyword = str_dup(arg);
    redit_disp_extradesc_menu(d);
    break;
  case REDIT_EXTRADESC_MENU:
    number = atoi(arg);
    switch (number) {
    case 0:
      {
	/* if something got left out, delete the extra desc
	 when backing out to menu */
	if (!((struct extra_descr_data *) * d->misc_data)->keyword ||
	    !((struct extra_descr_data *) * d->misc_data)->description) {
	  if (((struct extra_descr_data *) * d->misc_data)->keyword)
	    free(((struct extra_descr_data *) * d->misc_data)->keyword);
	  if (((struct extra_descr_data *) * d->misc_data)->description)
	    free(((struct extra_descr_data *) * d->misc_data)->description);

	  free(*d->misc_data);
	  *d->misc_data = NULL;
	}
	/* else, we don't need to do anything.. jump to menu */
	redit_disp_menu(d);
      }
      break;
    case 1:
      d->edit_mode = REDIT_EXTRADESC_KEY;
      send_to_char("Enter keywords, separated by spaces:", d->character);
      break;
    case 2:
      d->edit_mode = REDIT_EXTRADESC_DESCRIPTION;
      send_to_char("Enter extra description:\r\n", d->character);
      /* send out to modify.c */
      d->str = (char **) malloc(sizeof(char *));
      *(d->str) = NULL;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      break;
    case 3:
      if (!((struct extra_descr_data *) d->misc_data)->keyword ||
	  !((struct extra_descr_data *) d->misc_data)->description) {
	send_to_char("You can't edit the next extra desc without completing this one.\r\n", d->character);
	redit_disp_extradesc_menu(d);
      } else {
	struct extra_descr_data *new_extra;

	if (((struct extra_descr_data *) * d->misc_data)->next)
	  d->misc_data = (void **) &((struct extra_descr_data *) * d->misc_data)->next;
	else {
	  /* make new extra, attach at end */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  ((struct extra_descr_data *) * d->misc_data)->next = new_extra;
	  d->misc_data =
	    (void **) &((struct extra_descr_data *) * d->misc_data)->next;
	}
	redit_disp_extradesc_menu(d);
      }
    }
    break;
  default:
    /* we should never get here */
    break;
  }
}

