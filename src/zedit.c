#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "class.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "screen.h"
#include "zedit.h"

extern int    top_of_zone_table;
extern struct room_data *world;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct zone_data *zone_table;
extern const char *rmode[];
extern const char *equipment_types[];
extern const char *dirs[];

/* external function prototypes */
void log_zone_error(int zone, int cmd_no, char *message);
int remove (char *filename);
int rename (char *filename, char *filename2);

/* local function prototypes */
int zedit_save_zone_file(int zone, struct zone_data *zone_table);
int zedit_parse_zone_table(int zone, struct zone_data *zone_table);
int zedit_initialize(int rznum, struct zone_data *zone_table, struct zedit_struct *zedit_zone);
int zedit_update_zone(int rznum, struct zone_data *zone_table, struct zedit_struct *zedit_zone);
void zedit_parse(struct descriptor_data * d, char *arg);

void zedit_disp_menu(struct descriptor_data *d);
int zedit_disp_mob_menu(struct descriptor_data *d);
int zedit_disp_obj_menu(struct descriptor_data *d);
int zedit_disp_room_menu(struct descriptor_data *d);

int zedit_disp_mob_edit_menu(struct descriptor_data *d);
void zedit_disp_inventory_edit_menu(struct descriptor_data *d);
void zedit_disp_equipment_edit_menu(struct descriptor_data *d);
void zedit_disp_object_edit_menu(struct descriptor_data *d);
void zedit_disp_room_edit_menu(struct descriptor_data *d);

int zedit_update_objs(struct zone_data *resets, struct zedit_obj_list *objs, int cmd_count, char command);
int zedit_update_mobs(struct zone_data *resets, struct zedit_mob_list *mobs, int cmd_count);
int zedit_update_rooms(struct zone_data *resets, struct zedit_room_list *rooms, int cmd_count);
int zedit_update_removes(struct zone_data *resets, struct zedit_remove_list *removes, int cmd_count);

void free_zedit_struct (struct zedit_struct *zone);
int free_zedit_rooms (struct zedit_room_list *rooms, int retval);
int free_zedit_removes (struct zedit_remove_list *removes, int retval);
int free_zedit_objs (struct zedit_obj_list *objs, int retval);
int free_zedit_mobs (struct zedit_mob_list *mobs, int retval);

#define ZCMD zone_table[rzone].cmd[cmd_no]

const char *door_states[] = 
{
	"Open",
	"Closed",
	"Closed and Locked",
	"\n"
};


int zedit_initialize(int rzone, struct zone_data *zone_table, struct zedit_struct * zedit_zone)
{
	int cmd_no;
	struct zedit_mob_list *temp_mob;
	struct zedit_obj_list *temp_obj;
	struct zedit_room_list *temp_room;
	struct zedit_remove_list *temp_remove;

	zedit_zone->cmds = 0;
	zedit_zone->vnum = zone_table[rzone].number;
	zedit_zone->name = strdup(zone_table[rzone].name);
	zedit_zone->top = zone_table[rzone].top;
	zedit_zone->lifespan = zone_table[rzone].lifespan;
	zedit_zone->reset_mode = zone_table[rzone].reset_mode;
	zedit_zone->creator = strdup(zone_table[rzone].creator);
	zedit_zone->lvl_low = zone_table[rzone].lvl_low;
	zedit_zone->lvl_high = zone_table[rzone].lvl_high;

	zedit_zone->rooms = zedit_zone->current_room = NULL;
	zedit_zone->mobs = zedit_zone->current_mob = NULL;
	zedit_zone->objs = zedit_zone->current_obj = NULL;

	for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
		switch (ZCMD.command) {
		case 'M':
			/* This is a mob */
			CREATE(temp_mob, struct zedit_mob_list, 1);
			temp_mob->if_flag = ZCMD.if_flag;
			temp_mob->mob_vnum = mob_index[ZCMD.arg1].virtual;
			temp_mob->max_exist = ZCMD.arg2;
			temp_mob->room_vnum = world[ZCMD.arg3].number;
			temp_mob->inventory = NULL;
			/* Add to list */
			if (zedit_zone->mobs) {
				temp_mob->next = zedit_zone->mobs;
				temp_mob->next->previous = temp_mob;
			} else {
				temp_mob->next = NULL;
			}
			temp_mob->previous = temp_mob;
			zedit_zone->mobs = temp_mob;
			zedit_zone->current_mob = temp_mob;
			temp_mob = NULL;
			break;
		case 'O':
			/* This is an object */
			CREATE(temp_obj, struct zedit_obj_list, 1);
			temp_obj->if_flag = ZCMD.if_flag;
			temp_obj->obj_vnum = obj_index[ZCMD.arg1].virtual;
			temp_obj->max_exist = ZCMD.arg2;
			temp_obj->room_vnum = world[ZCMD.arg3].number;
			/* Add to list */
			if (zedit_zone->objs) {
				temp_obj->next = zedit_zone->objs;
				temp_obj->next->previous = temp_obj;
			} else {
				temp_obj->next = NULL;
			}
			temp_obj->previous = temp_obj;	/* head of list previous's itself */
			zedit_zone->objs = temp_obj;
			temp_obj = NULL;
			break;			
		case 'G':
			CREATE(temp_obj, struct zedit_obj_list, 1);
			temp_obj->if_flag = ZCMD.if_flag;
			temp_obj->obj_vnum = obj_index[ZCMD.arg1].virtual;
			temp_obj->max_exist = ZCMD.arg2;
			temp_obj->room_vnum = NULL;
			/* Add to list */
			if (!zedit_zone->current_mob) {
				/* ERROR */
				log("Error in zedit.c: zedit_initialize got 'G' without a current_mob");
			} else if (zedit_zone->current_mob->inventory) {
				temp_obj->next = zedit_zone->current_mob->inventory;
				temp_obj->next->previous = temp_obj;
			} else {
				temp_obj->next = NULL;
			}
			temp_obj->previous = temp_obj;	/* head of list previous's itself */
			zedit_zone->current_mob->inventory = temp_obj;
			temp_obj = NULL;
			break;
		case 'E':
			CREATE(temp_obj, struct zedit_obj_list, 1);
			temp_obj->if_flag = ZCMD.if_flag;
			temp_obj->obj_vnum = obj_index[ZCMD.arg1].virtual;
			temp_obj->max_exist = ZCMD.arg2;
			temp_obj->room_vnum = ZCMD.arg3;	/* position */
			/* Add to list */
			if (!zedit_zone->current_mob) {
				/* ERROR */
				log("Error in zedit.c: zedit_initialize got 'E' without a current_mob");
			} else if (zedit_zone->current_mob->equipment) {
				temp_obj->next = zedit_zone->current_mob->equipment;
				temp_obj->next->previous = temp_obj;
			} else {
				temp_obj->next = NULL;
			}
			temp_obj->previous = temp_obj;	/* head of list previous's itself */
			zedit_zone->current_mob->equipment = temp_obj;
			temp_obj = NULL;
			break;
		case 'P':
			/* This is an contains */
			CREATE(temp_obj, struct zedit_obj_list, 1);
			temp_obj->if_flag = ZCMD.if_flag;
			temp_obj->obj_vnum = obj_index[ZCMD.arg1].virtual;
			temp_obj->max_exist = ZCMD.arg2;
			temp_obj->room_vnum = world[ZCMD.arg3].number;
			/* Add to list */
			if (zedit_zone->contains) {
				temp_obj->next = zedit_zone->contains;
				temp_obj->next->previous = temp_obj;
			} else {
				temp_obj->next = NULL;
			}
			temp_obj->previous = temp_obj;	/* head of list previous's itself */
			zedit_zone->contains = temp_obj;
			temp_obj = NULL;
			break;
		case 'D':
			/* This is a door */
			CREATE(temp_room, struct zedit_room_list, 1);
			temp_room->if_flag = ZCMD.if_flag;
			temp_room->room_vnum = world[ZCMD.arg1].number;
			temp_room->door_direction = ZCMD.arg2;
			temp_room->door_state = ZCMD.arg3;
			/* Add to list */
			if (zedit_zone->rooms) {
				temp_room->next = zedit_zone->rooms;
				temp_room->next->previous = temp_room;
			} else {
				temp_room->next = NULL;
			}
			temp_room->previous = temp_room;
			zedit_zone->rooms = temp_room;
			temp_room = NULL;			
			break;
		case 'R':
			/* This is a remove */
			CREATE(temp_remove, struct zedit_remove_list, 1);
			temp_remove->if_flag = ZCMD.if_flag;
			temp_remove->room_vnum = world[ZCMD.arg1].number;
			temp_remove->obj_vnum = obj_index[ZCMD.arg2].virtual;
			if (zedit_zone->removes) {
				temp_remove->next = zedit_zone->removes;
				temp_remove->next->previous = temp_remove;
			} else {
				temp_remove->next = NULL;
			}
			temp_remove->previous = temp_remove;
			zedit_zone->removes = temp_remove;
			temp_remove = NULL;
			break;
		default:
			log("zedit.c, in initialize: got to default case");
			break;
		}
	}
	zedit_zone->cmds = cmd_no;
	return 1;
}

