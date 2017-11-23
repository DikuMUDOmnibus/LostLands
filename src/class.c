/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"

void 	obj_from_char(struct obj_data *obj);
void 	extract_obj(struct obj_data *obj);

#define MIN_ATTRIBUTE_AVE 13
/* Names first */

const char *class_abbrevs[] = {
  "Mag",
  "Cle",
  "Thi",
  "Fig",
  "Bar",
  "Sam",
  "Dru",
  "Wiz",
  "Mon",
  "Ava",
  "Nin",
  "Dua",
  "Tri",
  "\n"
};

const char *pc_class_types[] = {
  "Magic-User",
  "Cleric",
  "Thief",
  "Fighter",
  "Barbarian",
  "Samurai",
  "Druid",
  "Wizard",
  "Monk",
  "Avatar",
  "Ninja",
  "Dual-Class",
  "Triple-Class",
  "\n"
};

const char *race_abbrevs[] = {
  "HUM",
  "ELF",
  "H-E",
  "DWA",
  "HAL",
  "GNO",
  "HEM",
  "LLY",
  "MIN",
  "PIX",
  "HEN",
  "DRA",
  "NOR",
  "UND",
  "HMD",
  "ANI",
  "GIA",
  "INS",
  "DEM",
  "\n"
};

const char *pc_race_types[] = {
  "Human",
  "Elf",
  "Half-Elf",
  "Dwarf",
  "Halfling",
  "Gnome",
  "Hemnov",
  "Llyran",
  "Minotaur",
  "Pixie",
  "Hengyokai",
  "DragonLord",	/* last PC class */
  "Other",
  "Undead",
  "Humanoid",
  "Animal",
  "Giant",
  "Insectoid",
  "Demon",
  "\n"
};

/* The menu for choosing a race in interpreter.c: */
const char *race_menu1 =
"\r\nYour race determines the modifiers to your attributes and what classes\r\n"
"or multi-classes are available to you.\r\n"
"    Race also decides what players you can player-kill or can PK you.\n"
"   Race        Political Stance                 Attribute Mods\n"
"1) Human       Neutral                          None\n"
"2) Elf         Member of Yllantra Collective    +1 Dex, -1 Con\r\n"
"3) Halfelf     Neutral                          None\r\n"
"4) Halfling    Member of Yllantra Collective    +1 Dex, -1 Str\r\n"
"5) Gnome       Neutral                          +1 Int, -1 Wis\r\n"
"6) Dark Gnome  Member of Druhari Coalition      +1 Int, -1 Wis\r\n"
"7) Dwarf       Member of Druhari Coalition      +1 Con, -1 Cha\r\n";

const char *race_menu2 =
"8) Minotaur    Neutral                          +2 Str +2 Con, -2 Wis -2 Cha\r\n"
"9) Pixie       Neutral                          +1 Int, -1 Con\r\n";

const char *race_menu3 =
"v) Hemnov      Neutral                          ???\r\n"
"y) Llyran      Neutral                          ???\r\n";

const char *race_menu4 =
"w) Hengyokai   Neutral                          ???\r\n";

const char *race_menu5 =
"d) DragonLord  Neutral                          ???\r\n";

const char *race_menu6 =
"The Yllantra Collective are at war against the Druhari Coalition, and all\r\n"
"their members are automatically registered for war.\r\n"
"Non-members can register at the recruitment center in Capitol.\r\n"
"Gnomes are currently in civil war when the Dark Gnomes broke off and joined\r\n"
" the Druhari Coalition.\r\n"
"Except for Humans, multi-class characters are available.\r\n\r\n";

int parse_race(char arg)
{

  int i = CLASS_UNDEFINED;
  arg = LOWER(arg);

  switch (arg) {
  case '1':
    i = RACE_HUMAN;
    break;
  case '2':
    i = RACE_ELF;
    break;
  case '3':
    i = RACE_HALFELF;
    break;
  case '4':
    i = RACE_HALFLING;
    break;
  case '5':
    i = RACE_GNOME;
    break;
  case '6':
    i = RACE_DARKGNOME;
    break;
  case '7':
    i = RACE_DWARF;
    break;
  case '8':
    i = RACE_MINOTAUR;
    break;
  case '9':
    i = RACE_PIXIE;
    break;
  case 'v':
    i = RACE_HEMNOV; 
    break; 
  case 'y':
    i = RACE_LLYRA;
    break;
  case 'w':
    i = RACE_WEREFORM;
    break;
  case 'd':
    i = RACE_DRAGON;
    break;
  default:
    break;
  }
  return i;
}

/* The menu for choosing a class in interpreter.c: */
const char *class_menu1 =
"\r\n"
"Select a class:\r\n\r\n"
" [F]ighter     [B]arbarian   [S]amurai    [T]hief      [N]inja\r\n"
" [C]leric      [D]ruid                    [M]age       [W]izard\r\n\r\n"
" [1] Mage-Cleric             [2] Mage-Thief            [3] Mage-Fighter\r\n"
" [4] Mage-Cleric-Fighter     [5] Mage-Thief-Figher     [6] Cleric-Fighter\r\n"
" [7] Fighter-Thief                                     [8] Monk\r\n"
"    Race/Code    :  F B S C D M W T N 1 2 3 4 5 6 7 8\r\n"
"     Human       :  X X X X X X X X X               X\r\n"
"     Elf         :  X     X X X   X     X X   X   X\r\n"
"     Halfelf     :  X     X X X   X   X X X X X X X\r\n"
"     Dwarf       :  X X   X       X             X X\r\n"
"     Halfling    :  X X       X   X               X\r\n"
"     Gnome       :  X     X   X X X     X X       X X\r\n"
"     Minotaur    :  X X     X X   X\r\n"
"     Pixie       :  X             X               X\r\n";

const char *class_menu2 = 
"     Hemnov      :  X   X X X X   X   X     X       X\r\n"
"     Llyran      :  X   X X   X   X X   X X       X\r\n";

