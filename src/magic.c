/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;

void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);

void damage(struct char_data * ch, struct char_data * victim,
	         int damage, int weapontype, struct obj_data *obj);

void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int dice(int number, int size);
extern struct spell_info_type spell_info[];

bool CAN_MURDER(struct char_data * ch, struct char_data * victim);

struct char_data *read_mobile(int, int, int);


/*
 * Saving throws for:
 * MCTWBSDWN
 *   PARA, ROD, PETRI, BREATH, SPELL
 *     Levels 0-40
 */

const byte saving_throws[NUM_CLASSES][5][41] = {

  {				/* Mages */
		{90, 70, 69, 68, 67, 66, 65, 63, 61, 60, 59,	/* 0 - 10 */
/* PARA */	57, 55, 54, 53, 53, 52, 51, 50, 48, 46,		/* 11 - 20 */
		45, 44, 42, 40, 38, 36, 34, 32, 30, 28,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 55, 53, 51, 49, 47, 45, 43, 41, 40, 39,	/* 0 - 10 */
/* ROD */	37, 35, 33, 31, 30, 29, 27, 25, 23, 21,		/* 11 - 20 */
		20, 19, 17, 15, 14, 13, 12, 11, 10, 9,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,	/* 0 - 10 */
/* PETRI */	47, 45, 43, 41, 40, 39, 37, 35, 33, 31,		/* 11 - 20 */
		30, 29, 27, 25, 23, 21, 19, 17, 15, 13,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 73, 71, 69, 67, 65, 63, 61, 60, 59,	/* 0 - 10 */
/* BREATH */	57, 55, 53, 51, 50, 49, 47, 45, 43, 41,		/* 11 - 20 */
		40, 39, 37, 35, 33, 31, 29, 27, 25, 23,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 60, 58, 56, 54, 52, 50, 48, 46, 45, 44,	/* 0 - 10 */
/* SPELL */	42, 40, 38, 36, 35, 34, 32, 30, 28, 26,		/* 11 - 20 */
		25, 24, 22, 20, 18, 16, 14, 12, 10, 8,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */
  },

  {				/* Clerics */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Thieves */
		{90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,	/* 0 - 10 */
/* PARA */	55, 54, 53, 52, 51, 50, 49, 48, 47, 46,		/* 11 - 20 */
		45, 44, 43, 42, 41, 40, 39, 38, 37, 36,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,	/* 0 - 10 */
/* ROD */	50, 48, 46, 44, 42, 40, 38, 36, 34, 32,		/* 11 - 20 */
		30, 28, 26, 24, 22, 20, 18, 16, 14, 13,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,	/* 0 - 10 */
/* PETRI */	50, 49, 48, 47, 46, 45, 44, 43, 42, 41,		/* 11 - 20 */
		40, 39, 38, 37, 36, 35, 34, 33, 32, 31,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,	/* 0 - 10 */
/* BREATH */	70, 69, 68, 67, 66, 65, 64, 63, 62, 61,		/* 11 - 20 */
		60, 59, 58, 57, 56, 55, 54, 53, 52, 51,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,	/* 0 - 10 */
/* SPELL */	55, 53, 51, 49, 47, 45, 43, 41, 39, 37,		/* 11 - 20 */
		35, 33, 31, 29, 27, 25, 23, 21, 19, 17,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}			/* 31 - 40 */
  },

  {				/* Warriors */
		{90, 70, 68, 67, 65, 62, 58, 55, 53, 52, 50,	/* 0 - 10 */
/* PARA */	47, 43, 40, 38, 37, 35, 32, 28, 25, 24,		/* 11 - 20 */
		23, 22, 20, 19, 17, 16, 15, 14, 13, 12,		/* 21 - 30 */
		11, 10, 9, 8, 7, 6, 5, 4, 3, 2},		/* 31 - 40 */

		{90, 80, 78, 77, 75, 72, 68, 65, 63, 62, 60,	/* 0 - 10 */
/* ROD */	57, 53, 50, 48, 47, 45, 42, 38, 35, 34,		/* 11 - 20 */
		33, 32, 30, 29, 27, 26, 25, 24, 23, 22,		/* 21 - 30 */
		20, 18, 16, 14, 12, 10, 8, 6, 5, 4},		/* 31 - 40 */

		{90, 75, 73, 72, 70, 67, 63, 60, 58, 57, 55,	/* 0 - 10 */
/* PETRI */	52, 48, 45, 43, 42, 40, 37, 33, 30, 29,		/* 11 - 20 */
		28, 26, 25, 24, 23, 21, 20, 19, 18, 17,		/* 21 - 30 */
		16, 15, 14, 13, 12, 11, 10, 9, 8, 7},		/* 31 - 40 */

		{90, 85, 83, 82, 80, 75, 70, 65, 63, 62, 60,	/* 0 - 10 */
/* BREATH */	55, 50, 45, 43, 42, 40, 37, 33, 30, 29,		/* 11 - 20 */
		28, 26, 25, 24, 23, 21, 20, 19, 18, 17,		/* 21 - 30 */
		16, 15, 14, 13, 12, 11, 10, 9, 8, 7},		/* 31 - 40 */

		{90, 85, 83, 82, 80, 77, 73, 70, 68, 67, 65,	/* 0 - 10 */
/* SPELL */	62, 58, 55, 53, 52, 50, 47, 43, 40, 39,		/* 11 - 20 */
		38, 36, 35, 34, 33, 31, 30, 29, 28, 27,		/* 21 - 30 */
		25, 23, 21, 19, 17, 15, 13, 11, 9, 7}		/* 31 - 40 */
  },

  {				/* Barbarians */
		{90, 70, 68, 67, 65, 62, 58, 55, 53, 52, 50,	/* 0 - 10 */
/* PARA */	47, 43, 40, 38, 37, 35, 32, 28, 25, 24,		/* 11 - 20 */
		23, 22, 20, 19, 17, 16, 15, 14, 13, 12,		/* 21 - 30 */
		11, 10, 9, 8, 7, 6, 5, 4, 3, 2},		/* 31 - 40 */

		{90, 80, 78, 77, 75, 72, 68, 65, 63, 62, 60,	/* 0 - 10 */
/* ROD */	57, 53, 50, 48, 47, 45, 42, 38, 35, 34,		/* 11 - 20 */
		33, 32, 30, 29, 27, 26, 25, 24, 23, 22,		/* 21 - 30 */
		20, 18, 16, 14, 12, 10, 8, 6, 5, 4},		/* 31 - 40 */

		{90, 75, 73, 72, 70, 67, 63, 60, 58, 57, 55,	/* 0 - 10 */
/* PETRI */	52, 48, 45, 43, 42, 40, 37, 33, 30, 29,		/* 11 - 20 */
		28, 26, 25, 24, 23, 21, 20, 19, 18, 17,		/* 21 - 30 */
		16, 15, 14, 13, 12, 11, 10, 9, 8, 7},		/* 31 - 40 */

		{90, 85, 83, 82, 80, 75, 70, 65, 63, 62, 60,	/* 0 - 10 */
/* BREATH */	55, 50, 45, 43, 42, 40, 37, 33, 30, 29,		/* 11 - 20 */
		28, 26, 25, 24, 23, 21, 20, 19, 18, 17,		/* 21 - 30 */
		16, 15, 14, 13, 12, 11, 10, 9, 8, 7},		/* 31 - 40 */

		{90, 85, 83, 82, 80, 77, 73, 70, 68, 67, 65,	/* 0 - 10 */
/* SPELL */	62, 58, 55, 53, 52, 50, 47, 43, 40, 39,		/* 11 - 20 */
		38, 36, 35, 34, 33, 31, 30, 29, 28, 27,		/* 21 - 30 */
		25, 23, 21, 19, 17, 15, 13, 11, 9, 7}		/* 31 - 40 */
  },

  {				/* Samurai */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Druid  */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Wizards */
		{90, 70, 69, 68, 67, 66, 65, 63, 61, 60, 59,	/* 0 - 10 */
/* PARA */	57, 55, 54, 53, 53, 52, 51, 50, 48, 46,		/* 11 - 20 */
		45, 44, 42, 40, 38, 36, 34, 32, 30, 28,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 55, 53, 51, 49, 47, 45, 43, 41, 40, 39,	/* 0 - 10 */
/* ROD */	37, 35, 33, 31, 30, 29, 27, 25, 23, 21,		/* 11 - 20 */
		20, 19, 17, 15, 14, 13, 12, 11, 10, 9,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,	/* 0 - 10 */
/* PETRI */	47, 45, 43, 41, 40, 39, 37, 35, 33, 31,		/* 11 - 20 */
		30, 29, 27, 25, 23, 21, 19, 17, 15, 13,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 73, 71, 69, 67, 65, 63, 61, 60, 59,	/* 0 - 10 */
/* BREATH */	57, 55, 53, 51, 50, 49, 47, 45, 43, 41,		/* 11 - 20 */
		40, 39, 37, 35, 33, 31, 29, 27, 25, 23,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 60, 58, 56, 54, 52, 50, 48, 46, 45, 44,	/* 0 - 10 */
/* SPELL */	42, 40, 38, 36, 35, 34, 32, 30, 28, 26,		/* 11 - 20 */
		25, 24, 22, 20, 18, 16, 14, 12, 10, 8,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */
  },

  {				/* MONK */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Avatar */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Ninja */
		{90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,	/* 0 - 10 */
/* PARA */	55, 54, 53, 52, 51, 50, 49, 48, 47, 46,		/* 11 - 20 */
		45, 44, 43, 42, 41, 40, 39, 38, 37, 36,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,	/* 0 - 10 */
/* ROD */	50, 48, 46, 44, 42, 40, 38, 36, 34, 32,		/* 11 - 20 */
		30, 28, 26, 24, 22, 20, 18, 16, 14, 13,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,	/* 0 - 10 */
/* PETRI */	50, 49, 48, 47, 46, 45, 44, 43, 42, 41,		/* 11 - 20 */
		40, 39, 38, 37, 36, 35, 34, 33, 32, 31,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,	/* 0 - 10 */
/* BREATH */	70, 69, 68, 67, 66, 65, 64, 63, 62, 61,		/* 11 - 20 */
		60, 59, 58, 57, 56, 55, 54, 53, 52, 51,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,	/* 0 - 10 */
/* SPELL */	55, 53, 51, 49, 47, 45, 43, 41, 39, 37,		/* 11 - 20 */
		35, 33, 31, 29, 27, 25, 23, 21, 19, 17,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}			/* 31 - 40 */
  },

  {				/* Dual Class  */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  },

  {				/* Triple Class */
		{90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,	/* 0 - 10 */
/* PARA */	33, 31, 30, 29, 27, 26, 25, 24, 23, 22,		/* 11 - 20 */
		21, 20, 18, 15, 14, 12, 10, 9, 8, 7,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,	/* 0 - 10 */
/* ROD */	53, 51, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		41, 40, 38, 35, 34, 32, 30, 29, 28, 27,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,	/* 0 - 10 */
/* PETRI */	48, 46, 45, 44, 43, 41, 40, 39, 38, 37,		/* 11 - 20 */
		36, 35, 33, 31, 29, 27, 25, 24, 23, 22,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,	/* 0 - 10 */
/* BREATH */	63, 61, 60, 59, 57, 56, 55, 54, 53, 52,		/* 11 - 20 */
		51, 50, 48, 45, 44, 42, 40, 39, 38, 37,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 31 - 40 */

		{90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,	/* 0 - 10 */
/* SPELL */	58, 56, 55, 54, 53, 51, 50, 49, 48, 47,		/* 11 - 20 */
		46, 45, 43, 41, 39, 37, 35, 34, 33, 32,		/* 21 - 30 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0}		/* 31 - 40 */
  }
};


int mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  if (IS_NPC(ch)) /* NPCs use warrior tables according to some book */
    save = saving_throws[CLASS_WARRIOR][type][(int) GET_LEVEL(ch)];
  else
    save = saving_throws[(int) GET_CLASS_NUM_FULL(ch)][type][(int) GET_LEVEL(ch)];

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) < number(0, 99))
    return TRUE;
  else
    return FALSE;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  static struct affected_type *af, *next;
  static struct char_data *i;
  extern char *spell_wear_off_msg[];

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)	/* No action */
	af->duration = -1;	/* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\r\n", i);
	    }
	affect_remove(i, af);
      }
    }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL;
  struct obj_data *obj1 = NULL;
  struct obj_data *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
	send_to_char("A wart sprouts on your nose.\r\n", ch);
	break;
      case 1:
	send_to_char("Your hair falls out in clumps.\r\n", ch);
	break;
      case 2:
	send_to_char("A huge corn develops on your big toe.\r\n", ch);
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}

/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */

void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int is_mage = 0, is_cleric = 0;
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return;

  is_mage = (IS_MAGIC_USER(ch) || IS_WIZARD(ch));
  is_cleric = (IS_CLERIC(ch) || IS_DRUID(ch));

  switch (spellnum) {
    /* Mostly mages */
  case SPELL_MAGIC_MISSILE:
    if (is_mage)
      dam = dice(MIN(5, lvD4(ch)) , 8) + lvD4(ch);
    else 
      dam = dice(MIN(5, lvD4(ch)), 6) + lvD4(ch);
    if (AFF_FLAGGED(victim, AFF_SHIELD)) {
	send_to_char("Your shield absorbs the magic missile!\r\n", victim);
	send_to_char("Your magic missile disappears!!\r\n", ch);
	dam = 0;
    }
    break;
  case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
    if (is_mage)
      dam = dice(1, 8);
    else 
      dam = dice(1, 6);
    break;
  case SPELL_BURNING_HANDS:
    if (is_mage)
      dam = dice(1, 6) + MIN(20, GET_LEVEL(ch));
    else
      dam = dice(1, 4) + MIN(20, GET_LEVEL(ch));
    break;
  case SPELL_SHOCKING_GRASP:
    if (is_mage)
      dam = dice(1, 8) + GET_LEVEL(ch);
    else
      dam = dice(1, 6) + GET_LEVEL(ch);
    break;
  case SPELL_LIGHTNING_BOLT:
    if (is_mage)
      dam = dice(MIN(10, lvD2(ch)), 8);
    else
      dam = dice(MIN(10, lvD2(ch)), 6);
    break;
  case SPELL_COLOR_SPRAY:
    if (is_mage)
      dam = dice(1, 8) + lvD8(ch);
    else
      dam = dice(1, 6) + lvD8(ch);
    break;
  case SPELL_FIREBALL:
    if (is_mage)
      dam = dice(MIN(12, lvD2(ch)), 8);
    else
      dam = dice(MIN(12, lvD2(ch)), 6);
    break;

    /* Mostly clerics */
  case SPELL_DISPEL_EVIL:
    dam = dice(lvD4(ch), 8) + (int) (GET_HIT(victim) / 2);

    if (IS_EVIL(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
      act("You lift high your unholy symbol against yourself.", FALSE, ch, 0, victim, TO_CHAR);
    } else if (IS_GOOD(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return;
    }
    break;
  case SPELL_DISPEL_GOOD:
    dam = dice(lvD4(ch), 8) + (int) (GET_HIT(victim) / 2);

    if (IS_GOOD(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
      act("You lift high your holy symbol against yourself.", FALSE, ch, 0, victim, TO_CHAR);
    } else if (IS_EVIL(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return;
    }
    break;

  case SPELL_CALL_LIGHTNING:
    dam = dice((lvD2(ch) + 2), 8);
    break;

  case SPELL_HARM:
    if (GET_LEVEL(victim) < GET_LEVEL(ch))
	dam = dice(lvD2(ch), lvD4(ch));
    else
	dam = dice(lvD2(ch), lvD6(ch));
    break;

  case SPELL_ENERGY_DRAIN:
    if (GET_LEVEL(victim) <= GET_LEVEL(ch))
      dam = dice(lvD3(ch), lvD3(ch));
    else
      dam = dice(lvD3(ch), lvD4(ch));
    break;

  case SPELL_ACID_ARROW:
    dam = dice(lvD3(ch), 4);
    break;

  case SPELL_FLAME_ARROW:
    dam = dice(5*lvD5(ch), 4);
    break;

    /* Area spells */
  case SPELL_MINUTE_METEOR:
    dam = dice(lvD5(ch), 6);
    break;
  case SPELL_CONE_OF_COLD:
    dam = dice(MIN(20, GET_LEVEL(ch)), 4);
    break;
  case SPELL_AREA_LIGHTNING:
    dam = dice(MIN(20, GET_LEVEL(ch)), 6);
    break;
  case SPELL_EARTHQUAKE:
    dam = dice(GET_LEVEL(ch), lvD2(ch));
    break;
  case SPELL_FIRE_BREATH:
    dam = dice(MIN(12, lvD2(ch)), 8);
    break;
  case SPELL_GAS_BREATH:
    dam = dice(lvD2(ch), 12);
    break;
  case SPELL_FROST_BREATH:
    dam = dice(MIN(14, lvD2(ch)), 10);
    break;
  case SPELL_ACID_BREATH:
    dam = dice(MIN(GET_LEVEL(ch), 20), 12);;
    break;
  case SPELL_LIGHTNING_BREATH:
    dam = dice(MIN(10, lvD2(ch)), 8);
    break;
  case SPELL_BLADEBARRIER:
    dam = dice(8,lvD3(ch) + 1);
    break;
  } /* switch(spellnum) */

  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype))
    dam >>= 1;

  /* and finally, inflict the damage */
  if (!IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim))
      act("The Mark of Neutrality protects $N.", FALSE, ch, 0, victim, TO_CHAR);
  else
      damage(ch, victim, dam, spellnum, NULL);
}

/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		      int spellnum, int savetype)
{
  struct affected_type af, af2;
  int is_mage = FALSE;
  int is_cleric = FALSE;
  int accum_affect = FALSE;
  int accum_duration = FALSE;
  char *to_vict = NULL;
  char *to_room = NULL;

  if (victim == NULL || ch == NULL)
    return;

  is_mage = (IS_MAGIC_USER(ch) || IS_WIZARD(ch));
  is_cleric = (IS_CLERIC(ch) || IS_DRUID(ch));

  af.type = spellnum;
  af.bitvector = 0;
  af.bitvector2 = 0;
  af.bitvector3 = 0;
  af.modifier = 0;
  af.location = APPLY_NONE;

  af2.type = spellnum;
  af2.bitvector = 0;
  af2.bitvector2 = 0;
  af2.bitvector3 = 0;
  af2.modifier = 0;
  af2.location = APPLY_NONE;

  switch (spellnum) {

  case SPELL_CHILL_TOUCH:
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim)) {
	return;
    }
    af.location = APPLY_STR;
    if (mag_savingthrow(victim, savetype))
      af.duration = 1;
    else
      af.duration = 4;
    af.modifier = -1;
    accum_duration = TRUE;
    to_vict = "You feel your strength wither!";
    break;

  case SPELL_ARMOR:
    af.location = APPLY_AC;
    af.modifier = -5 - ((ch == victim) * 15);
    af.duration = 24;
    accum_duration = TRUE;
    to_vict = "You feel someone protecting you.";
    break;

  case SPELL_AID:
    af.bitvector = AFF_PROT_FIRE;
    af.bitvector2 = AFF2_PROT_COLD;

  case SPELL_BLESS:
    af.location = APPLY_HITROLL;
    af.modifier = 2 + ((ch == victim) * 3);
    af.duration = 5 + lvD6(ch);

    af2.location = APPLY_SAVING_SPELL;
    af2.modifier = -1;
    af2.duration = 6;
    to_vict = "You feel righteous.";
    break;

  case SPELL_BLINDNESS:
    if (MOB_FLAGGED(victim,MOB_NOBLIND) || mag_savingthrow(victim, savetype)) {
      send_to_char("You fail.\r\n", ch);
      return;
    }

    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim)) {
	act("The Mark of Neutrality protects $N.", FALSE, ch, 0, victim, TO_CHAR);
	return;
    }

    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 2;
    af.bitvector = AFF_BLIND;

    af2.location = APPLY_AC;
    af2.modifier = 40;
    af2.duration = 2;
    af2.bitvector = AFF_BLIND;

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_CURSE:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim)) {
	act("The Mark of Neutrality protects $N.", FALSE, ch, 0, victim, TO_CHAR);
	return;
    }

    af.location = APPLY_HITROLL;
    af.duration = 1 + (GET_LEVEL(ch) >> 1);
    af.modifier = 0 - lvD8(ch);
    af.bitvector = AFF_CURSE;

    af2.location = APPLY_DAMROLL;
    af2.duration = 1 + (GET_LEVEL(ch) >> 1);
    af2.modifier = 0 - lvD10(ch);
    af2.bitvector = AFF_CURSE;

    accum_duration = TRUE;
    to_room = "$n briefly glows red!";
    to_vict = "You feel very uncomfortable.";
    break;

  case SPELL_DETECT_ALIGN:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_ALIGN;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_INVIS:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_INVIS;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_MAGIC:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_MAGIC;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_INFRAVISION:
    af.duration = 12 + level;
    af.bitvector = AFF_INFRAVISION;
    accum_duration = TRUE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    af.duration = 12 + (GET_LEVEL(ch) >> 2);
    af.modifier = -40;
    af.location = APPLY_AC;
    af.bitvector = AFF_INVISIBLE;
    accum_duration = TRUE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim)) {
	act("The Mark of Neutrality protects $N.", FALSE, ch, 0, victim, TO_CHAR);
	break;
    }

    af.location = APPLY_STR;
    af.duration = GET_LEVEL(ch);
    af.modifier = -2;
    af.bitvector = AFF_POISON;
    accum_affect = TRUE;
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_PROT_FROM_EVIL:
    af.duration = 24;
    af.bitvector = AFF_PROTECT_EVIL;
    accum_duration = TRUE;
    to_vict = "You feel invulnerable!";
    break;

  case SPELL_SANCTUARY:
    af.duration = 4 + (int) (GET_LEVEL(ch) / 5);
    af.bitvector = AFF_SANCTUARY;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_SLEEP:
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim) && !CAN_MURDER(ch, victim))
      return;
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;
    if (mag_savingthrow(victim, savetype))
      return;

    af.duration = 4 + (GET_LEVEL(ch) >> 2);
    af.bitvector = AFF_SLEEP;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    af.location = APPLY_STR;
    af.duration = (GET_LEVEL(ch) >> 1) + 4;
    af.modifier = 1 + (int)(level / 10);
    accum_duration = TRUE;
    accum_affect = TRUE;
    to_vict = "You feel stronger!";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "Your feel your awareness improve.";
    af.duration = GET_LEVEL(ch);
    af.bitvector = AFF_SENSE_LIFE;
    accum_duration = TRUE;
    break;

  case SPELL_WATERWALK:
    af.duration = 24;
    af.bitvector = AFF_WATERWALK;
    accum_duration = TRUE;
    to_vict = "You feel webbing between your toes.";
    break;

  case SPELL_FLY:
    af.duration = 2 + GET_LEVEL(ch);
    af.modifier = -10;
    af.location = APPLY_AC;
    af.bitvector = AFF_FLYING;
    accum_duration = FALSE;
    to_vict = "You take off into the air.";
    to_room = "$n takes off into the air.";
    break;

  case SPELL_LEVITATE:
    af.duration = 8 + GET_LEVEL(ch);
    af.bitvector = AFF_WATERWALK;
    af.modifier = -5;
    af.location = APPLY_AC;
    accum_duration = TRUE;
    to_vict = "You start to float off the ground.";
    to_room = "$n begins to float off the ground.";
    break;

  case SPELL_PROT_FIRE:
    af.duration = 10 + GET_LEVEL(ch);
    af.bitvector = AFF_PROT_FIRE;
    accum_duration = FALSE;
    to_vict = "You feel a shell of insulation form around your body.";
    break;

  case SPELL_PROT_COLD:
    af.duration = 10 + GET_LEVEL(ch);
    af.bitvector2 = AFF2_PROT_COLD;
    accum_duration = FALSE;
    to_vict = "You feel a shell of warmth form around your body.";
    break;

  case SPELL_WATERBREATH:
    af.duration = 24;
    af.bitvector = AFF_WATERBREATH;
    accum_duration = FALSE;
    to_vict = "It feels as if you just sprouted some gills.";
    break;

  case SPELL_CONE_OF_COLD:
    af.duration = 1;
    af.bitvector2 = AFF2_FREEZING;
    accum_duration = TRUE;
    to_vict = "You are consumed with coldness.";
    to_room = "$n starts shivering.";
    break;

  case SPELL_ACID_ARROW:
    af.duration = 1;
    af.bitvector2 = AFF2_ACIDED;
    accum_duration = TRUE;
    to_vict = "The acid arrow drenches you.";
    to_room = "$n is drenched by an acid arrow.";
    break;

  case SPELL_FLAME_ARROW:
    af.duration = 1;
    af.bitvector2 = AFF2_BURNING;
    accum_duration = TRUE;
    to_vict = "The flame arrow sets you on fire.";
    to_room = "$n starts burning.";
    break;

  case SPELL_BARKSKIN:
    af.location = APPLY_AC;
    af.modifier = -20 - (IS_CLERIC(ch) * 20);
    af.duration = lvD5(ch);
    to_vict = "You feel your skin hardening.";
    break;

  case SPELL_STONESKIN:
    af.location = APPLY_AC;
    af.modifier = -40;
    af.duration = lvD8(ch);
    af.bitvector = AFF_SILVER;
    af.bitvector2 = AFF2_STONESKIN;
    to_vict = "You feel your skin turn into granite.";
    break;

  case SPELL_MIRROR_IMAGE:
    af.duration = 6;
    af.bitvector2 = AFF2_MIRRORIMAGE;
    af.location = APPLY_AC;
    af.modifier = -15;
    to_vict = "Your image breaks up!";
    to_room = "$n breaks up into many images!!!";
    break;

  case SPELL_BLINK:
    af.duration = 6;
    af.bitvector2 = AFF2_BLINK;
    to_vict = "You don't feel any different.";
    to_room = "You see $n shift a few feet away.";
    break;

  case SPELL_SHIELD:
    af.duration = MAX(1, lvD4(ch));
    af.bitvector = AFF_SHIELD;
    to_vict = "You create a force shield.";
    to_room = "A force shield surrounds $n.";
    af.location = APPLY_AC;
    af.modifier = -10;
    break;

  case SPELL_DEATHDANCE:
    af.duration = 24;
    af.bitvector = AFF_DEATHDANCE;
    to_vict = "You feel your life take on a whole new meaning....";
    to_room = "A wave of death dances forth from $n.";
    break;

  case SPELL_WRAITHFORM:
    af.duration = 8;
    af.bitvector3 = AFF3_PASSDOOR;
    to_vict = "You turn translucent!";
    to_room = "$n turns translucent!";
    break;

  case SPELL_FIRE_BREATH:
    af.duration = 1;
    af.bitvector2 = AFF2_BURNING;
    accum_duration = TRUE;
    to_vict = "You are engulfed in flames.";
    to_room = "$n is engulfed in flames.";
    break;

  case SPELL_FROST_BREATH:
    af.duration = 1;
    af.bitvector2 = AFF2_FREEZING;
    accum_duration = TRUE;
    to_vict = "You are consumed with coldness.";
    to_room = "$n starts shivering.";
    break;

  case SPELL_ACID_BREATH:
    af.duration = 1;
    af.bitvector2 = AFF2_ACIDED;
    accum_duration = TRUE;
    to_vict = "You are drenched in acid.";
    to_room = "$n drops some acid.";
    break;

  case SPELL_BURN:
    af.duration = 1;
    af.bitvector2 = AFF2_BURNING;
    accum_duration = TRUE;
    to_vict = "You are engulfed in flames.";
    to_room = "$n is engulfed in flames.";
    break;

  case SPELL_FREEZE:
    af.duration = 1;
    af.bitvector2 = AFF2_FREEZING;
    accum_duration = TRUE;
    to_vict = "You are consumed with coldness.";
    to_room = "$n starts shivering.";
    break;

  case SPELL_ACID:
    af.duration = 1;
    af.bitvector2 = AFF2_ACIDED;
    accum_duration = TRUE;
    to_vict = "You are drenched in acid.";
    to_room = "$n drops some acid.";
    break;

  case SPELL_CRIT_HIT:
    af.duration = 2;
    af.bitvector2 = AFF2_CRIT_HIT;
    accum_duration = FALSE;
    to_vict = "You bleed from a fatal wound.";
    to_room = "$n bleeds from a fatal wound.";
    break;

  case SPELL_ENH_HEAL:
    af.duration = 24;
    af.bitvector2 = AFF2_ENH_HEAL;
    to_vict = "You feel much healthier.";
    break;

  case SPELL_ENH_MANA:
    af.duration = 24;
    af.bitvector2 = AFF2_ENH_MANA;
    to_vict = "You feel much more intuned to the flow of mana.";
    break;

  case SPELL_ENH_MOVE:
    af.duration = 24;
    af.bitvector2 = AFF2_ENH_MOVE;
    to_vict = "You feel much more restful.";
    break;

  case SPELL_HASTE:
    af.duration = 2;
    af.bitvector2 = AFF2_HASTE;
    to_vict = "You feel yourself moving much faster.";
    break;

  
  /* end of spells */
  }
  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
   if (IS_NPC(victim) && (IS_AFFECTED(victim, af.bitvector|af2.bitvector) || 
	IS_SET(AFF2_FLAGS(victim), af.bitvector2|af2.bitvector2) || 
	IS_SET(AFF3_FLAGS(victim), af.bitvector3|af2.bitvector3)) &&
        !affected_by_spell(victim, spellnum)) {
     send_to_char(NOEFFECT, ch);
     return;
   }


  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  affect_join(victim, &af, accum_duration, FALSE, accum_affect, FALSE);
  if (af2.bitvector || af2.location)
    affect_join(victim, &af2, accum_duration, FALSE, accum_affect, FALSE);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void perform_mag_groups(int level, struct char_data * ch,
			struct char_data * tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL);
    break;
  case SPELL_GROUP_FLY:
    mag_affects(level, ch, tch, SPELL_FLY, savetype);
    break;
  case SPELL_GROUP_INVIS:
    mag_affects(level, ch, tch, SPELL_INVISIBLE, savetype);
    break;
  case SPELL_GROUP_PROT_EVIL:
    mag_affects(level, ch, tch, SPELL_PROT_FROM_EVIL, savetype);
    break;
  case SPELL_GROUP_WATBREATH:
    mag_affects(level, ch, tch, SPELL_WATERBREATH, savetype);
    break;
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && IS_AFFECTED(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, savetype);
  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(byte level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  char *to_char = NULL;
  char *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "You gesture and the earth begins to shake all around you!";
    to_room ="$n gracefully gestures and the earth begins to shake violently!";
    break;
  case SPELL_FIRE_BREATH:
    to_room ="With a draconian roar, $n breaths fire into the room!";
    to_char = "With a draconian roar, you toast everything in the room!";
    break;
  case SPELL_GAS_BREATH:
    to_room ="With a roar, $n fills the room with deadly vapors!";
    to_char ="You cast a deadly spell, filling the room with vapors from hell!";
    break;
  case SPELL_FROST_BREATH:
    to_room ="With a mighty roar, $n freezes everyone with a breath of ice!";
    to_char ="You roar, filling the room with a deadly stream of ice!";
    break;
  case SPELL_ACID_BREATH:
    to_room ="Spittle stream forth from the mouth of $n, eroding everything!";
    to_char ="You breath acid into the room!";
    break;
  case SPELL_LIGHTNING_BREATH:
    to_room ="$n roars, spraying bolts of electricity everywhere!";
    to_char ="You open your mouth and generate enough electricity to power Capitol for ten years!";
    break;
  case SPELL_CONE_OF_COLD:
    to_room ="$n launches a deadly cone of super-cooled air!";
    to_char ="You aim your cone of cold.";
    break;
  case SPELL_AREA_LIGHTNING:
    to_room ="With an almost fural scream, $n conjures a chain of lightning!";
    to_char ="You carefully flip on the switch to your UPS and focus the extra energy toward your enemies!";
    break;
  case SPELL_BLADEBARRIER:
    to_room ="$n laughs and attempts to put up a BLADE BARRIER!";
    to_char ="You concentrate on swords.  Swords. SWORDS!!!";
    break;
  }
  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);
  

  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     * players can only hit players in CRIMEOK rooms 4) players can only hit
     * charmed mobs in CRIMEOK rooms
     */

    if (tch == ch)
      continue;
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch) && !CAN_MURDER(ch, tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
      continue;

    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
  }
}