void free_zedit_struct (struct zedit_struct *zone)
{
     int temp = zone->cmds;

     if (zone->name)
	free(zone->name);
     if (zone->rooms)
	temp = free_zedit_rooms(zone->rooms, temp);
     if (zone->mobs)
        temp = free_zedit_mobs(zone->mobs, temp);
     if (zone->objs)
        temp = free_zedit_objs(zone->objs, temp);
     if (zone->contains)
        temp = free_zedit_objs(zone->contains, temp);
     if (zone->removes)
	temp = free_zedit_removes(zone->removes, temp);
     if (temp < 0)
	log("zedit.c, in free_zedit_struct: got a temp that's less than 0?!?");
     if (temp > 1)
	log("zedit.c, in free_zedit_struct: got a temp that's more than 1?!?");
     free(zone);
}

int free_zedit_rooms (struct zedit_room_list *rooms, int retval)
{
     if (rooms->next)
	retval = free_zedit_rooms(rooms->next, retval);
     free(rooms);
     return (retval - 1);
}

int free_zedit_removes (struct zedit_remove_list *removes, int retval)
{
     if (removes->next)
	retval = free_zedit_removes(removes->next, retval);
     free(removes);
     return (retval - 1);
}

int free_zedit_objs (struct zedit_obj_list *objs, int retval)
{
     if (objs->next)
	retval = free_zedit_objs(objs->next, retval);
     free(objs);     
     return (retval - 1);
}

int free_zedit_mobs (struct zedit_mob_list *mobs, int retval)
{
     if (mobs->inventory)
	retval = free_zedit_objs(mobs->inventory, retval);
     if (mobs->equipment)
	retval = free_zedit_objs(mobs->equipment, retval);
     if (mobs->next)
        retval = free_zedit_mobs(mobs->next, retval);
     free(mobs);
     return (retval - 1);
}

void free_zone_table(int rznum, struct zone_data *zone_table)
{
	int i;

	free(zone_table[rznum].name);
	for (i = 0; zone_table[rznum].cmd[i].command != 'S'; i++) {
		free(&(zone_table[rznum].cmd[i]));
	}
	free(&(zone_table[rznum].cmd[i]));
	return;
}

void zedit_disp_menu(struct descriptor_data *d)
{
	char outbuf[256];
	struct char_data *ch = d->character;
	
	send_to_char("ZEDIT Main menu          ZEDIT v1.00\r\n", ch);
	sprintf(outbuf, "You are editing zone %d\r\n\r\n", d->edit_number);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    a) Desc.   : %s\r\n", d->zedit_zone->name);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    b) Top     : %d\r\n", d->zedit_zone->top);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    c) Lifespan: %d\r\n", d->zedit_zone->lifespan);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    d) reset   : %s\r\n", rmode[d->zedit_zone->reset_mode]);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    e) Creator : %s\r\n", d->zedit_zone->creator);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    f) MinLevel: %d\r\n", d->zedit_zone->lvl_low);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    g) MaxLevel: %d\r\n\r\n", d->zedit_zone->lvl_high);
	send_to_char(outbuf, ch);
	send_to_char("    1) Edit Mobs to be loaded or their inventory\r\n", ch);
	send_to_char("    2) Edit Objs to be loaded\r\n", ch);
	send_to_char("    3) Edit rooms and their doors\r\n", ch);
	send_to_char("    4) Edit containers\r\n", ch);
	send_to_char("    5) Edit things to be removed\r\n",ch);
	send_to_char("    q) Quit\r\n", ch);
	send_to_char("\r\nPlease Enter your choice:  ", ch);
}

int zedit_disp_mob_menu(struct descriptor_data *d)
{
	char outbuf[256];
	int counter = 0;
	int retval;
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct char_data *mob = NULL;
	struct char_data *ch = d->character;
	struct zedit_mob_list *current_mob;

	send_to_char("Choose from the following commands:\r\n", ch);
	send_to_char("    a) add a mob\r\n", ch);
	send_to_char("    #) edit a mob\r\n", ch);
	send_to_char("    n) goto next mob\r\n", ch);
	send_to_char("    p) goto previous mob\r\n", ch);
        send_to_char("    y) pop to the top of the list\r\n", ch);
	send_to_char("    q) go back to main menu\r\n\r\n", ch);

	if (!(zedit_zone->current_mob))
		current_mob = zedit_zone->mobs;
	else
		current_mob = zedit_zone->current_mob;

	counter = 0;
	while (current_mob && counter < 9) {
		counter++;
		mob = &(mob_proto[real_mobile(current_mob->mob_vnum)]);
		sprintf(outbuf, "%5d) %.20s in %.20s\r\n", counter, 
			GET_NAME(mob), world[real_room(current_mob->room_vnum)].name);
		send_to_char(outbuf, ch);
		current_mob = current_mob->next;
	}
	retval = counter;
	while (counter < 9) {
		send_to_char("\r\n", ch);
		counter++;		
	}
	send_to_char("\r\n", ch);
	send_to_char("What do you wish to do? ", ch);
	return retval; 	/* return biggest number possible */
}

int zedit_disp_mob_edit_menu(struct descriptor_data *d)
{
	char outbuf[256];
	int counter = 0;
	int retval;
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct obj_data *obj = NULL;
	struct char_data *ch = d->character;
	struct zedit_mob_list *current_mob = zedit_zone->current_mob;
	struct zedit_obj_list *current_obj = zedit_zone->current_obj;
	struct char_data *mob = NULL;

	if (!zedit_zone->current_mob) {
		log("zedit.c, disp_mob_edit_menu: no current_mob?!?");
		return 0;
	} else {
		mob = &mob_proto[real_mobile(current_mob->mob_vnum)];
	}
	sprintf(outbuf, "%20.20s in %-20.20s     Strom's ZEDIT", GET_NAME(mob),
		world[real_room(current_mob->room_vnum)].name);
	send_to_char("Choose from the following commands:\r\n", ch);
	send_to_char("    a) add item\r\n", ch);
	send_to_char("    #) edit an item\r\n", ch);
	sprintf(outbuf, "    c) change mob       : %-20s\r\n",
		GET_NAME(mob));
	send_to_char(outbuf, ch);
	send_to_char(   "    d) delete this mob\r\n", ch);
	sprintf(outbuf, "    r) change location  : %-30s\r\n",
		world[real_room(current_mob->room_vnum)].name);
	send_to_char(outbuf, ch);
	sprintf(outbuf, "    m) change maximum   : %d\r\n",
		current_mob->max_exist);
	send_to_char(outbuf, ch);
	send_to_char("    n) goto next object\r\n", ch);
	send_to_char("    p) goto previous object\r\n", ch);
	if (zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT)
		send_to_char("    t) toggle to equipment display\r\n",ch);
	else
		send_to_char("    t) toggle to inventory display\r\n",ch);
        send_to_char("    y) pop to the top of the list\r\n", ch);
	send_to_char("    q) goto back to mob menu\r\n\r\n", ch);
	
	if (zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT)
		send_to_char("Inventory:\r\n",ch);
	else
		send_to_char("Equipment:\r\n",ch);
	counter = 0;
	while (current_obj && counter < 9) {
		counter++;
		obj = &(obj_proto[real_object(current_obj->obj_vnum)]);
		sprintf(outbuf, "%5d) %.20s\r\n", counter, 
			obj->short_description);
		send_to_char(outbuf, ch);
		current_obj = current_obj->next;
	}
	retval = counter;
	while (counter < 9) {
		send_to_char("\r\n", ch);
		counter++;		
	}
	send_to_char("\r\n", ch);
	send_to_char("What do you wish to do? ", ch);
	return retval; 	/* return biggest number possible */
}