const char *class_menu3 =
"     Hengyokai   :  X X     X X   X     X X   X     X\r\n"
"     DragonLord  :  X   X X     X X       X   X     X\r\n";

const char *class_menu4 =
"    Race/Code    :  F B S C D M W T N 1 2 3 4 5 6 7 8\r\n"
"        X == Available\r\n\r\n"
" Only Single Class Fighters/Barbarians/Samurai can be BountyHunters.\r\n"
" Only Single Class Thieves and Ninjas can register to be Assassins.\r\n"
" Avatars (A) are available at the last step upon the Golden Path.\r\n";

/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg, int race)
{
  int retval = CLASS_UNDEFINED;
  arg = LOWER(arg);

  switch (arg) {
  case 'b':
    if (race == RACE_HUMAN || race == RACE_DWARF ||
	  	race == RACE_HALFLING || race == RACE_MINOTAUR || race == RACE_WEREFORM)
	retval = CLASS_BARBARIAN;
    break;
  case 'c':
    if (!(race == RACE_HALFLING || race == RACE_MINOTAUR || race == RACE_PIXIE ||
		race == RACE_WEREFORM))
	retval = CLASS_CLERIC;
    break;
  case 'd':
    if (race == RACE_HUMAN || race == RACE_ELF || race == RACE_HALFELF ||
		 race == RACE_MINOTAUR || race == RACE_HEMNOV || race == RACE_WEREFORM)
	retval = CLASS_DRUID;
    break;
  case 'f':
    retval = CLASS_WARRIOR;
    break;
  case 'm':
    if (!(race == RACE_DWARF || race == RACE_PIXIE || race == RACE_DRAGON))
	retval = CLASS_MAGIC_USER;
    break;
  case 'n':
    if (race == RACE_HUMAN || race == RACE_LLYRA)
	retval = CLASS_NINJA;
    break;
  case 's':
    if (race == RACE_HUMAN || race == RACE_HEMNOV || race == RACE_WEREFORM ||
		race == RACE_DRAGON)
	retval = CLASS_SAMURAI;
    break;
  case 't':
    retval = CLASS_THIEF;
    break;
  case 'w':
    if (race == RACE_HUMAN || race == RACE_GNOME || race == RACE_DRAGON)
	retval = CLASS_WIZARD;
    break;
  case '1':
    if (race == RACE_HALFELF || race == RACE_HEMNOV) 
	retval = CLASS_MAG_CLE;
    break;
  case '2':
    if (race == RACE_ELF || race == RACE_HALFELF || race == RACE_GNOME ||
		race == RACE_LLYRA || race == RACE_WEREFORM)
	retval = CLASS_MAG_THI;
    break;
  case '3':
    if (race == RACE_ELF || race == RACE_HALFELF || race == RACE_GNOME ||
		race == RACE_LLYRA || race == RACE_WEREFORM || race == RACE_DRAGON)
	retval = CLASS_MAG_FIG;
    break;
  case '4':
    if (race == RACE_HALFELF || race == RACE_HEMNOV)
	retval = CLASS_MAG_CLE_FIG;
    break;
  case '5':
    if (race == RACE_ELF || race == RACE_HALFELF || race == RACE_WEREFORM || 
		race == RACE_DRAGON)
	retval = CLASS_MAG_THI_FIG;
    break;
  case '6':
    if (race == RACE_HALFELF || race == RACE_DWARF)
	retval = CLASS_CLE_FIG;
    break;
  case '7':
    if (!(race == RACE_HUMAN || race == RACE_MINOTAUR || race == RACE_HEMNOV ||
		race == RACE_WEREFORM || race == RACE_DRAGON))
	retval = CLASS_FIG_THI;
    break;
  case '8':
    if (race == RACE_HUMAN || race == RACE_GNOME || race == RACE_HEMNOV ||
		 race == RACE_WEREFORM || race == RACE_DRAGON)
	retval = CLASS_MONK;
    break;
  case 'a':
    if (!(race == RACE_WEREFORM || race == RACE_PIXIE))
	    retval = CLASS_AVATAR;
    break;
  default:
    retval = CLASS_UNDEFINED;
    break;
  }
  return retval;
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'm':
      return (1 << 0);
      break;
    case 'c':
      return (1 << 1);
      break;
    case 't':
      return (1 << 2);
      break;
    case 'f':
      return (1 << 3);
      break;
    case 'b':
      return (1 << 4);
      break;
    case 's':
      return (1 << 5);
      break;
    case 'd':
      return (1 << 6);
      break;
    case 'w':
      return (1 << 7);
      break;
    case '8':
      return (1 << 8);
      break;
    case 'a':
      return (1 << 9);
      break;
    case 'n':
      return (1 << 10);
      break;
    case 'y':
      return (1 << 14);
      break;
    case 'x':
      return (1 << 15);
    default:
      return 0;
      break;
  }
}

/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		2  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		3  min percent gain in skill per practice */
/* #define PRAC_TYPE		1  should it say 'spell' or 'skill'?	*/

int prac_params[2][NUM_CLASSES] = {
/* MAG    CLE    THE    FIG    BAR    SAM    DRU    WIZ    MON    AVA    NIN    Dual   Triple*/
  {85,    85,    85,    95,    80,    90,    85,    95,    80,    95,    90,    80,    75},	/* learned level */
  {SPELL, SPELL, SKILL, SKILL, SKILL, SKILL, SPELL, SPELL, SKILL, SKILL, SKILL, SKILL, SKILL}	/* prac name */
};
/*
  {43,    43,    33,    33,    20,    25,    75,    90,    70,    50}, */	/* max per prac */
/*
  {10,    10,     8,     8,     5,     6,    10,    15,    10,     8},	*/ /* min per pac */
/*
};
*/