void mag_points(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int hit = 0;
  int move = 0;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    hit = dice(1, 8) + 1 + lvD2(ch);
    send_to_char("You feel better.\r\n", victim);
    break;
  case SPELL_CURE_SERIOUS:
    hit = dice(3, 8) + 3 + lvD2(ch);
    send_to_char("You feel much better!\r\n", victim);    
    break;
  case SPELL_CURE_CRITIC:
    hit = dice(8, 8) + 3 + lvD2(ch);
    send_to_char("You feel a lot better!\r\n", victim);
    break;
  case SPELL_HEAL:
    hit = 100 + dice(3, 8) + lvD2(ch);
    send_to_char("A warm feeling floods your body.\r\n", victim);
    break;
  case SPELL_DEATHDANCE:
    hit = 0 - (GET_MAX_HIT(victim) / 2);
    move = 0 - (GET_MAX_MOVE(victim) / 4);
    break;
  }
  GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
}


void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		        int spellnum, int type)
{
  int spell = 0;
  char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_BLIND:
  case SPELL_HEAL:
    spell = SPELL_BLINDNESS;
    to_vict = "Your vision returns!";
    to_room = "There's a momentary gleam in $n's eyes.";
    break;
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "You don't feel so unlucky.";
    break;
  default:
    sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
    log(buf);
    return;
    break;
  }

  if (!affected_by_spell(victim, spell)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
		         int spellnum, int savetype)
{
  char *to_char = NULL;
  char *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
	  (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
	to_char = "$p glows briefly.";
      }
      break;
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}


