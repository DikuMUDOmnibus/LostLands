/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* preamble *************************************************************/


#include <sys/types.h>

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/
#define ANYRACE    -1
#define ANYALIGN   -1

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING	 0		/* Playing - Nominal state	*/
#define CON_CLOSE	 1		/* Disconnecting		*/
#define CON_GET_NAME	 2		/* By what name ..?		*/
#define CON_NAME_CNFRM	 3		/* Did I get that right, x?	*/
#define CON_PASSWORD	 4		/* Password:			*/
#define CON_NEWPASSWD	 5		/* Give me a password for x	*/
#define CON_CNFPASSWD	 6		/* Please retype password:	*/
#define CON_QSEX	 7		/* Sex?				*/
#define CON_QCLASS	 8		/* Class?			*/
#define CON_RMOTD	 9		/* PRESS RETURN after MOTD	*/
#define CON_MENU	 10		/* Your choice: (main menu)	*/
#define CON_EXDESC	 11		/* Enter a new description:	*/
#define CON_CHPWD_GETOLD 12		/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW 13		/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY   14		/* Verify new password		*/
#define CON_DELCNF1	 15		/* Delete confirmation 1	*/
#define CON_DELCNF2	 16		/* Delete confirmation 2	*/
#define CON_QATTRIB      17             /* query attributes 		*/
#define CON_QRACE        18		/* query race 			*/
#define CON_IEDIT	 19		/* OLC modes 			*/
#define CON_REDIT	 20
#define CON_ZEDIT	 21
#define CON_REINCNF1	 22
#define CON_REINCNF2     23
#define CON_MEDIT	 24

/* defined constants **********************************************/
#define LVL_IMPL	34
#define LVL_GRGOD	33
#define LVL_GOD		32
#define LVL_IMMORT	31

#define LVL_FREEZE	LVL_GRGOD

#define NUM_OF_DIRS	6	/* number of directions in a room (nsewud) */
#define NUM_WEARS	21	/* # of eq positions see objs.h */
#define NUM_CLASSES	13	/* # of PC classes see class.h */
#define NUM_RACES	12	/* # of PC races see class.h */

#define RENT_COST	200  /* flat rate for rent */

#define LAVA_DAMAGE	15   /* damage for going in Lava */
#define UNWAT_DAMAGE	10   /* damage for going underwater */
#define CRIT_DAMAGE      5   /* damage for being crit hit */

#define OPT_USEC	100000	/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (10 RL_SEC)
#define PULSE_VIOLENCE  (2 RL_SEC)

#define SMALL_BUFSIZE		1024
#define LARGE_BUFSIZE		(12 * 1024)
#define GARBAGE_SPACE		32

#define MAX_STRING_LENGTH	8192
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		60
#define MAX_NAME_LENGTH		20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE		3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS		200 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT2		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT3		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT		6 /* Used in obj_file_elem *DO*NOT*CHANGE* */


/***********************************************************************
 * Structures                                                          *
 **********************************************************************/

typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;
typedef char			byte;

typedef sh_int	room_num;
typedef sh_int	obj_num;


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/


/* object flags; used in obj_data */
struct obj_flag_data {
   int	value[4];	/* Values of the item (see list)    */
   byte type_flag;	/* Type of item			    */
   long	wear_flags;	/* Where you can wear it	    */
   long	extra_flags;	/* If it hums, glows, etc.	    */
   long	extra_flags2;	/* If it hums, glows, etc.	    */
   int	weight;		/* Weigt what else                  */
   int	cost;		/* Value when sold (gp.)            */
   int	cost_per_day;	/* Cost to keep pr. real day        */
   int	timer;		/* Timer for object                 */
   long	bitvector;	/* To set chars bits                */
   long	bitvector2;	/* To set chars bits                */
   long	bitvector3;	/* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_num item_number;		/* Where in data-base			*/
   room_num in_room;		/* In what room -1 when conta/carr	*/

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char	*name;                    /* Title of object :get etc.        */
   char	*description;		  /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   char *attack_verb;		  /* supplimentary attack verb        */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;	  /* Worn by?			      */
   char owner_name[20];		  /* for Engraved items		      */
   sh_int worn_on;		  /* Worn where?		      */
   sh_int orig_zone;		  /* Origination Zone		      */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
   obj_num item_number;