void zedit_disp_inventory_edit_menu(struct descriptor_data *d) {
	char outbuf[256];
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct obj_data *obj = NULL;
	struct char_data *ch = d->character;
	struct zedit_obj_list *current_obj = zedit_zone->current_obj;

	if (!current_obj) {
		log("zedit.c, disp_inventory_edit_menu: no current_obj?!?");
	} else {
		obj = &obj_proto[real_object(current_obj->obj_vnum)];
		send_to_char("Choose from the following commands:\r\n", ch);
		sprintf(outbuf, "    c) change object    : %-20s\r\n",
			obj->short_description);
		send_to_char(outbuf, ch);
		send_to_char(   "    d) delete object    :\r\n", ch);
		sprintf(outbuf, "    m) change maximum   : %d\r\n",
			current_obj->max_exist);
		send_to_char(outbuf, ch);
	}
	send_to_char(   "    q) return to mob edit\r\n", ch);
	send_to_char("\r\nWhat do you wish to do? ", ch);
	return;
}

void zedit_disp_equipment_edit_menu(struct descriptor_data *d) {
	char outbuf[256];
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct obj_data *obj = NULL;
	struct char_data *ch = d->character;
	struct zedit_obj_list *current_obj = zedit_zone->current_obj;

	if (!current_obj) {
		log("zedit.c, disp_inventory_edit_menu: no current_obj?!?");
	} else {
		obj = &obj_proto[real_object(current_obj->obj_vnum)];
		send_to_char("Choose from the following commands:\r\n", ch);
		sprintf(outbuf, "    c) change obj       : %-20s\r\n",
			obj->short_description);
		send_to_char(outbuf, ch);
		send_to_char(   "    d) delete object    :\r\n", ch);
		sprintf(outbuf, "    m) change maximum   : %d\r\n",
			current_obj->max_exist);
		send_to_char(outbuf, ch);
		sprintf(outbuf, "    l) change location  : %s\r\n",
			equipment_types[current_obj->room_vnum]);
		send_to_char(outbuf, ch);
	}
	send_to_char("    q) return to mob edit\r\n", ch);
	send_to_char("\r\nWhat do you wish to do? ", ch);
	return;
}

void zedit_disp_object_edit_menu(struct descriptor_data *d) {
	char outbuf[256];
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct obj_data *obj = NULL;
	struct char_data *ch = d->character;
	struct zedit_obj_list *current_obj = zedit_zone->current_obj;

	if (!current_obj) {
		log("zedit.c, disp_object_edit_menu: no current_obj?!?");
	} else {
		obj = &obj_proto[real_object(current_obj->obj_vnum)];
		send_to_char("Choose from the following commands:\r\n", ch);
		sprintf(outbuf, "    c) change obj       : %-20s\r\n",
			obj->short_description);
		send_to_char(outbuf, ch);
		send_to_char(   "    d) delete object    :\r\n", ch);
		sprintf(outbuf, "    m) change maximum   : %d\r\n",
			current_obj->max_exist);
		send_to_char(outbuf, ch);
		sprintf(outbuf, "    l) change location  : %s\r\n",
			world[real_room(current_obj->room_vnum)].name);
		send_to_char(outbuf, ch);
	}
	send_to_char("    q) return to object menu\r\n", ch);
	send_to_char("\r\nWhat do you wish to do? ", ch);
	return;
}

int zedit_disp_obj_menu(struct descriptor_data *d) {
	char outbuf[256];
	int counter = 0;
	int retval;
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct obj_data *obj = NULL;
	struct char_data *ch = d->character;
	struct zedit_obj_list *current_obj = zedit_zone->current_obj;

	send_to_char("Choose from the following commands:\r\n", ch);
	send_to_char("    a) add item\r\n", ch);
	send_to_char("    #) edit an item\r\n", ch);
	send_to_char("    n) goto next object\r\n", ch);
	send_to_char("    p) goto previous object\r\n", ch);
        send_to_char("    y) pop to the top of the list\r\n", ch);
	send_to_char("    q) goto back to main menu\r\n\r\n", ch);
	send_to_char("\r\nObjects to be loaded:\r\n", ch);
	counter = 0;
	while (current_obj && counter < 9) {
		counter++;
		obj = &(obj_proto[real_object(current_obj->obj_vnum)]);
		sprintf(outbuf, "%5d) %.20s to %.20s\r\n", counter, 
			obj->short_description, 
			world[real_room(current_obj->room_vnum)].name);
		send_to_char(outbuf, ch);
		current_obj = current_obj->next;
	}
	retval = counter;
	while (counter < 9) {
		send_to_char("\r\n", ch);
		counter++;		
	}
	send_to_char("\r\n", ch);
	send_to_char("What do you wish to do? ", ch);
	return retval; 	/* return biggest number possible */
}

int zedit_disp_room_menu(struct descriptor_data *d) {
	char outbuf[256];
	int counter = 0;
	int retval;
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct char_data *ch = d->character;
	struct zedit_room_list *current_room;

	send_to_char("Choose from the following commands:\r\n", ch);
	send_to_char("    a) add a door\r\n", ch);
	send_to_char("    #) edit a door\r\n", ch);
        send_to_char("    n) goto next door\r\n", ch);
	send_to_char("    p) goto previous door\r\n", ch);
        send_to_char("    y) pop to the top of the list\r\n", ch);
	send_to_char("    q) go back to main menu\r\n\r\n", ch);

	if (!(zedit_zone->current_room))
		current_room = zedit_zone->rooms;
	else
		current_room = zedit_zone->current_room;

	counter = 0;
	while (current_room && counter < 9) {
		counter++;
		sprintf(outbuf,"%5d) %s door to the %s in %.20s (%d)\r\n", counter,
			door_states[current_room->door_state],
			dirs[current_room->door_direction],
			world[real_room(current_room->room_vnum)].name,
			current_room->room_vnum);
		current_room = current_room->next;
		send_to_char(outbuf, ch);
	}
	retval = counter;
	while (counter < 9) {
		send_to_char("\r\n", ch);
		counter++;
	}
	send_to_char("\r\n", ch);
	send_to_char("What do you wish to do? ", ch);
	return retval;
}

void zedit_disp_room_edit_menu(struct descriptor_data *d) {
	char outbuf[256];
	struct zedit_struct *zedit_zone = d->zedit_zone;
	struct char_data *ch = d->character;
	struct zedit_room_list *current_room = zedit_zone->current_room;

	if (!current_room) {
		log("zedit.c, disp_room_edit_menu: no current_room?!?");
        } else {
		send_to_char("Choose from the following commands:\r\n", ch);
		sprintf(outbuf, "    r) change room      : %-25s\r\n",
			world[real_room(current_room->room_vnum)].name);
		send_to_char(outbuf, ch);
		sprintf(outbuf, "    c) change direction : %-10s\r\n",
			dirs[current_room->door_direction]);
		send_to_char(outbuf, ch);
		sprintf(outbuf, "    s) change state     : %-25s\r\n",
			door_states[current_room->door_state]);
		send_to_char(outbuf, ch);
		send_to_char(   "    d) delete door\r\n", ch);
	}
        send_to_char(   "    q) return to door menu\r\n", ch);
	send_to_char("\r\nWhat do you wish to do? ", ch);
	return;
}