/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 */
int guild_info[][7] = {

/* Capitol */
  {CLASS_CLERIC,	ANYRACE, ANYALIGN,		4263,	SCMD_NORTH, 1, LVL_IMPL},
  {CLASS_DRUID,		ANYRACE, ANYALIGN,		4260,	SCMD_SOUTH, 1, LVL_IMPL},
  {CLASS_MAGIC_USER + CLASS_AVATAR + CLASS_WIZARD, ANYRACE, ANYALIGN, 4175,
	SCMD_SOUTH, 1, LVL_IMPL},
  {CLASS_MONK + CLASS_AVATAR + CLASS_WARRIOR + CLASS_BARBARIAN + CLASS_SAMURAI,	
	ANYRACE, ANYALIGN, 4204, SCMD_WEST, 1, LVL_IMPL},
  {CLASS_THIEF + CLASS_NINJA,	ANYRACE, ANYALIGN, 4107, SCMD_SOUTH, 1, LVL_IMPL},
  {ANYCLASS, RACE_GNOME, ANYALIGN, 4058, SCMD_NORTH, 1, LVL_IMPL},
  {ANYCLASS, RACE_ELF, ANYALIGN, 4061, SCMD_NORTH, 1, LVL_IMPL},
  {ANYCLASS, RACE_HALFLING, ANYALIGN, 4064, SCMD_NORTH, 1, LVL_IMPL},
  {ANYCLASS, RACE_DWARF, ANYALIGN, 4093, SCMD_SOUTH, 1, LVL_IMPL},
  {ANYCLASS, RACE_ELF, ANYALIGN, 4047, SCMD_WEST, 1, LVL_IMPL},
  {ANYCLASS, RACE_HALFLING, ANYALIGN, 4047, SCMD_EAST, 1, LVL_IMPL},
  {ANYCLASS, ANYRACE, ANYALIGN, 4000, SCMD_NORTH, 1, 5},

/* this must go last -- add new guards above! */
{-1, -1, -1, -1, -1, 1 , LVL_IMPL}};




/* THAC0 for classes and levels.  (To Hit Armor Class 0) */

/* [class], [level] (all) */
const int thaco[NUM_CLASSES][LVL_IMPL + 1] = {

/* MAGE */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15,
  15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9},
  /* 20                  25                  30		    */

/* CLERIC */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 20, 18, 18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10,
  10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2, 1, 1, 1, 1},
  /* 20             25             30				    */

/* THIEF */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12,
  11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3},
  /* 20              25             30				    */

/* FIGHTER */
  /* 0                   5                  10              15	    */
  {100, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* 20             25             30				    */

/* BARBARIAN */
  /* 0                   5                  10              15	    */
  {100, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* 20             25             30				    */

/* SAMURAI */
  /* 0                   5                  10              15	    */
  {100, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* 20             25             30				    */

/* DRUID */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
  12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4},
  /*      20                 25             30				    */

/* WIZARD */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15,
  15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10},
  /*      20                  25                  30		    */

/* MONK */
  /* 0                   5                  10              15	    */
  {100, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* 20             25             30				    */

/* AVATAR */
  /* 0                   5                  10              15	    */
  {100, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* 20             25             30				    */

/* NINJA */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12,
  11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3},
  /* 20              25             30				    */

/* Dual-Class */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12,
  11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3},
  /* 20              25             30				    */

/* Triple-Class */
  /* 0                   5                  10                  15	    */
  {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12,
  11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3}
  /* 20              25             30				    */
};