void mag_creations(int level, struct char_data * ch, int spellnum)
{
  struct obj_data *tobj;
  int z;

  if (ch == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 10;
    break;
  case SPELL_CREATE_WATER:
  default:
    send_to_char("Spell unimplemented, it would seem.\r\n", ch);
    return;
    break;
  }

  if (!(tobj = read_object(z, VIRTUAL, 0))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    log(buf);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}



/*
  Every spell which summons/gates/conjours a mob comes through here.

  None of these spells are currently implemented in Circle 3.0; these
  were taken as examples from the JediMUD code.  Summons can be used
  for spells like clone, ariel servant, etc.
*/

static char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
  "$n animates a corpse!\r\n",
  "$N appears from a cloud of thick blue smoke!\r\n",
  "$N appears from a cloud of thick green smoke!\r\n",
  "$N appears from a cloud of thick red smoke!\r\n",
  "$N disappears in a thick black cloud!\r\n"
  "As $n makes a strange magical gesture, you feel a strong breeze.\r\n",
  "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
  "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
  "As $n makes a strange magical gesture, you feel the dust swirl.\r\n",
  "$n magically divides!\r\n",
  "$n animates a corpse!\r\n"
};

#define MOB_MONSUM_I		116	/* lesser level */
#define MOB_MONSUM_II		10	/* regular */
#define MOB_MONSUM_III		10	/* greater */
#define MOB_MONSUM_IV		273	/* dragon  */
#define MOB_MONSUM_V		275	/* ancient */
#define MOB_GATE_I		10
#define MOB_GATE_II		10
#define MOB_GATE_III		10
#define MOB_ELEMENTAL_BASE	10
#define MOB_CLONE		10
#define MOB_ZOMBIE		1176
#define MOB_AERIALSERVANT	19