int zedit_save_zone_file(int rzone, struct zone_data *zone_table)
{

	char buf[256], zcom, commentbuf[256];
	char ofname[256], nfname[256];
	int cmd_no, a, b, c;
	FILE	*fp, *old;
	struct char_data *mob = NULL;
	
	sprintf(nfname, "%s/%d.zon.temp", ZON_PREFIX, zone_table[rzone].number);
	fp = fopen(nfname, "w");
	sprintf(ofname, "%s/%d.zon", ZON_PREFIX, zone_table[rzone].number);
	old = fopen(ofname, "r");

	/* the preamble */
	fprintf(fp, "#%d %s %d %d\n", zone_table[rzone].number, 
		zone_table[rzone].creator, zone_table[rzone].lvl_low,
		zone_table[rzone].lvl_high);
	fprintf(fp, "%s~\n", zone_table[rzone].name);
	fprintf(fp, "%d %d %d\n", zone_table[rzone].top,
			zone_table[rzone].lifespan,
			zone_table[rzone].reset_mode);
	fprintf(fp, "*** Zone %d was created using Strom's zedit ***\n",
			zone_table[rzone].number);

	/* get builder info */

	do {
	   get_line2(old, buf);	
	   if (*buf == '*') {
		if (strstr(buf, "Builder")) {
			fprintf(fp, "%s\n", buf);
		}
           }
        } while (*buf != 'S');

	/* the resets */
	for (cmd_no = 0; (zcom = ZCMD.command) != 'S'; cmd_no++) {
		a = ZCMD.arg1;
		b = ZCMD.arg2;
		c = ZCMD.arg3;
		switch (zcom) {
		case 'M':
			mob = &(mob_proto[a]);
			sprintf(commentbuf, "Load %.20s to %.20s (R %d)",
				GET_NAME(mob), world[c].name,
	 			c = world[c].number);
			a = mob_index[a].virtual;
			break;
		case 'O':
			sprintf(commentbuf, "Load %.20s to %.20s (R %d)",
				obj_proto[a].short_description, world[c].name, 
				c = world[c].number);
			a = obj_index[a].virtual;
			break;
		case 'G':
			sprintf(commentbuf, "Give %.20s to %.20s",
				obj_proto[a].short_description, GET_NAME(mob));
			a = obj_index[a].virtual;
			break;
		case 'E':
			sprintf(commentbuf, "Equip %.20s with %.20s",
				GET_NAME(mob), obj_proto[a].short_description);
			a = obj_index[a].virtual;
			break;
		case 'P':
			sprintf(commentbuf, "Put %.20s in %.20s",
				obj_proto[a].name, obj_proto[c].short_description);
			a = obj_index[a].virtual;
			c = obj_index[c].virtual;
			break;
		case 'D':
			sprintf(commentbuf, "Door in %.20s (R %d)", 
				world[a].name, a = world[a].number);
			break;
		case 'R':
			sprintf(commentbuf, "Remove %.20s from %.20s (Room %d)",
				obj_proto[c].name, world[a].name,
				a = world[a].number);
			c = obj_index[c].virtual;
			break;
		default:
			break;
		}

		/* fprint the line of reset into the file */

		if (a < 0 || b < 0 || c < 0) {
			log_zone_error(rzone, cmd_no, "Invalid command generated in zedit");
			return 0;
		} else if (zcom == 0) {
			log_zone_error(rzone, cmd_no, "Invalid command generated in zedit... zcom of 0");
		} else if (strchr("MOEPD", ZCMD.command) == NULL) { /* 3-arg command */
			sprintf(buf, "%c %d %d %d", zcom, ZCMD.if_flag, a, b);
			fprintf(fp, "%-18s %s\n", buf, commentbuf);
		} else { /* 4-arg command */
			sprintf(buf, "%c %d %d %d %d", zcom, ZCMD.if_flag, a, b, c);
			fprintf(fp, "%-18s %s\n", buf, commentbuf);
		}
	}
	/* fprint the 'S' and close up shop*/
	fprintf(fp, "S   *Zone %d has been completed*\n$~", zone_table[rzone].number);
        fclose(old);
	fclose(fp);
        remove(ofname);
        rename(nfname, ofname);
	return TRUE;
}

int zedit_update_mobs(struct zone_data *resets, struct zedit_mob_list *mobs, int cmd_count)
{
	int temp_count = cmd_count;

	resets->cmd[temp_count].command = 'M';
	resets->cmd[temp_count].if_flag = mobs->if_flag;
	resets->cmd[temp_count].arg1 = real_mobile(mobs->mob_vnum);
	resets->cmd[temp_count].arg2= mobs->max_exist;
	resets->cmd[temp_count].arg3 = real_room(mobs->room_vnum);
	resets->cmd[temp_count].line = temp_count;

	temp_count++;
	if (mobs->inventory)
		temp_count = zedit_update_objs(resets, mobs->inventory, temp_count, 'G');
	if (mobs->equipment)
		temp_count = zedit_update_objs(resets, mobs->equipment, temp_count, 'E');
	if (mobs->next)
		temp_count = zedit_update_mobs(resets, mobs->next, temp_count);
	return temp_count;
}

int zedit_update_objs(struct zone_data *resets, struct zedit_obj_list *objs, int cmd_count, char command)
{
	int temp_count = cmd_count;

	resets->cmd[cmd_count].command = command;
	resets->cmd[cmd_count].if_flag = objs->if_flag;
	resets->cmd[cmd_count].arg1 = real_object(objs->obj_vnum);
	resets->cmd[cmd_count].arg2 = objs->max_exist;
	switch (command) {
		case 'O':
			resets->cmd[cmd_count].arg3 = real_room(objs->room_vnum);
			break;
		case 'P':
			resets->cmd[cmd_count].arg3 = real_object(objs->room_vnum);
			break;
		case 'E':
			resets->cmd[cmd_count].arg3 = objs->room_vnum;
			break;
		case 'G':
		default:
			resets->cmd[cmd_count].arg3 = 0;
			break;
	}
	resets->cmd[cmd_count].line = 0;

	temp_count++;
	if (objs->next)
		temp_count = zedit_update_objs(resets, objs->next, temp_count, command);
	return temp_count;
}

int zedit_update_rooms(struct zone_data *resets, struct zedit_room_list *rooms, int cmd_count)
{
	int temp_count = cmd_count;

	resets->cmd[cmd_count].command = 'D';
	resets->cmd[cmd_count].if_flag = rooms->if_flag;
	resets->cmd[cmd_count].arg1 = real_room(rooms->room_vnum);
	resets->cmd[cmd_count].arg2 = rooms->door_direction;
	resets->cmd[cmd_count].arg3 = rooms->door_state;
	resets->cmd[cmd_count].line = 0;

	temp_count++;
	if (rooms->next)
		temp_count = zedit_update_rooms(resets, rooms->next, temp_count);
	return temp_count;
}

int zedit_update_removes(struct zone_data *resets, struct zedit_remove_list *removes, int cmd_count) 
{
	int temp_count = cmd_count;

	resets->cmd[cmd_count].command = 'R';
	resets->cmd[cmd_count].if_flag = removes->if_flag;
	resets->cmd[cmd_count].arg1 = real_room(removes->room_vnum);
	resets->cmd[cmd_count].arg2 = real_object(removes->obj_vnum);
	resets->cmd[cmd_count].arg3 = 0;
	resets->cmd[cmd_count].line = 0;

	temp_count++;
	if (removes->next)
		temp_count = zedit_update_removes(resets, removes->next, temp_count);
	return temp_count;
}

#define Z  zone_table[rznum]
int zedit_update_zone(int rznum, struct zone_data *zone_table, struct zedit_struct *zedit_zone)
{
	/* dummy stub function */
	/* doesn't update zone_tables yet */
	int cmd_count = 0;
	struct reset_com *temp_com;
	free_zone_table(rznum, zone_table);

	Z.name = strdup(zedit_zone->name);
	Z.creator = strdup(zedit_zone->creator);
	Z.lvl_low = zedit_zone->lvl_low;
	Z.lvl_high = zedit_zone->lvl_high;
	Z.lifespan = zedit_zone->lifespan;
	Z.top	= zedit_zone->top;
	Z.number = zedit_zone->vnum;
	Z.reset_mode = zedit_zone->reset_mode;
	CREATE(temp_com, struct reset_com, zedit_zone->cmds);
	
	Z.cmd = temp_com;
	if (zedit_zone->mobs)
		cmd_count = zedit_update_mobs(&Z, zedit_zone->mobs, cmd_count);
	if (zedit_zone->objs)
		cmd_count = zedit_update_objs(&Z, zedit_zone->objs, cmd_count, 'O');
	if (zedit_zone->contains)
		cmd_count = zedit_update_objs(&Z, zedit_zone->contains, cmd_count, 'P');
	if (zedit_zone->rooms)
		cmd_count = zedit_update_rooms(&Z, zedit_zone->rooms, cmd_count);
	if (zedit_zone->removes)
		cmd_count = zedit_update_removes(&Z, zedit_zone->removes, cmd_count);

	Z.cmd[cmd_count].command = 'S';
	if (cmd_count != zedit_zone->cmds)
		log("zedit.c, zedit_update_zone: something bad has happened... cmd_count != zedit_zone->cmds");
	return TRUE;
}
#undef Z

/*  MAIN LOOP */