/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
void roll_real_abils(struct char_data * ch)
{
  void send_to_char(char *messg, struct char_data * ch);

  int i, j, m = 0;
  ubyte rolls[4];
  ubyte table[6];
  char buf[256];
  
  while (m < MIN_ATTRIBUTE_AVE) {
  for (m = 0, i = 0; i < 6; i++) {
	  for (j = 0; j < 4; j++)
	    rolls[j] = number(1, 6);
	  table[i] = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
	      	MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));
	  m += table[i];
  	}
  m = (int) (m / 6);
  }
  ch->real_abils.str_add = 0;
  ch->real_abils.str = table[0];
  ch->real_abils.dex = table[1];
  ch->real_abils.con = table[2];
  ch->real_abils.intel = table[3];
  ch->real_abils.wis = table[4];
  ch->real_abils.cha = table[5];

  if (IS_ELF(ch)) {
	ch->real_abils.dex += 1;
	ch->real_abils.con -= 1;
  } else if (IS_HALFLING(ch)) {
	ch->real_abils.dex += 1;
	ch->real_abils.str -= 1;
  } else if (IS_DWARF(ch)) {
	ch->real_abils.con += 1;
	ch->real_abils.cha -= 1;
  } else if (IS_GNOME(ch)) {
	ch->real_abils.intel += 1;
	ch->real_abils.wis -= 1;
  } else if (IS_MINOTAUR(ch)) {
	ch->real_abils.str += 2;
	ch->real_abils.con += 2;
	ch->real_abils.wis -= 2;	
	ch->real_abils.cha -= 2;
  } else if (IS_PIXIE(ch)) {
	ch->real_abils.intel += 1;
	ch->real_abils.con -= 1;
  } else if (IS_LLYRA(ch)) {
	ch->real_abils.str = 18 + dice(1, 4);
	ch->real_abils.dex = 18 + dice(1, 6);
	ch->real_abils.wis = 19 + dice(1, 4);
	ch->real_abils.con = 17 + dice(1, 3);
	ch->real_abils.cha = 20 + dice(1, 5);
	ch->real_abils.intel = 20 + dice(1, 4);
  } else if (IS_HEMNOV(ch)) {
	ch->real_abils.str = 18 + dice(1, 6);
	ch->real_abils.dex = 19 + dice(1, 4);
	ch->real_abils.wis = 18 + dice(1, 4);
	ch->real_abils.con = 20 + dice(1, 5);
	ch->real_abils.cha = 19 + dice(1, 6);
	ch->real_abils.intel = 17 + dice(1, 5);
  } else if (IS_WEREFORM(ch)) {
	ch->real_abils.str = 18 + dice(1, 4);
	ch->real_abils.dex = 19 + dice(1, 4);
	ch->real_abils.wis = 18 + dice(1, 3);
	ch->real_abils.con = 20 + dice(1, 5);
	ch->real_abils.cha = 20 + dice(1, 5);
	ch->real_abils.intel = 17 + dice(1, 4);
  } else if (IS_DRAGON(ch)) {
	ch->real_abils.str = 17 + dice(1, 8);
	ch->real_abils.dex = 18 + dice(1, 7);
	ch->real_abils.wis = 17 + dice(1, 8);
	ch->real_abils.con = 19 + dice(1, 6);
	ch->real_abils.cha = 18 + dice(1, 7);
	ch->real_abils.intel = 20 + dice(1, 5);
  }
  m = ch->real_abils.str + ch->real_abils.dex + ch->real_abils.con +
	ch->real_abils.intel + ch->real_abils.wis + ch->real_abils.cha;

  ch->aff_abils = ch->real_abils;
  sprintf(buf, "Str: %d/%d  Dex: %d  Con: %d  Int: %d  Wis: %d  Cha: %d  Ave: %d\r\n",
	ch->real_abils.str, ch->real_abils.str_add, ch->real_abils.dex, ch->real_abils.con,
	ch->real_abils.intel, ch->real_abils.wis, ch->real_abils.cha,
	(int) (m / 6) );
  send_to_char(buf, ch);
}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);
  struct obj_data *obj;

  int gold = 0;
  int divis = GET_NUM_OF_CLASS(ch);
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;
  GET_ALIGNMENT(ch) = 0;

  set_title(ch, NULL);
  ch->points.max_hit = 10;
  ch->points.max_mana = 50;  

  while (ch->carrying) {	/* clear out inventory, just in case */
	obj = ch->carrying;
	obj_from_char(obj);
	extract_obj(obj);
  }
	
  if (IS_AVATAR(ch)) {
	if (ch->real_abils.str == 18) {
		 ch->real_abils.str_add = number(1, 100);
		 ch->aff_abils.str_add = ch->real_abils.str_add;
	}
    gold += 100 * dice(3, 6) / divis;
  }
  if (IS_MONK(ch)) {
	if (ch->real_abils.str == 18) {
		 ch->real_abils.str_add = number(1, 100);
		 ch->aff_abils.str_add = ch->real_abils.str_add;
	}
    gold += 10 * dice(1, 6) / divis;
  }
  if (IS_NINJA(ch))
    gold += 10 * dice(2, 6) / divis;
  if (IS_MAGIC_USER(ch))
    gold += 10 * dice(3, 6) / divis;
  if (IS_CLERIC(ch))
    gold += 10 * dice(3, 6) / divis;

  if (IS_THIEF(ch)) {
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    gold += (int) (10 * dice(2, 6) / divis);
  } 

  if (IS_WARRIOR(ch)) {
	if (ch->real_abils.str == 18) {
		 ch->real_abils.str_add = number(1, 100);
		 ch->aff_abils.str_add = ch->real_abils.str_add;
	}
    gold += (int) (10 * dice(4, 6) / divis);
  }

  if (IS_BARBARIAN(ch)) {
	if (ch->real_abils.str == 18) {
		 ch->real_abils.str_add = number(1, 100);
		 ch->aff_abils.str_add = ch->real_abils.str_add;
	}
    SET_SKILL(ch, SKILL_KICK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    gold += (int) (10 * dice(2, 4) / divis);
  }

  if (IS_SAMURAI(ch)) {
	if (ch->real_abils.str == 18) {
		 ch->real_abils.str_add = number(1, 100);
		 ch->aff_abils.str_add = ch->real_abils.str_add;
	}
    SET_SKILL(ch, SPELL_ARMOR, 50);
    SET_SKILL(ch, SPELL_CURE_LIGHT, 25);
    gold += (int) (100 * dice(5, 6) / divis);
  }

  if (IS_DRUID(ch)) {
    SET_SKILL(ch, SPELL_WATERWALK, 15);
    gold += (int) (10 * dice(2, 6) / divis);
  }

  if (IS_WIZARD(ch)) {
    SET_SKILL(ch, SPELL_MAGIC_MISSILE, 20);
    gold += (int) (10 * dice(2, 4) / divis);
  }

  GET_GOLD(ch) = 100 * gold;
  GET_BANK_GOLD(ch) = 0;
  GET_PRACTICES(ch) = 10;
  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 70;
  GET_COND(ch, FULL) = 70;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  if (IS_ELF(ch) || IS_HALFLING(ch))
	SET_BIT(PRF2_FLAGS(ch), PRF2_WAR_YLLANTRA);
  if (IS_DWARF(ch))
	SET_BIT(PRF2_FLAGS(ch), PRF2_WAR_DRUHARI);

  SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPGOLD | PRF_DISPXP);
  SET_BIT(PRF_FLAGS(ch), PRF_SUMMONABLE | PRF_AUTOEXIT | PRF_NOWIZ | PRF_NOCLAN | PRF_NOTEAM);
  SET_BIT(PRF2_FLAGS(ch), PRF2_NOQUIT);
  SET_BIT(PLR_FLAGS(ch), PLR_NOTITLE);
}