static char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh shit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};


void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		      int spellnum, int savetype)
{
  struct char_data *mob;
  struct obj_data *tobj, *next_obj;
  int pfail = 0;
  int msg = 0, fmsg = 0;
  int mob_num = 0;
  int change_mob = 0;
  int handle_corpse = 0;
  char mob_name[256];
  char mob_sdes[256];

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_ANIMATE_DEAD:
    if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
	(!GET_OBJ_VAL(obj, 3))) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    handle_corpse = 1;
    change_mob = 1;
    msg = 15;
    mob_num = MOB_ZOMBIE;
    pfail = 8;
    strcpy(mob_name, (obj)->short_description);
    strcpy(mob_sdes, (obj)->short_description);
    break;
  case SPELL_MONSUM_I:
	msg = 4;
	mob_num = MOB_MONSUM_I;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_MONSUM_II:
	msg = 4;
	mob_num = MOB_MONSUM_II;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_MONSUM_III:
	msg = 4;
	mob_num = MOB_MONSUM_III;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_MONSUM_IV:
	msg = 4;
	mob_num = MOB_MONSUM_IV;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_MONSUM_V:
	msg = 4;
	mob_num = MOB_MONSUM_V;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_CONJ_ELEMENTAL:
	msg = 4;
	mob_num = MOB_ELEMENTAL_BASE;
	pfail = 20;
	change_mob = 0;
	break;
  case SPELL_CLONE:
    mob_num = MOB_CLONE;
    pfail = 20;
    strcpy(mob_name, GET_NAME(ch));
    strcpy(mob_sdes, GET_NAME(ch));
    change_mob = 1;
    break;
  default:
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }
  if (number(0, 101) < pfail) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    return;
  }
  if (!(mob = read_mobile(real_mobile(mob_num), REAL, world[ch->in_room].zone))) {
	send_to_char("I seem to have goofed.\r\n", ch);
	sprintf(buf, "SYSERR: spell_summons, spell %d, mob %d: mob not found",
		spellnum, mob_num);
	log(buf);
	return;
  }
  char_to_room(mob, ch->in_room);
  IS_CARRYING_W(mob) = 0;
  IS_CARRYING_N(mob) = 0;
  SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
  SET_BIT(MOB_FLAGS(mob), MOB_PET);
  add_follower(mob, ch);
  act(mag_summon_msgs[fmsg], FALSE, ch, 0, mob, TO_ROOM);

  if (change_mob) {
	  mob->player.name = strdup(mob_name);
	  mob->player.short_descr = strdup(mob_sdes);
	  sprintf(mob_sdes, "%s is standing here.\r\n", mob_sdes);
	  mob->player.long_descr = strdup(mob_sdes);
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


