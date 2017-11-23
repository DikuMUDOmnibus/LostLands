/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "structs.h"

const char circlemud_version[] = {
"CircleMUD, version 3.00 beta patchlevel 8: STROM'S LOST LANDS\r\n"};


/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */

/* char channel_bits[] will be read in from a file */
/* char clan_names[] will also be read in from a file */

const char *channel_bits[] = {
  "God Channel",
  "Radio Free LostLands",
  "Arena battles",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "11",
  "12",
  "13",
  "14",
  "15",
  "16",
  "17",
  "18",
  "19",
  "20",
  "21",
  "22",
  "23",
  "24",
  "25",
  "26",
  "27",
  "28",
  "29",
  "30",
  "31",
  "32",
  "\n"
};

/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const char *dirs2[] = 
{
  "the north",
  "the east",
  "the south",
  "the west",
  "above",
  "below",
  "\n"
};

const char *teleport_bits[] = {
  "Force Look",
  "Timed",
  "Random",
  "Spin(NOT AVAIL)",
  "Has Obj",
  "No Obj",
  "No MSG",
  "\n"
};

/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE (R)",
  "HCRSH (R)",
  "ATRIUM(R)",
  "OLC   (R)",
  "* (R)",		/* BFS MARK */
  "MURKY",
  "COURT",
  "BROADCAST",
  "RECEIVE",
  "\n"
};


/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "\n"
};

/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "Quicksand",
  "Lava",
  "\n"
};


/* SEX_x */
const char *genders[] =
{
  "Neutral",
  "Male",
  "Female"
};

const char *gend_he[] =
{
  "it",
  "he",
  "she"
};

/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};


/* PLR_x */
const char *player_bits[] = {
  "KILLER",
  "THIEF",
  "FROZEN",
  "DONTSET",
  "WRITING",
  "MAILING",
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",
  "LOADRM",
  "!WIZL",
  "!DEL",
  "INVST",
  "CRYO",
  "EDITING",
  "NOAUTOTITLE",
  "JUSTREIN",
  "\n"
};

const char *player_bits2[] = {
  "CLAN BIT1",
  "CLAN BIT2",
  "CLAN BIT3",
  "CLAN LEAD",
  "\n"
};

const char *player_bits3[] = {
  "\n"
};

/* MOB_x */
const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",
  "MEMORY",
  "HELPER",
  "!CHARM",
  "!SUMMN",
  "!SLEEP",
  "!BASH",
  "!BLIND",
  "!Corpse",
  "PET",
  "ETHEREAL",
  "FAST REGEN",
  "HUNT",
  "\n"
};


const char *action_bits2[] = {
  "NO BURN",
  "MORE BURN",
  "NO FREEZE",
  "MORE FREEZE",
  "NO ACID",
  "MORE ACID",
  "CAN BURN",
  "CAN FREEZE",
  "CAN ACID",
  "GAZE PETRIF",
  "\n"
};

const char *action_bits3[] = {
  "CAN TALK",
  "CAN'T FLEE",
  "\n"
};

/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",
  "COMPACT",
  "DEAF",
  "!TELL",
  "D_HP",
  "D_MANA",
  "D_MOVE",
  "A_EXIT",
  "!HASS",
  "QUEST",
  "SUMN",
  "!REP",
  "LIGHT",
  "C1",
  "C2",
  "!WIZ",
  "L1",
  "L2",
  "!AUC",
  "!GOS",
  "!GTZ",
  "RMFLG",
  "!CLAN",
  "!CHAT",
  "!WAR",
  "!ARENA",
  "A_DIR",
  "A_SAC",
  "D_GOLD",
  "D_XP",
  "D_DAM",
  "\n"
};

const char *preference_bits2[] = {
  "REINCARN1",
  "REINCARN2",
  "REINCARN3",
  "WAR_DRUHARI",
  "WAR_YLLANTRA",
  "RETIRED",
  "ARENA_RED",
  "ARENA_BLUE",
  "AFK",
  "BOUNTY",
  "ASSASSIN",
  "NOQUIT",
  "\n"
};

/* AFF_x */
const char *affected_bits[] =
{
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SEN-LIFE",
  "H2O-WALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "UNUSED",
  "SHIELD",
  "SNEAK",
  "HIDE",
  "DEATHDANCE",
  "CHARM",
  "FLYING",
  "H2O-BREATH",
  "PROT_FIRE",
  "+1",
  "+2",
  "+3",
  "+4",
  "+5",
  "+Silver",
  "\n"
};

/* AFF2_x */
const char *affected_bits2[] =
{
  "MIRROR",
  "STONESKIN",
  "FARSEE",
  "ENH_HEAL",
  "ENH_MANA",
  "ENH_MOVE",
  "HELD",
  "CRITED",
  "BURNING",
  "FREEZING",
  "ACIDED",
  "PROT COLD",
  "BLINK",
  "HASTE",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "\n"
};

/* AFF_x */
const char *affected_bits3[] =
{
  "PASSDOOR",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "\n"
};