/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(struct char_data * ch)
{
  int add_hp = 0, add_mana = 0, add_move = 0, i;
  int divis = GET_NUM_OF_CLASS(ch);

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  add_hp = con_app[GET_CON(ch)].hitp;

  if (IS_MAGIC_USER(ch) || IS_AVATAR(ch)) {
    add_hp += (int) (number(3, 8) / divis);
    add_mana += (int) (	number(3, 6) + 	number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch))) / divis);
    add_move += (int) (5 / divis);
  }

  if (IS_CLERIC(ch)) {
    add_hp += (number(5, 10) / divis);
    add_mana += (number(4, 8) + number(GET_LEVEL(ch), (int) (2.0 * GET_LEVEL(ch))) / divis);
    add_move += (8 / divis);
  }

  if (IS_THIEF(ch) || IS_NINJA(ch)) {
    add_hp += (number(7, 13) / divis);
    add_mana += (number(0, 2) / divis);
    add_move += (number(5, 10) / divis);
  }

  if (IS_WARRIOR(ch) || IS_MONK(ch) || IS_AVATAR(ch)) {
    add_hp += (number(10, 15)  / divis);
    add_mana += (number(0, 3) / divis);
    add_move += (10 / divis);
  }

  if (IS_BARBARIAN(ch)) {
    add_hp += (number(15, 25) / divis);
    add_mana += (number(1, 3) / divis);
    add_move += (15 / divis);
  }

  if (IS_SAMURAI(ch)) {
    add_hp += (10 + number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch))) / divis);
    add_mana += (number(0, 2) / divis);
    add_move += (number(10, 15) / divis);
  }

  if (IS_DRUID(ch)) {
    add_hp += (number(5, 10) / divis);
    add_mana += (number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch))) / divis);
    add_move += (6 / divis);
  }

  if (IS_WIZARD(ch)) {
    add_hp += (number(3, 6) / divis);
    add_mana += (number(3,8) + number((int) (1.5 * GET_LEVEL(ch)), (int) (2.2 * GET_LEVEL(ch))) / divis);
    add_move += (number(0, 2) / divis);
  }

  ch->points.max_hit += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);
  ch->points.max_mana += MAX(0, add_mana);

  if (IS_AVATAR(ch) || IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_WIZARD(ch) || IS_DRUID(ch))
    GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
  else
    GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }

  save_char(ch, NOWHERE);

  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(struct char_data *ch, struct obj_data *obj) {
  long temp_class = ~0;

  if (IS_MOB(ch))
	return 0;
  if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
	  REMOVE_BIT(temp_class, CLASS_DUAL | CLASS_TRIPLE);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER))
		REMOVE_BIT(temp_class, CLASS_WIZARD | CLASS_MAGIC_USER);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC))
		REMOVE_BIT(temp_class, CLASS_CLERIC | CLASS_DRUID);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF))
		REMOVE_BIT(temp_class, CLASS_THIEF | CLASS_NINJA);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR))
		REMOVE_BIT(temp_class, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_SAMURAI | CLASS_MONK | CLASS_AVATAR);
     if IS_SET(GET_CLASS(ch), temp_class)
	return 0;
     else
	return 1;
  } else {
	temp_class = 0;
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER))
		SET_BIT(temp_class, CLASS_WIZARD | CLASS_MAGIC_USER);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC))
		SET_BIT(temp_class, CLASS_CLERIC | CLASS_DRUID);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF))
		SET_BIT(temp_class, CLASS_THIEF | CLASS_NINJA);
	  if (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR))
		SET_BIT(temp_class, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_SAMURAI | CLASS_MONK | CLASS_AVATAR);
     if (GET_CLASS(ch) & temp_class)
	return 1;
     else
	return 0; 
  }
}

int invalid_race(struct char_data *ch, struct obj_data *obj) {
   if ((IS_OBJ_STAT(obj, ITEM_ANTI_HUMAN) && (IS_HUMAN(ch) || IS_HALFELF(ch))) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_ELF) && (IS_ELF(ch) || IS_HALFELF(ch))) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_DWARF) && IS_DWARF(ch)) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_GNOME) && IS_GNOME(ch)) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_MINOTAUR) && IS_MINOTAUR(ch)) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_PIXIE) && IS_PIXIE(ch)) ||
       (IS_OBJ_STAT(obj, ITEM_ANTI_HALFLING) && IS_HALFLING(ch)))
	return 1;
   else
	return 0;
}

/* Names of class/levels and exp required for each level */