void zedit_parse(struct descriptor_data * d, char *arg)
{
  extern struct room_data *world;
  extern struct zone_data *zone_table;

  struct char_data *ch = d->character;
  struct zedit_struct *tempzone;
  char outbuf[256];
  int rznum = d->edit_zone;
  int vznum = d->edit_number;
  int tempi = 0;
  struct zedit_mob_list *tempmob = NULL;
  struct zedit_obj_list *tempobj = NULL;
  struct zedit_room_list *temproom = NULL;
  struct zedit_remove_list *tempremove = NULL;

  switch (d->edit_mode) {
  case ZEDIT_CONFIRM_EDIT:
	switch (*arg) {
	case 'y':
	case 'Y':
		CREATE(tempzone, struct zedit_struct, 1);
		d->zedit_zone = tempzone;
		zedit_initialize(rznum, zone_table, d->zedit_zone);
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
   		d->edit_mode = ZEDIT_MAIN_MENU;
		break;
	case 'n':
	case 'N':
		STATE(d) = CON_PLAYING;
		REMOVE_BIT(PLR_FLAGS(ch), PLR_EDITING);
		break;
	default:
		sprintf(outbuf, "%s is not a valid choice!\r\nDo you wish to edit zone %d?  ", arg, vznum);
		send_to_char(outbuf, ch);
		break;
	}
	break;   /* end of ZEDIT_CONFIRM_EDIT */
  case ZEDIT_CONFIRM_SAVEDB:
	switch (*arg) {
	case 'y':
	case 'Y':
		sprintf(outbuf, "Writing zone %d to disk\r\n", vznum);
		if (zedit_save_zone_file(rznum, zone_table))
			send_to_char("Zone has been saved.\r\n", ch);
		else
			send_to_char("Something bad happened.  Your zone was not saved.\r\n", ch);
		/* fall through into 'N' */
	case 'n':
	case 'N':
		send_to_char("Leaving ZEDIT...\r\n", ch);
		REMOVE_BIT(PLR_FLAGS(ch), PLR_EDITING);
		STATE(d) = CON_PLAYING;
		if (d->zedit_zone)
			free_zedit_struct(d->zedit_zone);
		d->zedit_zone = NULL;
		send_to_char("Done.\r\n", ch);
		break;
	default:
		sprintf(outbuf, "%s is not a valid choice!\r\n
			Do you wish to save zone %d to file?  ", arg, vznum);
		send_to_char(outbuf, ch);
		break;
	}
	break; /* end of ZEDIT_CONFIRM_SAVEDB */
  case ZEDIT_CONFIRM_SAVE:
	switch (*arg) {
	case 'y':
	case 'Y':
		sprintf(outbuf, "Updating zone %d\r\n", vznum);
		if (zedit_update_zone(rznum, zone_table, d->zedit_zone))
			send_to_char("Zone has been updated.\r\n", ch);
		else
			send_to_char("Something bad happened.  Your zone was not updated correctly.\r\n", ch);
		/* fall through into 'N' */
		d->edit_mode = ZEDIT_CONFIRM_SAVEDB;
		sprintf(outbuf, "Do you wish to save zone %d to file?  ", vznum);
		send_to_char(outbuf, ch);
		break;		
	case 'n':
	case 'N':
		send_to_char("Leaving ZEDIT...\r\n", ch);
		REMOVE_BIT(PLR_FLAGS(ch), PLR_EDITING);
		STATE(d) = CON_PLAYING;
		if (d->zedit_zone)
			free_zedit_struct(d->zedit_zone);
		d->zedit_zone = NULL;
		send_to_char("Done.\r\n", ch);
		break;
	default:
		sprintf(outbuf, "%s is not a valid choice!\r\n
			Do you wish to update zone %d internally?  ", arg, vznum);
		send_to_char(outbuf, ch);
		break;
	}
	break; /* end of ZEDIT_CONFIRM_SAVE */
  case ZEDIT_CONFIRM_EXIT:
	switch (*arg) {
	case 'y':
	case 'Y':
		d->edit_mode = ZEDIT_CONFIRM_SAVE;
		sprintf(outbuf, "Do you wish to update zone %d internally?  ", vznum);
		send_to_char(outbuf, ch);
		break;		
	case 'n':
	case 'N':
		d->edit_mode = ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
		break;
	default:
		sprintf(outbuf, "%s is not a valid choice!\r\nDo you wish to stop editing zone %d?  ",
			 arg, vznum);
		send_to_char(outbuf, ch);
		break;
	}
	break; /* end of ZEDIT_CONFIRM_STOP */
     case ZEDIT_MAIN_DESC:
	if (d->zedit_zone->name)
		free(d->zedit_zone->name);
	d->zedit_zone->name = str_dup(arg);
	d->edit_mode= ZEDIT_MAIN_MENU;
	send_to_char(KCLS, ch);
	zedit_disp_menu(d);
	break;
     case ZEDIT_MAIN_TOP:
	tempi = atoi(arg);
	if (tempi < d->zedit_zone->vnum * 100) {
		sprintf(outbuf, "%d is not a valid number! Please enter the top room vnum: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->top = tempi;
		d->edit_mode= ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
	}
	break;
     case ZEDIT_MAIN_LIFESPAN:
	tempi = atoi(arg);
	if (tempi <= 0 || tempi >= 100) {
		sprintf(outbuf, "%d is not a valid number!  Choose a number from 1 to 100: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->lifespan = tempi;
		d->edit_mode= ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
	}
	break;
     case ZEDIT_MAIN_RESET:
	tempi = atoi(arg);
	if (tempi < 0 || tempi > 2) {
		sprintf(outbuf, "%d is not a valid number!  Choose a number from 0 to 2: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->reset_mode = tempi;
		d->edit_mode= ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
	}
	break;
     case ZEDIT_MAIN_CREATOR:
	if (d->zedit_zone->creator)
		free(d->zedit_zone->creator);
	d->zedit_zone->creator = str_dup(arg);
	d->edit_mode= ZEDIT_MAIN_MENU;
	send_to_char(KCLS, ch);
	zedit_disp_menu(d);
	break;
     case ZEDIT_MAIN_MINLEV:
	tempi = atoi(arg);
	if (tempi <= 0 || tempi >= d->zedit_zone->lvl_high) {
		sprintf(outbuf, "%d is not a valid number!  Choose a number from 1 to %d: ", tempi, d->zedit_zone->lvl_high);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->lvl_low = tempi;
		d->edit_mode= ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
	}
	break;
     case ZEDIT_MAIN_MAXLEV:
	tempi = atoi(arg);
	if (tempi <= d->zedit_zone->lvl_low || tempi > LVL_IMMORT) {
		sprintf(outbuf, "%d is not a valid number!  Choose a number from %d to %d: ", tempi,
			d->zedit_zone->lvl_low, LVL_IMMORT);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->lvl_high = tempi;
		d->edit_mode= ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
	}
	break;
     case ZEDIT_MAIN_MENU:
	switch (*arg) {
	case 'a':
	case 'A':
		send_to_char("Please enter new description: ", ch);
		d->edit_mode= ZEDIT_MAIN_DESC;
		break;
	case 'b':
	case 'B':
		send_to_char("Please enter new top room's vnum: ", ch);
		d->edit_mode= ZEDIT_MAIN_TOP;
		break;
	case 'c':
	case 'C':
		send_to_char("Please enter new lifespan: ", ch);
		d->edit_mode= ZEDIT_MAIN_LIFESPAN;
		break;
	case 'd':
	case 'D':
		send_to_char("Please enter new reset mode: ", ch);
		d->edit_mode= ZEDIT_MAIN_RESET;
		break;
	case 'e':
	case 'E':
		send_to_char("Please enter creator's name: ", ch);
		d->edit_mode= ZEDIT_MAIN_CREATOR;
		break;
	case 'f':
	case 'F':
		send_to_char("Please enter new minimum level: ", ch);
		d->edit_mode= ZEDIT_MAIN_MINLEV;
		break;
	case 'g':
	case 'G':
		send_to_char("Please enter new maximum level: ", ch);
		d->edit_mode= ZEDIT_MAIN_MAXLEV;
		break;
	case 'q':
	case 'Q':
		sprintf(outbuf, "Do you wish to stop editing zone %d?  ", vznum);
	  	send_to_char(outbuf, ch);
	  	d->edit_mode= ZEDIT_CONFIRM_EXIT;
	  	break;
	case '1':
		send_to_char(KCLS, ch);
		d->zedit_zone->current_mob = d->zedit_zone->mobs;
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		d->edit_mode = ZEDIT_MOB_MENU;
		break;
	case '2':
		send_to_char(KCLS, ch);
		d->zedit_zone->current_obj = d->zedit_zone->objs;
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		d->edit_mode = ZEDIT_OBJ_MENU;
		break;
	case '3':
		send_to_char(KCLS, ch);
		d->zedit_zone->current_room = d->zedit_zone->rooms;
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		d->edit_mode = ZEDIT_ROOM_MENU;
		break;
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			 KCLS, arg);
		send_to_char(outbuf, ch);
		zedit_disp_menu(d);
		break;
	}
	break; /* end of ZEDIT_MAIN_MENU */
     case ZEDIT_MOB_MENU:
	if (isdigit(*arg) && (tempi = atoi(arg)) 
	    && (tempi > 0 && tempi <= d->zedit_zone->reg1)) {
		while (tempi-- > 1) {
			if (d->zedit_zone->current_mob && d->zedit_zone->current_mob->next)
				d->zedit_zone->current_mob = d->zedit_zone->current_mob->next;
			else
				log("zedit.c in mob_menu: Counting error while incrementing pointer");
		}
		d->zedit_zone->current_obj = d->zedit_zone->current_mob->inventory;
		d->zedit_zone->reg2 = ZEDIT_INVENTORY_EDIT;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		d->edit_mode = ZEDIT_MOB_EDIT;
	} else switch(*arg) {
	case 'a':
	case 'A':
		d->zedit_zone->cmds += 1;
		CREATE(tempmob, struct zedit_mob_list, 1);
		tempmob->if_flag = 0;
		tempmob->mob_vnum = 1;
		tempmob->max_exist = 1;
		tempmob->room_vnum = 0;
		tempmob->inventory = NULL;
		if (d->zedit_zone->mobs) {
			tempmob->next = d->zedit_zone->mobs;
			tempmob->next->previous = tempmob;
		} else {
			tempmob->next = NULL;
		}
		tempmob->previous = tempmob;
		d->zedit_zone->mobs = tempmob;
		d->zedit_zone->current_mob = tempmob;
		d->zedit_zone->current_obj = NULL;
		tempmob = NULL;
		d->edit_mode = ZEDIT_MOB_EDIT;		
		d->zedit_zone->reg2 = ZEDIT_INVENTORY_EDIT;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'q':
	case 'Q':
		d->zedit_zone->current_mob = d->zedit_zone->mobs;
		d->zedit_zone->current_obj = d->zedit_zone->objs;
		d->edit_mode = ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
		break;
	case 'n':
	case 'N':
		if (d->zedit_zone->reg1 >= 9 && d->zedit_zone->current_mob && d->zedit_zone->current_mob->next)
			d->zedit_zone->current_mob = d->zedit_zone->current_mob->next;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		break;
	case 'p':
	case 'P':
		if (d->zedit_zone->current_mob)
			d->zedit_zone->current_mob = d->zedit_zone->current_mob->previous;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		break;
	case 'y':
	case 'Y':
		if (d->zedit_zone->mobs)
			d->zedit_zone->current_mob = d->zedit_zone->mobs;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		break;
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		break;		
	}
	break; /* end of ZEDIT_MOB_MENU */
     case ZEDIT_MOB_NAME:
	tempi = atoi(arg);
	if (real_mobile(tempi) == -1) {
		sprintf(outbuf, "%d is not a valid number! Please enter the mob's vnum: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->current_mob->mob_vnum = tempi;
		d->edit_mode= ZEDIT_MOB_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_mob_edit_menu(d);
	}
	break;
     case ZEDIT_MOB_ROOM:
	tempi = atoi(arg);
	if (real_room(tempi) == -1) {
		sprintf(outbuf, "%d is not a valid number! Please enter the room's vnum: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->current_mob->room_vnum = tempi;
		d->edit_mode= ZEDIT_MOB_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_mob_edit_menu(d);
	}
	break;
     case ZEDIT_MOB_MAXNUM:
	tempi = atoi(arg);
	if (tempi <= 0 || tempi > 1000) {
		sprintf(outbuf, "%d is not a valid number! Please enter a number from 1 to 1000: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->current_mob->max_exist = tempi;
		d->edit_mode= ZEDIT_MOB_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_mob_edit_menu(d);
	}
	break;
     case ZEDIT_MOB_VNUM:
	d->edit_mode=ZEDIT_MOB_EDIT;
	send_to_char(KCLS, ch);
	zedit_disp_mob_edit_menu(d);
	break;
     case ZEDIT_MOB_EDIT:
	if (isdigit(*arg) && (tempi = atoi(arg)) 
	    && (tempi > 0 && tempi <= d->zedit_zone->reg1)) {
		while (tempi-- > 1) {
			if (d->zedit_zone->current_obj && d->zedit_zone->current_obj->next)
				d->zedit_zone->current_obj = d->zedit_zone->current_obj->next;
			else
				log("zedit.c in mob_edit: Counting error while incrementing pointer");
		}
		send_to_char(KCLS, ch);
		if (d->zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT) {
			zedit_disp_inventory_edit_menu(d);
			d->edit_mode = ZEDIT_INVENTORY_EDIT;
		} else {
			zedit_disp_equipment_edit_menu(d);
			d->edit_mode = ZEDIT_EQUIPMENT_EDIT;
		}
	} else switch(*arg) {
	case 'a':
	case 'A':
		d->zedit_zone->cmds += 1;
		CREATE(tempobj, struct zedit_obj_list, 1);
		d->zedit_zone->current_obj = tempobj;
		tempobj->if_flag = 1;
		tempobj->obj_vnum = 1;
		tempobj->max_exist = 1;
		tempobj->room_vnum = 0;
		send_to_char(KCLS, ch);
		if (d->zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT) {
			if (d->zedit_zone->current_mob->inventory) {
				tempobj->next = d->zedit_zone->current_mob->inventory;
				tempobj->next->previous = tempobj;
			} else {
				tempobj->next = NULL;
			}
			d->zedit_zone->current_mob->inventory = tempobj;
			d->edit_mode = ZEDIT_INVENTORY_EDIT;		
			zedit_disp_inventory_edit_menu(d);
		} else {
			if (d->zedit_zone->current_mob->equipment) {
				tempobj->next = d->zedit_zone->current_mob->equipment;
				tempobj->next->previous = tempobj;
			} else {
				tempobj->next = NULL;
			}
			d->zedit_zone->current_mob->equipment = tempobj;
			d->edit_mode = ZEDIT_EQUIPMENT_EDIT;		
			zedit_disp_equipment_edit_menu(d);
		}
		tempobj->previous = tempobj;
		d->zedit_zone->current_obj = tempobj;
		tempobj = NULL;
		break;
	case 'c':
	case 'C':
		send_to_char("Please enter new mob's vnum: ", ch);
		d->edit_mode= ZEDIT_MOB_NAME;
		break;
	case 'd':
	case 'D':
		tempmob = d->zedit_zone->current_mob;
		if (tempmob->previous != tempmob) { /* not head of list */
			tempmob->previous->next = tempmob->next;
			if (tempmob->next)
				tempmob->next->previous = tempmob->previous;
		} else {
			d->zedit_zone->mobs = tempmob->next;
			if (tempmob->next)
				tempmob->next->previous = tempmob->next;
		}
		d->zedit_zone->current_mob = d->zedit_zone->mobs;
		tempmob->next = NULL;
		free_zedit_mobs(tempmob, 1);
		tempmob = NULL;
		d->zedit_zone->cmds -= 1;
		/* FALLS INTO 'Q'... I'm Lazy */
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_MOB_MENU;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_menu(d);
		break;
	case 'm':
	case 'M':
		send_to_char("Please enter new maximum: ", ch);
		d->edit_mode= ZEDIT_MOB_MAXNUM;
		break;
	case 'n':
	case 'N':
		if (d->zedit_zone->reg1 >= 9 && d->zedit_zone->current_obj && d->zedit_zone->current_obj->next)
			d->zedit_zone->current_obj = d->zedit_zone->current_obj->next;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'p':
	case 'P':
		if (d->zedit_zone->current_obj)
			d->zedit_zone->current_obj = d->zedit_zone->current_obj->previous;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'y':
	case 'Y':
		if (d->zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT) {
			d->zedit_zone->current_obj = d->zedit_zone->current_mob->inventory;
		} else {
			d->zedit_zone->current_obj = d->zedit_zone->current_mob->equipment;	
		}
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break; 
	case 'r':
	case 'R':
		send_to_char("Please enter new room's vnum: ", ch);
		d->edit_mode= ZEDIT_MOB_ROOM;
		break;
	case 't':
	case 'T':
		if (d->zedit_zone->reg2 == ZEDIT_INVENTORY_EDIT) {
			d->zedit_zone->reg2 = ZEDIT_EQUIPMENT_EDIT;
			d->zedit_zone->current_obj = d->zedit_zone->current_mob->equipment;
		} else {
			d->zedit_zone->reg2 = ZEDIT_INVENTORY_EDIT;
			d->zedit_zone->current_obj = d->zedit_zone->current_mob->inventory;	
		}
		send_to_char(KCLS, ch);		
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'v':
	case 'V':
		send_to_char("Please enter new mob's name: ", ch);
		d->edit_mode= ZEDIT_MOB_VNUM;
		break;
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;		
	}
        break; /* MOB_EDIT */
     case ZEDIT_OBJ_MENU:
	if (isdigit(*arg) && (tempi = atoi(arg)) 
	    && (tempi > 0 && tempi <= d->zedit_zone->reg1)) {
		while (tempi-- > 1) {
			if (d->zedit_zone->current_obj && d->zedit_zone->current_obj->next)
				d->zedit_zone->current_obj = d->zedit_zone->current_obj->next;
			else
				log("zedit.c in obj_edit: Counting error while incrementing pointer");
		}
		send_to_char(KCLS, ch);
		zedit_disp_object_edit_menu(d);
		d->edit_mode = ZEDIT_OBJECT_EDIT;
	} else switch(*arg) {
	case 'a':
	case 'A':
		d->zedit_zone->cmds += 1;
		CREATE(tempobj, struct zedit_obj_list, 1);
		d->zedit_zone->current_obj = tempobj;
		tempobj->if_flag = 0;
		tempobj->obj_vnum = 1;
		tempobj->max_exist = 1;
		tempobj->room_vnum = 0;
		send_to_char(KCLS, ch);
		if (d->zedit_zone->objs) {
			tempobj->next = d->zedit_zone->objs;
			tempobj->next->previous = tempobj;
		} else {
			tempobj->next = NULL;
		}
		d->zedit_zone->objs = tempobj;
		d->edit_mode = ZEDIT_OBJECT_EDIT;		
		zedit_disp_object_edit_menu(d);
		tempobj->previous = tempobj;
		tempobj = NULL;
		break;
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
		break;
	case 'n':
	case 'N':
		if (d->zedit_zone->reg1 >= 9 && d->zedit_zone->current_obj && d->zedit_zone->current_obj->next)
			d->zedit_zone->current_obj = d->zedit_zone->current_obj->next;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		break;
	case 'p':
	case 'P':
		if (d->zedit_zone->current_obj)
			d->zedit_zone->current_obj = d->zedit_zone->current_obj->previous;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		break;
	case 'y':
	case 'Y':
		if (d->zedit_zone->objs)
			d->zedit_zone->current_obj = d->zedit_zone->objs;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		break;
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		break;		
	}
        break; /* OBJECT_MENU */
     case ZEDIT_INVENTORY_NAME:
     case ZEDIT_EQUIPMENT_NAME:
     case ZEDIT_OBJECT_NAME:
	tempi = atoi(arg);	
	if (real_object(tempi) == -1) {
		sprintf(outbuf, "%d is not a valid number! Please enter the object's vnum: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->current_obj->obj_vnum = tempi;
		send_to_char(KCLS, ch);
		switch (d->edit_mode) {
		case ZEDIT_INVENTORY_NAME:
			d->edit_mode= ZEDIT_INVENTORY_EDIT;
			zedit_disp_inventory_edit_menu(d);
			break;
		case ZEDIT_EQUIPMENT_NAME:
			d->edit_mode= ZEDIT_EQUIPMENT_EDIT;
			zedit_disp_equipment_edit_menu(d);
			break;
		case ZEDIT_OBJECT_NAME:
			d->edit_mode= ZEDIT_OBJECT_EDIT;
			zedit_disp_object_edit_menu(d);
			break;
		default:
			log("zedit.c, OBJECT_NAME: got here somehow... bad");
			d->edit_mode= ZEDIT_MAIN_MENU;
			zedit_disp_menu(d);
			break;
		}	
	}
	break;
     case ZEDIT_INVENTORY_MAXNUM:
     case ZEDIT_EQUIPMENT_MAXNUM:
     case ZEDIT_OBJECT_MAXNUM:
	tempi = atoi(arg);
	d->zedit_zone->current_obj->max_exist= tempi;
	send_to_char(KCLS, ch);
	switch (d->edit_mode) {
	case ZEDIT_INVENTORY_MAXNUM:
		d->edit_mode= ZEDIT_INVENTORY_EDIT;
		zedit_disp_inventory_edit_menu(d);
		break;
	case ZEDIT_EQUIPMENT_MAXNUM:
		d->edit_mode= ZEDIT_EQUIPMENT_EDIT;
		zedit_disp_equipment_edit_menu(d);
		break;
	case ZEDIT_OBJECT_MAXNUM:
		d->edit_mode= ZEDIT_OBJECT_EDIT;
		zedit_disp_object_edit_menu(d);
		break;
	default:
		log("zedit.c, OBJECT_MAXNUM: got here somehow... bad");
		d->edit_mode= ZEDIT_MAIN_MENU;
		zedit_disp_menu(d);
		break;
	}			
	break;
     case ZEDIT_EQUIPMENT_LOC:
	tempi = atoi(arg);
	if (tempi >= NUM_WEARS || tempi < 0) {
	        sprintf(outbuf, "%d is not a valid number! Please enter the wear location between 0 and %d: ", tempi, NUM_WEARS - 1);
                send_to_char(outbuf, ch);
  	} else {	
		d->zedit_zone->current_obj->room_vnum= tempi;
		send_to_char(KCLS, ch);
		d->edit_mode= ZEDIT_EQUIPMENT_EDIT;
		zedit_disp_equipment_edit_menu(d);
	}
	break;
     case ZEDIT_OBJECT_LOC:
	tempi = atoi(arg);
	if (real_room(tempi) == -1) {
	        sprintf(outbuf, "%d is not a valid number! Please enter a valid room vnum: ", tempi);
                send_to_char(outbuf, ch);
  	} else {	
		d->zedit_zone->current_obj->room_vnum= tempi;
		send_to_char(KCLS, ch);
		d->edit_mode= ZEDIT_OBJECT_EDIT;
		zedit_disp_object_edit_menu(d);
	}
	break;
     case ZEDIT_INVENTORY_EDIT:
	switch(*arg) {
	case 'c':
	case 'C':
		send_to_char("Please enter new object's vnum: ", ch);
		d->edit_mode= ZEDIT_INVENTORY_NAME;
		break;
	case 'd':
	case 'D':
		tempobj = d->zedit_zone->current_obj;
		if (tempobj->previous != tempobj) { /* not head of list */
			tempobj->previous->next = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->previous;
		} else {
			d->zedit_zone->current_mob->inventory = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->next;
		}
		d->zedit_zone->current_obj = d->zedit_zone->current_mob->inventory;
		tempobj->next = NULL;
		free_zedit_objs(tempobj, 1);
		tempobj = NULL;
		d->zedit_zone->cmds -= 1;
		/* FALLS INTO 'Q'... I'm Lazy */
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_MOB_EDIT;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'm':
	case 'M':
		send_to_char("Please enter new maximum: ", ch);
		d->edit_mode= ZEDIT_INVENTORY_MAXNUM;
		break;
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		zedit_disp_inventory_edit_menu(d);
		break;		
	}
        break; /* INVENTORY_EDIT */
     case ZEDIT_EQUIPMENT_EDIT:
	switch(*arg) {
	case 'c':
	case 'C':
		send_to_char("Please enter new object's vnum: ", ch);
		d->edit_mode= ZEDIT_EQUIPMENT_NAME;
		break;
	case 'd':
	case 'D':
		tempobj = d->zedit_zone->current_obj;
		if (tempobj->previous != tempobj) { /* not head of list */
			tempobj->previous->next = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->previous;
		} else {
			d->zedit_zone->current_mob->equipment = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->next;
		}
		d->zedit_zone->current_obj = d->zedit_zone->current_mob->equipment;
		tempobj->next = NULL;
		free_zedit_objs(tempobj, 1);
		tempobj = NULL;
		d->zedit_zone->cmds -= 1;
		/* FALLS INTO 'Q'... I'm Lazy */
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_MOB_EDIT;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_mob_edit_menu(d);
		break;
	case 'm':
	case 'M':
		send_to_char("Please enter new maximum: ", ch);
		d->edit_mode= ZEDIT_EQUIPMENT_MAXNUM;
		break;
	case 'l':
	case 'L':
		send_to_char("Please enter new location: ", ch);
		d->edit_mode = ZEDIT_EQUIPMENT_LOC;
		break;		
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		zedit_disp_equipment_edit_menu(d);
		break;		
	}
        break; /* EQUIPMENT_EDIT */
     case ZEDIT_OBJECT_EDIT:
	switch(*arg) {
	case 'c':
	case 'C':
		send_to_char("Please enter new object's vnum: ", ch);
		d->edit_mode= ZEDIT_OBJECT_NAME;
		break;
	case 'd':
	case 'D':
		tempobj = d->zedit_zone->current_obj;
		if (tempobj->previous != tempobj) { /* not head of list */
			tempobj->previous->next = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->previous;
		} else {
			d->zedit_zone->objs = tempobj->next;
			if (tempobj->next)
				tempobj->next->previous = tempobj->next;
		}
		d->zedit_zone->current_obj = d->zedit_zone->objs;
		tempobj->next = NULL;
		free_zedit_objs(tempobj, 1);
		tempobj = NULL;
		d->zedit_zone->cmds -= 1;
		/* FALLS INTO 'Q'... I'm Lazy */
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_OBJ_MENU;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_obj_menu(d);
		break;
	case 'm':
	case 'M':
		send_to_char("Please enter new maximum: ", ch);
		d->edit_mode= ZEDIT_OBJECT_MAXNUM;
		break;
	case 'l':
	case 'L':
		send_to_char("Please enter new location: ", ch);
		d->edit_mode = ZEDIT_OBJECT_LOC;
		break;		
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		zedit_disp_object_edit_menu(d);
		break;		
	}
        break; /* OBJECT_EDIT */
     case ZEDIT_ROOM_MENU:
	if (isdigit(*arg) && (tempi = atoi(arg)) 
	    && (tempi > 0 && tempi <= d->zedit_zone->reg1)) {
		while (tempi-- > 1) {
			if (d->zedit_zone->current_room && d->zedit_zone->current_room->next)
				d->zedit_zone->current_room = d->zedit_zone->current_room->next;
			else
				log("zedit.c in room_menu: Counting error while incrementing pointer");
		}
		send_to_char(KCLS, ch);
		zedit_disp_room_edit_menu(d);
		d->edit_mode = ZEDIT_ROOM_EDIT;
	} else switch(*arg) {
	case 'a':
	case 'A':
		d->zedit_zone->cmds += 1;
		CREATE(temproom, struct zedit_room_list, 1);
		temproom->if_flag = 0;
		temproom->room_vnum = 0;
		temproom->previous = temproom;
		if (d->zedit_zone->rooms) {
			temproom->next = d->zedit_zone->rooms;
			temproom->next->previous = temproom;
		} else {
			temproom->next = NULL;
		}
		d->zedit_zone->rooms = temproom;
		d->zedit_zone->current_room = temproom;
		temproom = NULL;
		d->edit_mode = ZEDIT_ROOM_EDIT;		
		send_to_char(KCLS, ch);
		zedit_disp_room_edit_menu(d);
		break;
	case 'q':
	case 'Q':
		d->zedit_zone->current_mob = d->zedit_zone->mobs;
		d->zedit_zone->current_obj = d->zedit_zone->objs;
		d->zedit_zone->current_room = d->zedit_zone->rooms;
		d->edit_mode = ZEDIT_MAIN_MENU;
		send_to_char(KCLS, ch);
		zedit_disp_menu(d);
		break;
	case 'n':
	case 'N':
		if (d->zedit_zone->reg1 >= 9 && d->zedit_zone->current_room && d->zedit_zone->current_room->next)
			d->zedit_zone->current_room = d->zedit_zone->current_room->next;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		break;
	case 'p':
	case 'P':
		if (d->zedit_zone->current_room)
			d->zedit_zone->current_room = d->zedit_zone->current_room->previous;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		break;
	case 'y':
	case 'Y':
		if (d->zedit_zone->rooms)
			d->zedit_zone->current_room = d->zedit_zone->rooms;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		break;
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		break;		
	}
	break; /* end of ZEDIT_ROOM_MENU */
     case ZEDIT_ROOM_NAME:
	tempi = atoi(arg);
	if (real_room(tempi) == -1) {
		sprintf(outbuf, "%d is not a valid number! Please enter the room's vnum: ", tempi);
		send_to_char(outbuf, ch);
	} else {
		d->zedit_zone->current_room->room_vnum = tempi;
		d->edit_mode= ZEDIT_ROOM_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_room_edit_menu(d);
	}
	break;
     case ZEDIT_ROOM_DIR:
	tempi = atoi(arg);
	if (tempi < 0 || tempi > 5) {
		sprintf(outbuf, "%d is not a valid number! Please enter the door's direction number:\r\n", tempi);
		send_to_char(outbuf, ch);
		send_to_char(" 0) North  1) East  2) South  3) West  4) Up  5) Down  Your choice[0-5]: ", ch);
	} else {
		d->zedit_zone->current_room->door_direction = tempi;
		d->edit_mode= ZEDIT_ROOM_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_room_edit_menu(d);
	}
	break;
     case ZEDIT_ROOM_STATE:
	tempi = atoi(arg);
	if (tempi < 0 || tempi > 2) {
		sprintf(outbuf, "%d is not a valid number! Please enter the door's state:\r\n ", tempi);
		send_to_char(outbuf, ch);
		send_to_char(" 0) Open   1) Closed   2) Closed and Locked   Your choice[0-2]: ", ch);
	} else {
		d->zedit_zone->current_room->door_state = tempi;
		d->edit_mode= ZEDIT_ROOM_EDIT;
		send_to_char(KCLS, ch);
		zedit_disp_room_edit_menu(d);
	}
	break;
     case ZEDIT_ROOM_EDIT:
	switch(*arg) {
	case 'r':
	case 'R':
		send_to_char("Please enter new room's vnum: ", ch);
		d->edit_mode= ZEDIT_ROOM_NAME;
		break;
	case 'd':
	case 'D':
		temproom = d->zedit_zone->current_room;
		if (temproom->previous != temproom) { /* not head of list */
			temproom->previous->next = temproom->next;
			if (temproom->next)
				temproom->next->previous = temproom->previous;
		} else {
			d->zedit_zone->rooms = temproom->next;
			if (temproom->next)
				temproom->next->previous = temproom->next;
		}
		d->zedit_zone->current_room = d->zedit_zone->rooms;
		temproom->next = NULL;
		free_zedit_rooms(temproom, 1);
		temproom = NULL;
		d->zedit_zone->cmds -= 1;
		/* FALLS INTO 'Q'... I'm Lazy */
	case 'q':
	case 'Q':
		d->edit_mode = ZEDIT_ROOM_MENU;
		send_to_char(KCLS, ch);
		d->zedit_zone->reg1 = zedit_disp_room_menu(d);
		break;
	case 's':
	case 'S':
		send_to_char("Please enter new state:\r\n ", ch);
		send_to_char(" 0) Open   1) Closed   2) Closed and Locked   Your choice[0-2]: ", ch);
		d->edit_mode= ZEDIT_ROOM_STATE;
		break;
	case 'c':
	case 'C':
		send_to_char("Please enter new direction: \r\n", ch);
		send_to_char(" 0) North  1) East  2) South  3) West  4) Up  5) Down  Your choice[0-5]: ", ch);
		d->edit_mode = ZEDIT_ROOM_DIR;
		break;		
	case 'z':	/* out of bounds */
	case 'Z':
	default:
		sprintf(outbuf, "%s%s is not a valid choice!\r\nPlease try again.\r\n",
			KCLS, arg);
		send_to_char(outbuf, ch);
		zedit_disp_room_edit_menu(d);
		break;		
	}
        break; /* ROOM_EDIT */
     default:
	d->edit_mode = ZEDIT_MAIN_MENU;	
	zedit_disp_menu(d);
	break;	
     }
}