   int	value[4];
   long	extra_flags;
   long	extra_flags2;
   int	weight;
   int	timer;
   long	bitvector;
   long	bitvector2;
   long	bitvector3;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
   char owner_name[20];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   int	time;
   int	rentcode;
   int	net_cost_per_diem;
   int	gold;
   int	account;
   int	nitems;
   int	spare0;
   int	spare1;
   int	spare2;
   int	spare3;
   int	spare4;
   int	spare5;
   int	spare6;
   int	spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
   char	*general_description;       /* When look DIR.			*/

   char	*keyword;		/* for open/close			*/

   sh_int exit_info;		/* Exit info				*/
   obj_num key;			/* Key's number (-1 for no key)		*/
   room_num to_room;		/* Where direction leads (NOWHERE)	*/
   room_num to_room_vnum;       /* the vnum of the room. Used for OLC   */
};


struct teleport_data {
	int time;
	int targ;
	long mask;
	int cnt;
        int obj;
};

/* Broadcast Info, if it's a broad   */
struct broadcast_data {
	int channel;  /* number from 0 to 32 */
	int targ1;
	int targ2;
};

/* ================== Memory Structure for room ======================= */
struct room_data {
   room_num number;		/* Rooms number	(vnum)		      */
   sh_int zone;                 /* Room zone (for resetting)          */
   int	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   int room_flags;		/* DEATH,DARK ... etc                 */

   struct teleport_data *tele;  /* Teleport Info, if it's a teleport  */
   struct broadcast_data *broad; /* Broadcast Info, if it's a broad   */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;

/* MOBProgram foo */
struct mob_prog_act_list {
  struct mob_prog_act_list *next;
  char *buf;
  struct char_data *ch;
  struct obj_data *obj;
  void *vo;
};
typedef struct mob_prog_act_list MPROG_ACT_LIST;

struct mob_prog_data {
  struct mob_prog_data *next;
  int type;
  char *arglist;
  char *comlist;
};

typedef struct mob_prog_data MPROG_DATA;

extern bool MOBTrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024

/* end of MOBProg foo */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   byte hours, day, month;
   sh_int year;
};

/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char	passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char	*name;	       /* PC / NPC s name (kill ...  )         */
   char	*short_descr;  /* for NPC 'actions'                    */
   char	*long_descr;   /* for 'look'			       */
   char	*description;  /* Extra descriptions                   */
   char	*title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   int  class;         /* PC / NPC's class		       */
   byte level;         /* PC / NPC's level                     */
   byte race;	       /* PC / NPC's race		       */
   int	hometown;      /* PC s Hometown (zone)                 */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */
   int zone_edit;      /* PC Zone being edited *reditmod*      */
   struct obj_data *obj_buffer;/*PC object edit buffer*oeditmod*/
   struct char_data *mob_buf;/*PC mob edit buffer     *meditmod*/
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   sh_int mana;
   sh_int max_mana;     /* Max move for PC/NPC			   */
   sh_int hit;
   sh_int max_hit;      /* Max hit for PC/NPC                      */
   sh_int move;
   sh_int max_move;     /* Max move for PC/NPC                     */

   sh_int armor;        /* Internal -100..100, external -10..10 AC */
   int	gold;           /* Money carried                           */
   int	bank_gold;	/* Gold the char has in a bank account	   */
   int	exp;            /* The experience of the player            */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
};