/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1",
  "Self-Delete 2",
  "Get Attrib",
  "Get Race",
  "I-EDITING",
  "R-EDITING",
  "Z-EDITING"
  "Reincarn 1",
  "Reincarn 2",
  "\n"
};


/* WEAR_x - for eq list */
const char *where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as a shield>   ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<dual-wielded>       ",
  "<held>               ",
  "<worn on back>       ",
  "<worn on face>       "
};


/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as a shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Dual-Wielded",
  "Held",
  "Worn on the back",
  "Worn on face",
  "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "FLIGHT",
  "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "BACK",
  "FACE",
  "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVISIBLE",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEUTRAL",
  "!MAGE",
  "!CLERIC",
  "!THIEF",
  "!WARRIOR",
  "!SELL",
  "!HUMAN",
  "!ELF",
  "!DWARF",
  "!GNOME",
  "!HALFLING",
  "!MINOTAUR",
  "!PIXIE",
  "DAMS+1",
  "DAMS+2",
  "DAMS+3",
  "DAMS+4",
  "DAMS+5",
  "DAMS+SIL",
  "!BREAK",
  "\n"
};

const char *extra_bits2[] = {
  "RETURNING",
  "!REMOVE",
  "ENGRAVED",
  "2-Handed",
  "AUTOENGRAVE",
  "\n"
};

/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "\n"
};


/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};


/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "water",
  "\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};


/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear"
};


/* level of fullness for drink containers */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[41] = {
  {-5, -4, 0, 0},
  {-5, -4, 30, 10},
  {-3, -2, 30, 20},
  {-3, -1, 100, 30},
  {-2, -1, 250, 40},
  {-2, -1, 550, 50},
  {-1, 0, 800, 60},
  {-1, 0, 900, 70},
  {0, 0, 1000, 80},
  {0, 0, 1000, 90},
  {0, 0, 1150, 100},	/* 10 */
  {0, 0, 1150, 110},
  {0, 0, 1400, 120},
  {0, 0, 1400, 130},
  {0, 0, 1700, 140},
  {0, 0, 1700, 150},
  {0, 1, 1950, 160},
  {1, 1, 2200, 180},
  {1, 2, 2550, 200},	/* 18 */
  {3, 7, 6400, 400},      
  {3, 8, 7000, 400},
  {4, 9, 8100, 400},
  {4, 10, 9700, 400},
  {5, 11, 11300, 400},
  {5, 12, 13400, 400},
  {6, 13, 15500, 400},    /* 25 */
  {6, 14, 20000, 600},	
  {7, 15, 25000, 600},
  {7, 16, 35000, 600},
  {8, 17, 40000, 600},
  {8, 18, 50000, 600},	/* 30 */
  {9, 19, 60000, 600},
  {9, 20, 70000, 600},
  {10,20, 80000, 600},
  {10,20, 90000, 600},
  {10,20, 90000, 600},	/* 35 */
  {1, 3, 2800, 220},      /* 18/01 */
  {2, 3, 3050, 240},
  {2, 4, 3300, 260},
  {2, 5, 3800, 280},
  {3, 6, 4800, 300}       /* 18/100 (40) */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[36] = {
  {-99, -99, -90, -99, -60},
  {-90, -90, -60, -90, -50},
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	/* 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		/* 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25},
  {30, 30, 20, 25, 25},
  {30, 30, 20, 25, 25},
  {30, 35, 20, 30, 30},
  {30, 35, 20, 30, 30},
  {35, 35, 25, 30, 30},		/* 30 */
  {35, 35, 25, 35, 35},
  {35, 35, 25, 35, 35},
  {35, 40, 25, 35, 35},
  {35, 40, 30, 35, 35},
  {40, 40, 30, 35, 35}		/* 35 */
};



/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[36] = {
  1,				/* 0 */
  2,				/* 1 */
  2,
  2,
  2,
  2,				/* 5 */
  2,
  2,
  3,
  3,
  3,				/* 10 */
  3,
  3,
  3,
  4,
  4,				/* 15 */
  4,
  4,
  4,
  4,
  4,				/* 20 */
  5,
  5,
  5,
  5,
  5,				/* 25 */
  5,
  5,
  5,
  5,
  5,				/* 30 */
  5,
  5,
  5,
  5,
  5				/* 35 */
};


/* [dex] apply (all) */
struct dex_app_type dex_app[36] = {
  {-7, -7, 6},
  {-6, -6, 5},
  {-4, -4, 5},
  {-3, -3, 4},  /*  3 */
  {-2, -2, 3},
  {-1, -1, 2},
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},	/* 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},	/* 18 */
  {3, 3, -4},   
  {3, 3, -4},	/* 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6},
  {5, 5, -6},
  {5, 5, -6},
  {6, 6, -7},
  {6, 6, -7},
  {6, 6, -7},
  {6, 6, -7},
  {7, 7, -8},
  {7, 7, -8},
  {7, 7, -8},
  {7, 7, -9}	/* 35 */
};