const struct title_type titles[NUM_CLASSES][LVL_IMPL + 1] = {
  {{"the Nobody Mage", 			0},
  {"the Apprentice of Magic", 		1},
  {"the Spell Student",  		2500},
  {"the Scholar of Magic", 		5000},
  {"the Delver in Spells", 		10000},
  {"the Medium of Magic", 		20000},
  {"the Scribe of Magic", 		40000},
  {"the Seer", 				60000},
  {"the Sage", 				90000},
  {"the Illusionist",			135000},
  {"the Abjurer", 			250000},
  {"the Invoker", 			375000},
  {"the Enchanter", 			750000},
  {"the Conjurer", 			1125000},
  {"the Magician", 			1500000},
  {"the Creator", 			1875000},
  {"the Savant", 			2250000},
  {"the Magus", 			2625000},
  {"the SpellCrafter", 			3000000},
  {"the Warlock", 			3375000},
  {"the Sorcerer", 			3750000},
  {"the Necromancer", 			4000000},
  {"the Thaumaturge", 			4300000},
  {"the Student of the Occult", 	4600000},
  {"the Disciple of the Uncanny", 	4900000},
  {"the Minor Elemental", 		5200000},
  {"the Greater Elemental", 		5500000},
  {"the Crafter of Magics", 		5950000},
  {"the Shaman", 			6400000},
  {"the Keeper of Talismans", 		6850000},
  {"the Archmage", 			7400000},
  {"the Immortal Warlock", 		8000000},
  {"the Avatar of Magic", 		9000000},
  {"the God of Magic", 			9500000},
  {"the Implementor", 			10000000}
  },
  {{"the Nobody Cleric", 		0},
  {"the Believer", 			1},
  {"the Attendant",		 	1500},
  {"the Acolyte", 			3000},
  {"the Novice", 			6000},
  {"the Missionary", 			13000},
  {"the Adept", 			27500},
  {"the Deacon", 			55000},
  {"the Vicar", 			110000},
  {"the Priest", 			225000},
  {"the Minister", 			450000},
  {"the Canon", 			675000},
  {"the Levite", 			900000},
  {"the Curate",			1125000},
  {"the Devote",			1350000},
  {"the Healer", 			1575000},
  {"the Chaplain", 			1800000},
  {"the Expositor", 			2100000},
  {"the Bishop",			2400000},
  {"the Arch Bishop", 			2700000},
  {"the Holy Avatar", 			3000000},
  {"the Holy Avatar (21)",		3250000},
  {"the Holy Avatar (22)",		3500000},
  {"the Holy Avatar (23)",		3800000},
  {"the Holy Avatar (24)",		4100000},
  {"the Holy Avatar (25)", 		4400000},
  {"the Holy Avatar (26)",		4800000},
  {"the Holy Avatar (27)",		5200000},
  {"the Holy Avatar (28)",		5600000},
  {"the Holy Avatar (29)",		6000000},
  {"the Holy Avatar (30)", 		6400000},
  {"the Immortal Cleric",		7000000},
  {"the Inquisitor", 			9000000},
  {"the Deity",				9500000},
  {"the Implementor",			10000000}
  },
  {{"the Nobody Thief",			0},
  {"the Pilferer",			1},
  {"the Footpad",			1250},
  {"the Filcher",			2500},
  {"the Pick-Pocket",			5000},
  {"the Sneak",				10000},
  {"the Pincher",			20000},
  {"the Cut-Purse",			30000},
  {"the Snatcher",			70000},
  {"the Sharper",			110000},
  {"the Rogue",				160000},
  {"the Robber",			220000},
  {"the Magsman",			440000},
  {"the Highwayman",			660000},
  {"the Burglar",			880000},
  {"the Thief",				1100000},
  {"the Knifer",			1500000},
  {"the Quick-Blade",			2000000},
  {"the Killer", 			2500000},
  {"the Brigand", 			3000000},
  {"the Cut-Throat",			3500000},
  {"the Cut-Throat (21)", 		3650000},
  {"the Cut-Throat (22)", 		3800000},
  {"the Cut-Throat (23)", 		4100000},
  {"the Cut-Throat (24)", 		4400000},
  {"the Cut-Throat (25)", 		4700000},
  {"the Cut-Throat (26)", 		5100000},
  {"the Cut-Throat (27)", 		5500000},
  {"the Cut-Throat (28)", 		5900000},
  {"the Cut-Throat (29)", 		6300000},
  {"the Cut-Throat (30)", 		6650000},
  {"the Immortal Thief",		7000000},
  {"the Master Craftsman",		9000000},
  {"the Deity of Thieves", 		9500000},
  {"the Implementor",			10000000}
  }, 
  {{"the Nobody Warrior", 		0},
  {"the Swordpupil", 			1},
  {"the Recruit", 			2000},
  {"the Sentry", 			4000},
  {"the Fighter", 			8000},
  {"the Soldier", 			16000},
  {"the Warrior", 			32000},
  {"the Veteran", 			64000},
  {"the Swordsman", 			125000},
  {"the Fencer", 			250000},
  {"the Combatant", 			500000},
  {"the Hero", 				750000},
  {"the Myrmidon", 			1000000},
  {"the Swashbuckler", 			1250000},
  {"the Mercenary",			1500000},
  {"the Swordmaster", 			1850000},
  {"the Lieutenant", 			2200000},
  {"the Champion", 			2550000},
  {"the Dragoon",			2900000},
  {"the Cavalier",			3250000},
  {"the Knight", 			3600000},
  {"the Knight (21)",			3900000},
  {"the Knight (22)", 			4200000},
  {"the Knight (23)", 			4500000},
  {"the Knight (24)", 			4800000},
  {"the Knight (25)", 			5150000},
  {"the Knight (26)", 			5500000},
  {"the Knight (27)", 			5950000},
  {"the Knight (28)", 			6400000},
  {"the Knight (29)", 			6850000},
  {"the Knight (30)", 			7400000},
  {"the Immortal Fighter",		8000000},
  {"the Extirpator", 			9000000},
  {"the Deity of War",			9500000},
  {"the Implementor",			10000000}
  },
  {{"the Nobody Barbarian", 		0},
  {"the Grasslands Barbarian", 		1},
  {"the Field Barbarian",		2200},
  {"the Plains Barbarian",		4400},
  {"the Plateau Barbarian",		8800},
  {"the Desert Barbarian",		18000},
  {"the Valley Barbarian",		36000},
  {"the Hill Barbarian",		72000},
  {"the Mountain Barbarian", 		150000},
  {"the Barbarian",			300000},
  {"the Barbarian", 			600000},
  {"the Barbarian", 			900000},
  {"the Keshik", 			1200000},
  {"the Keshik", 			1500000},
  {"the Keshik", 			1800000},
  {"the saKeshik",			2100000},
  {"the saKeshik",			2500000},
  {"the saKeshik",			2900000},
  {"the ilKeshik",			3300000},
  {"the ilKeshik",			3700000},
  {"the ilKeshik",			4100000},
  {"the Kahn of the Grasslands", 	4500000},
  {"the Kahn of the Fields",		4900000},
  {"the Kahn of the Plains",		5300000},
  {"the Kahn of the Plateaus",		5700000},
  {"the Kahn of the Deserts",		6100000},
  {"the Kahn of the Valleys", 		6500000},
  {"the Kahn of the Hills", 		6900000},
  {"the Kahn of the Mountains", 	7300000},
  {"the ilKahn", 			7700000},
  {"the ilKahn", 			8200000},
  {"LoreMaster", 			8600000},
  {"RitesKeeper", 			9000000},
  {"TrialMaster", 			9500000},
  {"the WorldMaker",			10000000}
  },
  {{"the Ronin Samurai", 		0},
  {"the Swordpupil",			1},
  {"the Recruit", 			4000},
  {"the Swordbearer",			8000},
  {"the Swordwielder",			16000},
  {"the Footsoldier",			32000},
  {"the Warrior", 			64000},
  {"the Veteran", 			125000},
  {"the Swordsman", 			250000},
  {"the Guardian", 			500000},
  {"the Swordmaster",			750000},
  {"the Swordmaster",			1000000},
  {"the Swordmaster",			1250000},
  {"the Elite Swordmaster",		1500000},
  {"the Elite Swordmaster",		1800000},
  {"the Elite Swordmaster",		2100000},
  {"the Samurai (Rank 9)",		2500000},
  {"the Samurai (Rank 8)",		2900000},
  {"the Samurai (Rank 7)",		3300000},
  {"the Samurai (Rank 6)", 		3700000},
  {"the Samurai (Rank 5)",		4100000},
  {"the Samurai (Rank 4)", 		4500000},
  {"the Samurai (Rank 3)", 		4900000},
  {"the Samurai (Rank 2)", 		5300000},
  {"the Samurai (Rank 1)",		5700000},
  {"the Samurai Lord",			6100000},
  {"the Samurai Lord",			6500000},
  {"the Samurai Lord",			6900000},
  {"the Shogun",			7300000},
  {"the Shogun", 			7700000},
  {"the Shogun",			8200000},
  {"the Immortal Samurai", 		8600000},
  {"the Lord of Death",			9000000},
  {"the Wrath of Kahn",			9500000},
  {"the Implementor",			10000000}
  },
  {{"the Nobody Druid",			0},
  {"the fledgling Druid",		1},
  {"the Believer",			3000},
  {"the Acolyte",			6000},
  {"the elder Acolyte",			13000},
  {"the Novice", 			27500},
  {"the Adept",				55000},
  {"the Forester", 			110000},
  {"the Deacon",			225000},
  {"the Vicar",				450000},
  {"the Druid",				700000},
  {"the Grand Adept",			950000},
  {"the Grand Forester",		1200000},
  {"the Grand Deacon",			1450000},
  {"the Grand Vicar",			1700000},
  {"a Druid of the Leaf",		1950000},
  {"a Druid of the Branch",		2200000},
  {"a Druid of the Forest",		2500000},
  {"a Druid of the Stream",		2800000},
  {"a Druid of the River", 		3100000},
  {"a Druid of the Lake", 		3400000},
  {"a Druid of the Cave", 		3700000},
  {"a Druid of the Hill", 		4000000},
  {"a Druid of the Mountain", 		4300000},
  {"a Druid of the Land", 		4600000},
  {"a Druid of the Wind", 		5000000},
  {"a Druid of the Sky", 		5400000},
  {"a Druid of the Higher Skies", 	5800000},
  {"the Grand Druid",			6300000},
  {"the Elder Druid",			6800000},
  {"the Heirophant", 			7400000},
  {"the Druid of Undying",		8000000},
  {"the Druid of Light", 		9000000},
  {"the Druid of Heavens", 		9500000},
  {"the Grand Heirophant",		10000000}
  },
  {{"the Nobody Wizard", 		0},
  {"the Apprentist",			1},
  {"the Spellbookkeeper", 		5000},
  {"the Wizling", 			10000},
  {"the Spellslinger",			20000},
  {"the Scribe", 			40000},
  {"the Seer",				60000},
  {"the minor Sage", 			90000},
  {"the major Sage",			135000},
  {"the Fire Starter",			250000},
  {"the Lightning Breather",		375000},
  {"the Elementalist", 			750000},
  {"the Sight Stealer",			1125000},
  {"the Summoner",			1500000},
  {"the Relocator",			1875000},
  {"the Fire Shaper",			2250000},
  {"the Flametongue",			2625000},
  {"the Fire Breather", 		3000000},
  {"the Transmutator",			3375000},
  {"the Apprentist Wizard",		3750000},
  {"the Wizard of the Outer Circuit", 	4000000},
  {"the Wizard of the Third Circuit", 	4300000},
  {"the Wizard of the Second Circuit", 	4600000},
  {"the Wizard of the Inner Circuit",	4900000},
  {"the Wizard of Earth",		5200000},
  {"the Wizard of Lightning",		5500000},
  {"the Wizard of Fire",		5950000},
  {"the Wizard of Frost",		6400000},
  {"the Wizard of Air",			6850000},
  {"the Wizard of Water",		7200000},
  {"the Grand Wizard",			7600000},
  {"the Immortal Wizard",		8000000},
  {"the Grand Avatar of Magic",		9000000},
  {"the God of Magic", 			9500000},
  {"the Implementor", 			10000000}
  },
  {{"the Beginning Monk", 		0},
  {"the Grasslands Barbarian", 		1},
  {"the Field Barbarian",		2200},
  {"the Plains Barbarian",		4400},
  {"the Plateau Barbarian",		8800},
  {"the Desert Barbarian",		18000},
  {"the Valley Barbarian",		36000},
  {"the Hill Barbarian",		72000},
  {"the Mountain Barbarian", 		150000},
  {"the Barbarian",			300000},
  {"the Barbarian", 			600000},
  {"the Barbarian", 			900000},
  {"the Keshik", 			1200000},
  {"the Keshik", 			1500000},
  {"the Keshik", 			1800000},
  {"the saKeshik",			2100000},
  {"the saKeshik",			2500000},
  {"the saKeshik",			2900000},
  {"the ilKeshik",			3300000},
  {"the ilKeshik",			3700000},
  {"the ilKeshik",			4100000},
  {"the Kahn of the Grasslands", 	4500000},
  {"the Kahn of the Fields",		4900000},
  {"the Kahn of the Plains",		5300000},
  {"the Kahn of the Plateaus",		5700000},
  {"the Kahn of the Deserts",		6100000},
  {"the Kahn of the Valleys", 		6500000},
  {"the Kahn of the Hills", 		6900000},
  {"the Kahn of the Mountains", 	7300000},
  {"the ilKahn", 			7700000},
  {"the ilKahn", 			8200000},
  {"LoreMaster", 			8600000},
  {"RitesKeeper", 			9000000},
  {"TrialMaster", 			9500000},
  {"the WorldMaker",			10000000}
  },
  {{"the Starting Avatar", 		0},
  {"the Apprentist",			1},
  {"the Spellbookkeeper", 		5000},
  {"the Wizling", 			10000},
  {"the Spellslinger",			20000},
  {"the Scribe", 			40000},
  {"the Seer",				60000},
  {"the minor Sage", 			90000},
  {"the major Sage",			135000},
  {"the Fire Starter",			250000},
  {"the Lightning Breather",		375000},
  {"the Elementalist", 			750000},
  {"the Sight Stealer",			1125000},
  {"the Summoner",			1500000},
  {"the Relocator",			1875000},
  {"the Fire Shaper",			2250000},
  {"the Flametongue",			2625000},
  {"the Fire Breather", 		3000000},
  {"the Transmutator",			3375000},
  {"the Apprentist Wizard",		3750000},
  {"the Wizard of the Outer Circuit", 	4000000},
  {"the Wizard of the Third Circuit", 	4300000},
  {"the Wizard of the Second Circuit", 	4600000},
  {"the Wizard of the Inner Circuit",	4900000},
  {"the Wizard of Earth",		5200000},
  {"the Wizard of Lightning",		5500000},
  {"the Wizard of Fire",		5950000},
  {"the Wizard of Frost",		6400000},
  {"the Wizard of Air",			6850000},
  {"the Wizard of Water",		7200000},
  {"the Grand Wizard",			7600000},
  {"the Immortal Wizard",		8000000},
  {"the Grand Avatar of Magic",		9000000},
  {"the God of Magic", 			9500000},
  {"the Implementor", 			10000000}
  },
  {{"the Starting Ninja", 		0},
  {"the Swordpupil",			1},
  {"the Recruit", 			4000},
  {"the Swordbearer",			8000},
  {"the Swordwielder",			16000},
  {"the Footsoldier",			32000},
  {"the Warrior", 			64000},
  {"the Veteran", 			125000},
  {"the Swordsman", 			250000},
  {"the Guardian", 			500000},
  {"the Swordmaster",			750000},
  {"the Swordmaster",			1000000},
  {"the Swordmaster",			1250000},
  {"the Elite Swordmaster",		1500000},
  {"the Elite Swordmaster",		1800000},
  {"the Elite Swordmaster",		2100000},
  {"the Samurai (Rank 9)",		2500000},
  {"the Samurai (Rank 8)",		2900000},
  {"the Samurai (Rank 7)",		3300000},
  {"the Samurai (Rank 6)", 		3700000},
  {"the Samurai (Rank 5)",		4100000},
  {"the Samurai (Rank 4)", 		4500000},
  {"the Samurai (Rank 3)", 		4900000},
  {"the Samurai (Rank 2)", 		5300000},
  {"the Samurai (Rank 1)",		5700000},
  {"the Samurai Lord",			6100000},
  {"the Samurai Lord",			6500000},
  {"the Samurai Lord",			6900000},
  {"the Shogun",			7300000},
  {"the Shogun", 			7700000},
  {"the Shogun",			8200000},
  {"the Immortal Samurai", 		8600000},
  {"the Lord of Death",			9000000},
  {"the Wrath of Kahn",			9500000},
  {"the Implementor",			10000000}
  },
  {{"the Nobody Double Class", 		0},
  {"the Swordpupil", 			1},
  {"the Recruit", 			2000},
  {"the Sentry", 			4000},
  {"the Fighter", 			8000},
  {"the Soldier", 			16000},
  {"the Warrior", 			32000},
  {"the Veteran", 			64000},
  {"the Swordsman", 			125000},
  {"the Fencer", 			250000},
  {"the Combatant", 			500000},
  {"the Hero", 				750000},
  {"the Myrmidon", 			1000000},
  {"the Swashbuckler", 			1250000},
  {"the Mercenary",			1500000},
  {"the Swordmaster", 			1850000},
  {"the Lieutenant", 			2200000},
  {"the Champion", 			2550000},
  {"the Dragoon",			2900000},
  {"the Cavalier",			3250000},
  {"the Knight", 			3600000},
  {"the Knight (21)",			3900000},
  {"the Knight (22)", 			4200000},
  {"the Knight (23)", 			4500000},
  {"the Knight (24)", 			4800000},
  {"the Knight (25)", 			5150000},
  {"the Knight (26)", 			5500000},
  {"the Knight (27)", 			5950000},
  {"the Knight (28)", 			6400000},
  {"the Knight (29)", 			6850000},
  {"the Knight (30)", 			7400000},
  {"the Immortal Fighter",		8000000},
  {"the Extirpator", 			9000000},
  {"the Deity of War",			9500000},
  {"the Implementor",			10000000}
  },
  {{"the Nobody Triple Class", 		0},
  {"the Swordpupil", 			1},
  {"the Recruit", 			2000},
  {"the Sentry", 			4000},
  {"the Fighter", 			8000},
  {"the Soldier", 			16000},
  {"the Warrior", 			32000},
  {"the Veteran", 			64000},
  {"the Swordsman", 			125000},
  {"the Fencer", 			250000},
  {"the Combatant", 			500000},
  {"the Hero", 				750000},
  {"the Myrmidon", 			1000000},
  {"the Swashbuckler", 			1250000},
  {"the Mercenary",			1500000},
  {"the Swordmaster", 			1850000},
  {"the Lieutenant", 			2200000},
  {"the Champion", 			2550000},
  {"the Dragoon",			2900000},
  {"the Cavalier",			3250000},
  {"the Knight", 			3600000},
  {"the Knight (21)",			3900000},
  {"the Knight (22)", 			4200000},
  {"the Knight (23)", 			4500000},
  {"the Knight (24)", 			4800000},
  {"the Knight (25)", 			5150000},
  {"the Knight (26)", 			5500000},
  {"the Knight (27)", 			5950000},
  {"the Knight (28)", 			6400000},
  {"the Knight (29)", 			6850000},
  {"the Knight (30)", 			7400000},
  {"the Immortal Fighter",		8000000},
  {"the Extirpator", 			9000000},
  {"the Deity of War",			9500000},
  {"the Implementor",			10000000}
  }
};