/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   int	alignment;		/* +-3000 for alignments                */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   long	act;			/* act flag for NPC's; player flag for PC's */
   long	act2;			/* act flag for NPC's; player flag for PC's */
   long	act3;			/* act flag for NPC's; player flag for PC's */

   long	affected_by;		/* Bitvector for spells/skills affected by */
   long	affected_by2;		/* Bitvector for spells/skills affected by */
   long	affected_by3;		/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;	/* Opponent				*/
   struct char_data *hunting;	/* Char hunted by this char		*/

   byte position;		/* Standing, fighting, sleeping, etc.	*/

   int	carry_weight;		/* Carried weight			*/
   byte carry_items;		/* Number of items carried		*/
   int	timer;			/* Timer for update			*/

   struct char_special_data_saved saved; /* constants saved in plrfile	*/
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   byte skills[MAX_SKILLS+1];	/* array of skills plus skill 0		*/
   byte PADDING0;		/* How many can you learn yet this level*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   byte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   room_num load_room;		/* Which room to place char in		*/
   long	pref;			/* preference flags for PC's.		*/
   long	pref2;			/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[3];         /* Drunk, full, thirsty			*/

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ubyte mirror_images;
   ubyte spare1;
   ubyte spare2;
   ubyte spare3;
   ubyte spare4;
   ubyte spare5;
   int spells_to_learn;		/* How many can you learn yet this level*/
   int spare7;
   int spare8;
   int spare9;
   int spare10;
   int clan;
   int honor;
   int killed_player;
   int killed_by_player;
   int killed_by_mob;
   int inn_num;			/* room vnum of the inn */
   long	channel;		/* 32 broadcasting channels */
   long	spare18;
   long	spare19;
   long	spare20;
   long	spare21;
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char	*poofin;		/* Description on arrival of a god.     */
   char	*poofout;		/* Description upon a god's exit.       */
   struct alias *aliases;	/* Character's aliases			*/
   struct quest *quests;	/* Character's questbits		*/
   long last_tell;		/* idnum of last tell from		*/
   void *last_olc_targ;		/* olc control				*/
   int last_olc_mode;		/* olc control				*/
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   int	attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;	    /* List of attackers to remember	       */
   byte damnodice;          /* The number of damage dice's	       */
   byte damsizedice;        /* The size of the damage dice's           */
   byte attack_num;	    /* The number of attacks		       */
   int wait_state;	    /* Wait state for bashed mobs	       */
   sh_int orig_zone;	    /* Origination Zone 		       */
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long	bitvector;       /* Tells which bits to set (AFF_XXX)       */
   long	bitvector2;       /* Tells which bits to set (AFF2_XXX)       */
   long	bitvector3;       /* Tells which bits to set (AFF3_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
   int pfilepos;			 /* playerfile pos		  */
   sh_int nr;                            /* Mob's rnum			  */
   room_num in_room;                     /* Location (real room number)	  */
   room_num was_in_room;		 /* location for linkdead people  */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;	 /* Abilities without modifiers   */
   struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;	/* PC/NPC specials	  */
   struct player_special_data *player_specials; /* PC specials		  */
   struct mob_special_data mob_specials;	/* NPC specials		  */

   struct affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   struct follow_type *followers;        /* List of chars followers       */
   struct char_data *master;             /* Who is char following?        */
   MPROG_ACT_LIST *mpact;
   int mpactnum;
   
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH];
   char	title[MAX_TITLE_LENGTH+1];
   byte sex;
   int class;
   byte level;
   byte race;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int	played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char	*text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct descriptor_data {
   int	descriptor;		/* file descriptor for socket		*/
   char	host[HOST_LENGTH+1];	/* hostname				*/
   byte	bad_pws;		/* number of bad pw attemps this login	*/
   int	connected;		/* mode of 'connectedness'		*/
   int	wait;			/* wait for how many loops		*/
   int	desc_num;		/* unique num assigned to desc		*/
   time_t login_time;		/* when the person connected		*/
   char	*showstr_head;		/* for paging through texts		*/
   char	*showstr_point;		/*		-			*/
   char	**str;			/* for the modify-str system		*/
   int	max_str;		/*		-			*/
   long	mail_to;		/* name for mail system			*/
   int	prompt_mode;		/* control of prompt-printing		*/
   char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
   char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;		/* ptr to the current output buffer	*/
   int  bufptr;			/* ptr to end of current output		*/
   int	bufspace;		/* space left in the output buffer	*/
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;		/* q of unprocessed input		*/
   struct char_data *character;	/* linked to char			*/
   struct char_data *original;	/* original char if switched		*/
   struct descriptor_data *snooping; /* Who is this char snooping	*/
   struct descriptor_data *snoop_by; /* And who is snooping this char	*/
   struct descriptor_data *next; /* link to next descriptor		*/
   int edit_mode;                /* editing sub mode */
   int edit_number;              /* virtual num of thing being edited */
   int edit_number2;             /* misc number for editing */
   int edit_zone;                /* which zone object is part of      */
   void **misc_data;              /* misc data, usually for extra data crap */
   struct obj_data *edit_obj;    /* iedit */
   struct room_data *edit_room;  /* redit */
   struct zedit_struct *zedit_zone; /* zedit */
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   long int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int	change;	/* How fast and what way does it change. */
   int	sky;	/* How is the sky. */
   int	sunlight;	/* And how much sun. */
};


struct title_type {
   char	*title;
   int	exp;
};


/* element in monster and object index-tables   */
struct index_data {
   int	virtual;    /* virtual number of this mob/obj           */
   int	*number;   /* number of existing units of this mob/obj	*/
   int  progtypes;  /* program types for MOBProg		*/
   MPROG_DATA *mobprogs; /* programs for MOBProg		*/
   SPECIAL(*func);
};