/* [con] apply (all) */
struct con_app_type con_app[36] = {
  {-4, 20},
  {-3, 25},
  {-2, 30},
  {-2, 35},	/* 3 */
  {-1, 40},
  {-1, 45},
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},	/* 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},
  {2, 95},
  {2, 97},
  {3, 99},
  {3, 99},
  {4, 99},	/* 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99},
  {6, 100},
  {7, 100},
  {7, 100},
  {7, 100},
  {7, 100},	/* 30 */
  {8, 100},
  {8, 100},
  {8, 100},
  {8, 100},
  {9, 100}	/* 35 */
};



/* [int] apply (all) */
struct int_app_type int_app[36] = {
  {3},
  {5},				/* 1 */
  {7},
  {8},
  {9},
  {10},				/* 5 */
  {11},
  {12},
  {13},
  {15},
  {17},				/* 10 */
  {19},
  {22},
  {25},
  {30},
  {35},				/* 15 */
  {40},
  {45},
  {50},
  {53},
  {55},				/* 20 */
  {56},
  {57},
  {58},
  {59},
  {60},				/* 25 */
  {61},
  {62},
  {63},
  {64},
  {65},				/* 30 */
  {66},
  {69},
  {70},
  {80},
  {99}				/* 35 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[36] = {
  {0},				/* 0 */
  {0},				/* 1 */
  {0},
  {0},
  {0},
  {0},				/* 5 */
  {0},
  {0},
  {0},
  {0},
  {0},				/* 10 */
  {0},
  {2},
  {2},
  {3},
  {3},				/* 15 */
  {3},
  {4},
  {5},				/* 18 */
  {6},
  {6},				/* 20 */
  {6},
  {6},
  {7},
  {7},
  {7},				/* 25 */
  {7},
  {8},
  {8},
  {8},
  {8},				/* 30 */
  {8},
  {8},
  {8},
  {9},
  {9}				/* 35 */
};



const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",		/* 0 */
  "You feel less protected.",	/* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",		/* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",		/* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",		/* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.",/* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",		/* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",		/* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",		/* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",			/* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "!Animate Dead!",		/* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",	/* 50 */
  "Your feet seem less boyant.",
  "!Relocate!",
  "!Peace!",
  "You find a soft landing spot.",
  "You float gently to the ground.",
  "You feel less insulated.",
  "Your gills disappear and you return to normal.",
  "!Group Fly!",
  "!Group Invis!",
  "!Group Prot E!",
  "!Acid Arrow!",
  "!Flame Arrow!",
  "!Cure serious!",
  "!Minute Meteor!",
  "!Area lightning!",
  "!Group Water Breath!",
  "You feel your force shield dwindle off.",
  "You feel yourself becoming solid again.",
  "You feel your strength returning.",
  "Your skin becomes supple again.",
  "You feel warm again.",
  "You feel your prayers run out.",
  "You feel your skin become soft again.",
  "You feel your stomach rumble.",
  "!Feast All!",
  "!BladeBarrier!",
  "!Conj Elemental!",
  "!MONSUM 1!",
  "!MONSUM 2!",
  "!MONSUM 3!",
  "!MONSUM 4!",
  "!MONSUM 5!",
  "Your courage returns!",
  "You feel your bravery return to normal.",
  "!KNOCK!",
  "You regain your senses.",
  "!Planeshift!",
  "Your images recombine into one.",
  "You feel more in tune with your surroundings.",
  "You start to slow down.",
  "!DISPEL MAGIC!",
  "You feel your life force return to you.",
  "You feel your shell of warm dissipate.",
  "!94",
  "!95",
  "!96",
  "!97",
  "!98",
  "!99",
  "!100",
  "!IDENTIFY!",
  "The flames surrounding you dies down.",
  "!GAS BREATH!",
  "You feel warmer.",
  "You stop dissolving.",
  "!LIGHTNING BREATH!",
  "You feel your health return to normal.",
  "Your link to mana returns to normal.",
  "You feel your stamina return to normal.",
  "!UNUSED!"
};



const char *npc_class_types[] = {
  "Normal",
  "Undead",
  "\n"
};


const char *align_types[] = {
   "Evil Incarnate",
   "Demonic",
   "Evil",
   "Sinister",
   "Nasty",
   "Neutral",
   "Helpful",
   "Nice",
   "Good",
   "Saintly",
   "Godly",
   "\n"
};

const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};


const int movement_loss[] =
{
  1,				/* Inside     */
  1,				/* City       */
  2,				/* Field      */
  3,				/* Forest     */
  4,				/* Hills      */
  6,				/* Mountains  */
  4,				/* Swimming   */
  1,				/* Unswimable */
  1,			        /* Flight     */
  10,				/* quicksand  */
  5,	  			/* lava	      */
};


const char *weekdays[7] = {
  "the Day of the Moon",
  "the Day of Lightning",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"};


const char *month_name[17] = {
  "Month of Winter",		/* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};

const char * rmode[] =    {
  "Never resets",
  "Resets only when deserted",
  "Always resets",
  "\n"
};

