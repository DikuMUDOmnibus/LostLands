/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "rooms.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

int do_guild(struct char_data *ch, void *me, int cmd, char *argument, int skilltype);

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  if (real_mobile(mob) >= 0)
    mob_index[real_mobile(mob)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
	    mob);
    log(buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
	    obj);
    log(buf);
  }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
	    room);
    log(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  SPECIAL(postmaster);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(mageguild);
  SPECIAL(clericguild);
  SPECIAL(fighterguild);
  SPECIAL(thiefguild);
  SPECIAL(othersguild);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  SPECIAL(cleric);
  SPECIAL(recruiter);
  SPECIAL(trainer);

  /* Immortal Zone */
  ASSIGNMOB(1, puff);
  ASSIGNMOB(30, receptionist);
  ASSIGNMOB(31, postmaster);
  ASSIGNMOB(32, janitor);

  /* MOB POOL */
  ASSIGNMOB(101, magic_user);
  ASSIGNMOB(102, magic_user);
  ASSIGNMOB(103, magic_user);
  ASSIGNMOB(104, magic_user);
  ASSIGNMOB(208, fido);
  ASSIGNMOB(1178, magic_user);

  /* Capitol */
  ASSIGNMOB(300, trainer);
  ASSIGNMOB(4000, postmaster);  
  ASSIGNMOB(4005, receptionist);
  ASSIGNMOB(4006, mageguild);
  ASSIGNMOB(4007, clericguild); 
  ASSIGNMOB(4008, othersguild); /* druid guild */ 
  ASSIGNMOB(4009, thiefguild); /* thieves guild */ 
  ASSIGNMOB(4010, fighterguild); /* fighter guild */ 
  ASSIGNMOB(4011, guild_guard);
  ASSIGNMOB(4012, guild_guard);
  ASSIGNMOB(4013, guild_guard);
  ASSIGNMOB(4014, guild_guard);
  ASSIGNMOB(4015, guild_guard); /* thief */
  ASSIGNMOB(4017, recruiter);
  ASSIGNMOB(4018, cityguard);
  ASSIGNMOB(4019, guild_guard);	/* embassy */

/* Midgaard
  ASSIGNMOB(3005, receptionist);
  ASSIGNMOB(3010, postmaster);
  ASSIGNMOB(3020, guild);
  ASSIGNMOB(3021, guild);
  ASSIGNMOB(3022, guild);
  ASSIGNMOB(3023, guild);
  ASSIGNMOB(3024, guild_guard);
  ASSIGNMOB(3025, guild_guard);
  ASSIGNMOB(3026, guild_guard);
  ASSIGNMOB(3027, guild_guard);
  ASSIGNMOB(3059, cityguard);
  ASSIGNMOB(3060, cityguard);
  ASSIGNMOB(3061, janitor);
  ASSIGNMOB(3062, fido);
  ASSIGNMOB(3066, fido);
  ASSIGNMOB(3067, cityguard);
  ASSIGNMOB(3068, janitor);
  ASSIGNMOB(3095, cryogenicist);
  ASSIGNMOB(3105, mayor);

  MORIA 
  ASSIGNMOB(4000, snake);
  ASSIGNMOB(4001, snake);
  ASSIGNMOB(4053, snake);
  ASSIGNMOB(4100, magic_user);
  ASSIGNMOB(4102, snake);
  ASSIGNMOB(4103, thief);

  Redferne's 
  ASSIGNMOB(7900, cityguard);

  PYRAMID 
  ASSIGNMOB(5300, snake);
  ASSIGNMOB(5301, snake);
  ASSIGNMOB(5304, thief);
  ASSIGNMOB(5305, thief);
  ASSIGNMOB(5309, magic_user); 
  ASSIGNMOB(5311, magic_user);
  ASSIGNMOB(5313, magic_user); 
  ASSIGNMOB(5314, magic_user); 
  ASSIGNMOB(5315, magic_user); 
  ASSIGNMOB(5316, magic_user); 
  ASSIGNMOB(5317, magic_user);

  High Tower Of Sorcery 
  ASSIGNMOB(2501, magic_user); 
  ASSIGNMOB(2504, magic_user);
  ASSIGNMOB(2507, magic_user);
  ASSIGNMOB(2508, magic_user);
  ASSIGNMOB(2510, magic_user);
  ASSIGNMOB(2511, thief);
  ASSIGNMOB(2514, magic_user);
  ASSIGNMOB(2515, magic_user);
  ASSIGNMOB(2516, magic_user);
  ASSIGNMOB(2517, magic_user);
  ASSIGNMOB(2518, magic_user);
  ASSIGNMOB(2520, magic_user);
  ASSIGNMOB(2521, magic_user);
  ASSIGNMOB(2522, magic_user);
  ASSIGNMOB(2523, magic_user);
  ASSIGNMOB(2524, magic_user);
  ASSIGNMOB(2525, magic_user);
  ASSIGNMOB(2526, magic_user);
  ASSIGNMOB(2527, magic_user);
  ASSIGNMOB(2528, magic_user);
  ASSIGNMOB(2529, magic_user);
  ASSIGNMOB(2530, magic_user);
  ASSIGNMOB(2531, magic_user);
  ASSIGNMOB(2532, magic_user);
  ASSIGNMOB(2533, magic_user);
  ASSIGNMOB(2534, magic_user);
  ASSIGNMOB(2536, magic_user);
  ASSIGNMOB(2537, magic_user);
  ASSIGNMOB(2538, magic_user);
  ASSIGNMOB(2540, magic_user);
  ASSIGNMOB(2541, magic_user);
  ASSIGNMOB(2548, magic_user);
  ASSIGNMOB(2549, magic_user);
  ASSIGNMOB(2552, magic_user);
  ASSIGNMOB(2553, magic_user);
  ASSIGNMOB(2554, magic_user);
  ASSIGNMOB(2556, magic_user);
  ASSIGNMOB(2557, magic_user);
  ASSIGNMOB(2559, magic_user);
  ASSIGNMOB(2560, magic_user);
  ASSIGNMOB(2562, magic_user);
  ASSIGNMOB(2564, magic_user);

  SEWERS 
  ASSIGNMOB(7006, snake);
  ASSIGNMOB(7009, magic_user);
  ASSIGNMOB(7200, magic_user);
  ASSIGNMOB(7201, magic_user);
  ASSIGNMOB(7202, magic_user);
*/
/* FOREST 
  ASSIGNMOB(6112, magic_user);
  ASSIGNMOB(6113, snake);
  ASSIGNMOB(6114, magic_user);
  ASSIGNMOB(6115, magic_user);
  ASSIGNMOB(6116, magic_user);
  ASSIGNMOB(6117, magic_user);
 
  ARACHNOS 
  ASSIGNMOB(6302, magic_user);
  ASSIGNMOB(6309, magic_user);
  ASSIGNMOB(6312, magic_user);
  ASSIGNMOB(6314, magic_user);
  ASSIGNMOB(6315, magic_user);

Desert
  ASSIGNMOB(5004, magic_user);
  ASSIGNMOB(5005, guild_guard);
  ASSIGNMOB(5010, magic_user);
  ASSIGNMOB(5014, magic_user);
 */
  /* Drow City
  ASSIGNMOB(5103, magic_user);
  ASSIGNMOB(5104, magic_user);
  ASSIGNMOB(5107, magic_user);
  ASSIGNMOB(5108, magic_user);
 */
  /* Old Thalos 
  ASSIGNMOB(5200, magic_user);
  ASSIGNMOB(5201, magic_user);
  ASSIGNMOB(5209, magic_user);
*/
  /* New Thalos 
  ASSIGNMOB(5404, receptionist);
  ASSIGNMOB(5421, magic_user);
  ASSIGNMOB(5422, magic_user);
  ASSIGNMOB(5423, magic_user);
  ASSIGNMOB(5424, magic_user);
  ASSIGNMOB(5425, magic_user);
  ASSIGNMOB(5426, magic_user);
  ASSIGNMOB(5427, magic_user);
  ASSIGNMOB(5428, magic_user);
  ASSIGNMOB(5434, cityguard);
  ASSIGNMOB(5440, magic_user);
  ASSIGNMOB(5455, magic_user);
  ASSIGNMOB(5461, cityguard);
  ASSIGNMOB(5462, cityguard);
  ASSIGNMOB(5463, cityguard);
  ASSIGNMOB(5482, cityguard);
*/
/*
5400 - Guildmaster (Mage)
5401 - Guildmaster (Cleric)
5402 - Guildmaster (Warrior)
5403 - Guildmaster (Thief)
5456 - Guildguard (Mage)
5457 - Guildguard (Cleric)
5458 - Guildguard (Warrior)
5459 - Guildguard (Thief)
*/

  /* ROME
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);
 */
  /* King Welmar's Castle (not covered in castle.c)
  ASSIGNMOB(15015, thief);
  ASSIGNMOB(15032, magic_user);
 */
  /* DWARVEN KINGDOM 
  ASSIGNMOB(6500, cityguard);
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6516, magic_user);
*/
}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(bank);
  SPECIAL(gen_board);

  ASSIGNOBJ(4000, gen_board);	/* elf's board */   
  ASSIGNOBJ(4001, gen_board);   /* dwarf's board */
  ASSIGNOBJ(4002, gen_board);   /* gnome's board */
  ASSIGNOBJ(4003, gen_board);	/* halfling's board */
  ASSIGNOBJ(4004, gen_board);   /* figher's board */
  ASSIGNOBJ(4005, gen_board);   /* cleric's board */
  ASSIGNOBJ(4006, gen_board);   /* mage's board */
  ASSIGNOBJ(4007, gen_board);   /* thief's board */
  ASSIGNOBJ(4008, gen_board);   /* immort */
  ASSIGNOBJ(4009, gen_board);   /* morts */
  ASSIGNOBJ(4010, gen_board);   /* freeze */
  ASSIGNOBJ(4011, gen_board);   /* social (adventurers' guild)*/

  ASSIGNOBJ(4012, bank);
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  SPECIAL(dump);
  SPECIAL(pet_shops);
  SPECIAL(pray_for_items);
  SPECIAL(war_reg);
  SPECIAL(bounty_reg);
  SPECIAL(assass_reg);
  SPECIAL(engraver);

  ASSIGNROOM(4088, war_reg);
  ASSIGNROOM(4228, pet_shops);
  ASSIGNROOM(4194, engraver);
  ASSIGNROOM(4202, bounty_reg);
  ASSIGNROOM(4118, assass_reg);
  ASSIGNROOM(4106, dump);
 
  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}
