/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
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
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct room_data *world;



/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

char *spells[] =
{
  "!RESERVED!",			/* 0 - reserved */

  /* SPELLS */

  "armor",			/* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "color spray",		/* 10 */
  "control weather",
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",		/* 20 */
  "detect poison",
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",		/* 30 */
  "locate object",
  "magic missile",
  "poison",
  "protect from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",			/* 40 */
  "ventriloquate",
  "word of recall",
  "remove poison",
  "sense life",
  "animate dead",
  "dispel good",
  "group armor",
  "group heal",
  "group recall",
  "infravision",		/* 50 */
  "waterwalk",
  "relocate",
  "peace",
  "fly",
  "levitate",			/* 55 */
  "protect from fire",
  "breath water",
  "group fly",
  "group invis",
  "group prot from evil",			/* 60 */
  "acid arrow",
  "flame arrow",
  "cure serious", 
  "minute meteor",
  "chain lightning",	/* 65 */
  "group breath water",
  "shield",
  "wraithform",
  "weaken",
  "stoneskin",	/* 70 */
  "cone of cold",
  "aid",
  "barkskin",
  "feast",
  "heros feast",       /* 75 */
  "blade barrier",	
  "conjure elemental",
  "summon lesser monster",
  "summon monster",
  "summon greater monster", /* 80 */
  "summon dragon",	
  "summon ancient dragon",
  "fear",
  "remove fear",
  "knock",	/* 85 */
  "hold person",
  "plane shift",
  "mirror image",
  "blink", 
  "haste",	/* 90 */
  "dispel magic",
  "death dance",
  "protect from cold",
  "!UNUSED!", "!UNUSED!",	/* 95 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 100 */
  "identify",
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath", /* 106 */
  "enhanced heal",
  "enhanced mana",	
  "enhanced move",
  "!UNUSED!",	/* 110 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 115 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 120 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 125 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 130 */

  /* SKILLS */

  "backstab",			/* 131 */
  "bash",
  "hide",
  "kick",
  "pick lock",
  "punch",
  "rescue",
  "sneak",
  "steal",
  "track",			/* 140 */
  "disarm",
  "break weapon",
  "second attack",
  "third attack",
  "enhanced damage",	/* 145 */
  "dual wield",
  "enhanced sight",
  "parry",
  "dodge",
  "fillet",	/* 150 */
  "cook",
  "camp",
  "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 155 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 160 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 165 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 170 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 175 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 180 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 185 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 190 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 195 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 200 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */

  "identify",
  "burning",
  "freezing",
  "dissolving",
  "bleeding",
  "enh_heal",
  "enh_mana",
  "enh_move",
  "\n",
  "\n"				/* the end */
};


struct syllable {
  char *org;
  char *new;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"bl", "nost"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"he", "alarc"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"peace", "pax"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}, {"1", "b"},
  {"2", "f"}, {"3", "t"}, {"4", "a"}, {"5", "j"}, {"6", "g"}, {"7", "i"}, {"8", "m"},
  {"9", "c"}, {"0", "p"},
};

int mag_manacost(struct char_data * ch, int spellnum)
{
  int mana, i, min_level;

  min_level = LVL_IMMORT;

  for (i = 0; i < NUM_CLASSES; i++) {
	if (IS_SET(GET_CLASS(ch), 1 << i) && (SINFO.min_level[i] < min_level))
		min_level = SINFO.min_level[i];
        }

  mana = MAX(SINFO.mana_max - (SINFO.mana_change *
		    (GET_LEVEL(ch) - min_level)),
	     SINFO.mana_min);
  return mana;
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
	            struct obj_data * tobj)
{
  char lbuf[256];

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
    else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
  else
    sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_RACE(ch) == GET_RACE(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    GET_RACE(ch) == GET_RACE(tch) ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}


int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_abbrev(name, spells[index]))
      return index;

    ok = 1;
    temp = any_one_arg(spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first, first2))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}

/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype)
{
  bool CAN_MURDER(struct char_data * ch, struct char_data * vict);
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)) {
    send_to_char("Your magic fizzles out and dies.\r\n", caster);
    act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }
  if ( cvict != NULL && ( IS_SET(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL) || !CAN_MURDER(caster, cvict) )
	&&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    send_to_char("A flash of white light fills the room, dispelling your "
		 "violent magic!\r\n", caster);
    act("White light from no particular source suddenly fills the room, "
	"then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
    act("Someone just tried to attack you with hostile magic!!!", FALSE, cvict, 0, 0, TO_CHAR);
    return 0;
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_TELEPORT:  	MANUAL_SPELL(spell_teleport); break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
    case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
    case SPELL_SUMMON:		MANUAL_SPELL(spell_summon); break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
    case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_PIDENTIFY:       MANUAL_SPELL(spell_identify); break;
    case SPELL_RELOCATE:	MANUAL_SPELL(spell_relocate); break;
    case SPELL_PEACE:		MANUAL_SPELL(spell_peace); break;
    }
  return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.
 *
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified.
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		          char *argument)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (ch == tch)
	  continue;
	if (GET_OBJ_VAL(obj, 0))
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     GET_OBJ_VAL(obj, 0), CAST_STAFF);
	else
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
		     DEFAULT_STAFF_LVL, CAST_STAFF);
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description != NULL)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION)))
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
	           struct obj_data * tobj, int spellnum)
{
  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("You dream about great magical powers.\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      break;
    }
    return 0;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("You are afraid you might hurt your master!\r\n", ch);
    return 0;
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char("You can only cast this spell upon yourself!\r\n", ch);
    return 0;
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("You cannot cast this spell upon yourself!\r\n", ch);
    return 0;
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("You can't cast this spell if you're not in a group!\r\n",ch);
    return 0;
  }
  send_to_char(OK, ch);
  say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, min_lev, target = 0;

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */

  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }
  min_lev = LVL_IMMORT;
  for (i = 0; i < NUM_CLASSES; i++)
	if (IS_SET(GET_CLASS(ch), 1 << i) && (SINFO.min_level[i] < min_lev) )
		min_lev = SINFO.min_level[i];
  if (GET_LEVEL(ch) < min_lev) {
    send_to_char("You do not know that spell!\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char("You are unfamiliar with that spell.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && !str_cmp(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101) > GET_SKILL(ch, spellnum)) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum, 0))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (mana > 0)
	GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
    }
  }
}


/* Assign the spells on boot up */

void spello(int spl, int mlev, int clev, int tlev, int flev, int blev, int slev, int dlev, int wlev, 
		 int olev, int alev, int nlev, int ylev, int xlev,
		 int spelltype,
		 int max_mana, int min_mana, int mana_change, int minpos,
	         int targets, int violent, int routines)
{
  spell_info[spl].min_level[0] = mlev;		/* class_magic_user */
  spell_info[spl].min_level[1] = clev;		/* class_cleric */
  spell_info[spl].min_level[2] = tlev;		/* class_thief */
  spell_info[spl].min_level[3] = flev;	/* class_fighter */
  spell_info[spl].min_level[4] = blev;	/* class_barbarian */
  spell_info[spl].min_level[5] = slev;	/* class_samurai */
  spell_info[spl].min_level[6] = dlev;	/* class_druid */
  spell_info[spl].min_level[7] = wlev;	/* class_wizard */
  spell_info[spl].min_level[8] = olev;	/* class_monk */
  spell_info[spl].min_level[9] = alev;	/* class_avatar */
  spell_info[spl].min_level[10] = nlev;	/* class_ninja */
  spell_info[spl].min_level[11] = ylev;		/* class_dual */
  spell_info[spl].min_level[12] = xlev;	/* class_triple */
  spell_info[spl].type = spelltype;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
}

/*
 * Arguments for spello calls:
 *
 * spellnum, levels (MCTFBSDW23), maxmana, minmana, manachng, minpos, targets,
 * violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL). levels  :  Minimum level (mage, cleric,
 * thief, warrior) a player must be to cast this spell.  Use 'X' for immortal
 * only. maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell). minmana :  The minimum
 * mana this spell will take, no matter how high level the caster is.
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|'). violent :  TRUE or FALSE,
 * depending on if this is considered a violent spell and should not be cast
 * in PEACEFUL rooms or on yourself. routines:  A list of magic routines
 * which are associated with this spell. Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

#define UU (LVL_IMPL+1)
#define UNUSED UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,0,0,0,0,0,0,0,0

#define X LVL_IMMORT

void mag_assign_spells(void)
{
  int i;

  for (i = 1; i <= TOP_SPELL_DEFINE; i++)
    spello(i, UNUSED);
		   /* C L A S S E S                M A N A   */
		   /* Ma  Cl  Th  Fi  Ba  Sa  Dr  Wi  Mo  Av  Ni  2  3  Type Max Min Chn */
  spello(SPELL_ARMOR,  1,  X,  X,  X,  X,  X,  X,  2,  X,  1,  X, X, X,
	 SKILL_TYPE_MAGIC,  30, 15,  3,
	 POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLESS,  X,  1,  X,  X,  5,  X,  1,  X, X, 1, X, X, X, 
	 SKILL_TYPE_CLERIC, 35, 5,  3,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_BLINDNESS, 6, X, X, X,  X,  X, X, 7, X, 1, 1, X, X, 
	 SKILL_TYPE_E_MAGIC, 35, 25, 1,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_BURNING_HANDS, 4, X, X, X, X, X, X, 4, 1, 1, X, X, X, 
         SKILL_TYPE_MAGIC, 30, 10, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, X, 9, X, X, 18, X, 9, X, 1, X, X, X, X,
	 SKILL_TYPE_MAGIC, 40, 25, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CHARM, 15, 14, X, X, X, X, 14, X, X, 1, X, X, X,
	 SKILL_TYPE_E_CLERIC | SKILL_TYPE_E_MAGIC, 75, 50, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CHILL_TOUCH, 4, X, X, X, X, X, X, 5, X, 1, X, X, X, 
	 SKILL_TYPE_E_MAGIC, 30, 10, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_COLOR_SPRAY, 5, X, X, X, X, X, X, X, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC, 30, 15, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CONTROL_WEATHER, 25, 24, X, X, X, X, 24, X, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 75, 25, 5,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_CREATE_FOOD, X, 2, X, X, X, X, 2, X, 2, X, X,  X, X,
	 SKILL_TYPE_CLERIC, 30, 5, 4,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, X, 2, X, X, X, X, 2, X, 2, X, X, X, X,
	 SKILL_TYPE_CLERIC, 30, 5, 4,
	 POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_CREATIONS);

  spello(SPELL_CURE_BLIND, X, 13, X, X, X, X, 12, X, 12, 1, X, X, X,
	 SKILL_TYPE_CLERIC, 30, 5, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_CURE_CRITIC, X, 20, X, X, X, X, 20, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC, 50, 20, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_LIGHT, X, 1, X, X, X, 10, 2, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC, 30, 10, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURSE, X, X, X, X, X, X, X, 13, X, X, 12, X, X,
	 SKILL_TYPE_E_MAGIC, 80, 50, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DETECT_ALIGN, 7, 7, X, X, X, X, X, X, X, 1, X, X, X,
   	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
		   /* Ma  Cl  Th  Fi  Ba  Sa  Dr  Wi  Mo  Av  Ni  2  3  Type Max Min Chn */
  spello(SPELL_DETECT_INVIS, 8, 16, X, X, X, X, 12, X, 6, 1, X, X, X,
	 SKILL_TYPE_MAGIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, 1, 1, X, X, X, X, 2, 2, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, X, 3, X, X, X, X, 1, X, 1, 1, 1, X, X,
	 SKILL_TYPE_CLERIC, 15, 5, 1,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DISPEL_EVIL, X, 19, X, X, X, X, 21, X, X, X, 10, X, X,
	 SKILL_TYPE_CLERIC, 40, 25, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DISPEL_GOOD, X, 19, X, X, X, X, 21, X, X, X, 10, X, X,
	 SKILL_TYPE_E_CLERIC, 40, 25, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, X, 29, X, X, X, X, 28, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC, 40, 25, 3,
	 POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ENCHANT_WEAPON, 17, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_MAGIC, 350, 250, 5,
	 POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_ENERGY_DRAIN, 18, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC, 40, 25, 1,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(SPELL_GROUP_ARMOR, X, X, X, X, X, X, X, 16, X, X, X, X, X,
	 SKILL_TYPE_MAGIC, 50, 30, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_FIREBALL, 11, X, X, X, X, X, X, X, X, X, 25, X, X,
	 SKILL_TYPE_MAGIC, 40, 30, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GROUP_HEAL, X, 28, X, X, X, X, 29, X, X, 5, X, X, X, 
	 SKILL_TYPE_CLERIC, 80, 60, 5,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_HARM, X, 22, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_CLERIC, 75, 45, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HEAL, X, 22, X, X, X, X, 22, X, X, 1, X, X, X,
	 SKILL_TYPE_CLERIC, 60, 40, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);

  spello(SPELL_INFRAVISION, 10, X, X, X, X, X, X, X, X, 1, 8, X, X,
	 SKILL_TYPE_MAGIC, 25, 10, 1,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INVISIBLE, 6, X, X, X, X, X, X, 8, X, 1, 3, X, X,
	 SKILL_TYPE_MAGIC, 35, 25, 1,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_LIGHTNING_BOLT, 10, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_MAGIC, 30, 15, 1,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LOCATE_OBJECT, 9, 12, X, X, X, X, 13, 12, X, 15, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 85, 70, 1,
	 POS_STANDING, TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_MAGIC_MISSILE, 1, X, X, X, X, X, X, 1, X, X, 8, X, X,
	 SKILL_TYPE_MAGIC, 25, 10, 3,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_POISON, X, X, X, X, X, X, X, X, X, X, 1, X, X, 
	 SKILL_TYPE_NONE, 50, 20, 3,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_PROT_FROM_EVIL, 3, 2, X, X, X, X, 3, 6, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 40, 10, 3,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_REMOVE_CURSE, 17, 15, X, X, X, X, 14, X, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 45, 25, 5,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SANCTUARY, X, 15, X, X, X, X, 18, X, X, 9, X, X, X,
	 SKILL_TYPE_CLERIC, 100, 75, 5,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SHOCKING_GRASP, 2, X, X, X, X, X, X, 3, X, X, 9, X, X,
	 SKILL_TYPE_MAGIC, 30, 15, 1,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SLEEP, 4, X, X, X, X, X, X, X, X, 1, X, X, X, 
	 SKILL_TYPE_MAGIC, 60, 25, 2,
	 POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_STRENGTH, 6, X, X, X, 1, X, X, X, 3, X, X, X, X,
 	 SKILL_TYPE_MAGIC, 35, 30, 1,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SUMMON, X, 16, X, X, X, X, X, 16, X, 8, X, X, X,
	 SKILL_TYPE_CLERIC, 75, 50, 3,
	 POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_WORD_OF_RECALL, X, 16, X, X, X, X, 16, X, X, 7, X, X, X, 
	 SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_REMOVE_POISON, X, 13, X, X, X, X, 12, X, X, 3, X, X, X,
	 SKILL_TYPE_CLERIC, 40, 8, 4,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SENSE_LIFE, X, 17, X, X, X, X, 17, X, X, 1, 4, X, X,
	 SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_PIDENTIFY, 20, 25, X, X, 29, X, X, X, X, 8, X, X, X,
 	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 75 , 30, 5,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_FIRE_BREATH, X, 21, X, X, X, X, 19, 13, X, 13, X, X, X,
	 SKILL_TYPE_E_MAGIC, 100, 50, 4,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS | MAG_AFFECTS);

  spello(SPELL_GAS_BREATH, X, X, X, X, X, X, X, 22, X, 22, X, X, X,
	 SKILL_TYPE_E_MAGIC, 100, 50, 3,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS);

  spello(SPELL_FROST_BREATH, X, X, X, X, X, X, X, 18, X, 18, 26, X, X,
	 SKILL_TYPE_E_MAGIC, 100, 50, 4,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS | MAG_AFFECTS);

  spello(SPELL_ACID_BREATH, X, X, X, X, X, X, X, 26, X, 26, X, X, X,
	 SKILL_TYPE_E_MAGIC, 100, 50, 3,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS | MAG_AFFECTS);

  spello(SPELL_LIGHTNING_BREATH, X, X, X, X, X, X, X, 11, X, 11, X, X, X,
	 SKILL_TYPE_E_MAGIC, 100, 50, 5,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS);

  spello(SPELL_RELOCATE, 23, 19, X, X, X, X, 21, X, X, 12, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 100, 70, 3,
         POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_PEACE, X, 14, X, X, 28, X, X, X, 12, 8, X, X, X,
	 SKILL_TYPE_CLERIC, 50, 50, 0,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

  spello(SPELL_FLY, 13, 18, X, X, X, X, 18, X, 16, 1, 12, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 50, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_LEVITATE, 9, 12, X, X, X, X, 11, X, 4, 1, 4, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 30, 15, 3,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_PROT_FIRE, X, 10, X, X, X, X, 10, 1, 1, 7, 12, X, X,
	 SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_WATERBREATH, 12, 12, X, X, X, X, 12, X, 3, 3, 3, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_FLY, 23, 28, X, X, X, X, 28, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 50, 10, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_INVIS, 13, X, X, X, X, X, X, 16, X, X, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_MAGIC, 80, 50, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_PROT_EVIL, X, 16, X, X, X, X, 16, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 50, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_ACID_ARROW, 8, X, X, X, X, X, X, X, X, 8, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_MAGIC, 30, 20, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_FLAME_ARROW, 12, X, X, X, X, X, X, X, X, 12, 12, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_MAGIC, 35, 20, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CURE_SERIOUS, X, 15, X, X, X, X, 15, X,  X, X, X, X, X,
         SKILL_TYPE_CLERIC, 40, 15, 2,
         POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_MINUTE_METEOR, 12, X, X, X, X, X, X, X, X, X, X, X, X,
         SKILL_TYPE_MAGIC, 45, 30, 2,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_AREA_LIGHTNING, 25, X, X, X, X, X, X, X, X, X, X, X, X,
         SKILL_TYPE_MAGIC, 60, 35, 3,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS);

  spello(SPELL_GROUP_WATBREATH, 20, X, X, X, X, X, X, X, X, X, X, X, X,
         SKILL_TYPE_MAGIC | SKILL_TYPE_MAGIC, 80, 50, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_SHIELD, 3, X, X, X, X, X, X, 4, X, 1, X, X, X,
         SKILL_TYPE_MAGIC, 35, 15, 2,
         POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_WRAITHFORM, X, X, X, X, X, X, X, 11, X, 6, X, X, X,
         SKILL_TYPE_MAGIC, 35, 15, 2,
         POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_WEAKEN, 9, X, X, X, X, X, X, X, X, X, X, X, X,
         SKILL_TYPE_MAGIC, 35, 25, 2,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_STONESKIN, 18,  X,  X,  X,  X,  X,  X,  X,  X, X, X, X, X,
         SKILL_TYPE_MAGIC,  80, 50,  3,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CONE_OF_COLD, 21, X, X, X, X, X, X, X, X, 8, X, X, X,
         SKILL_TYPE_E_MAGIC, 100, 60, 4,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AREAS | MAG_AFFECTS);

  spello(SPELL_AID,   X,  6,  X,  X,  X,  X,  6,  X,  X, X, X, X, X,
         SKILL_TYPE_CLERIC, 35, 10,  3,
         POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BARKSKIN,  X,  9,  X,  X,  X,  X,  8,  X,  X, X, X, X, X,
         SKILL_TYPE_CLERIC, 45, 10,  3,
         POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_FEAST, X, 11, X, X, X, X, 8, X,  5, 1, 5, X, X,
         SKILL_TYPE_CLERIC, 30, 5, 4,
         POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);
  
  spello(SPELL_FEAST_ALL, X, 18, X, X, X, X, 16, X,  X, 3, X, X, X,
         SKILL_TYPE_CLERIC, 35, 35, 4,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_AREAS);

  spello(SPELL_BLADEBARRIER,  X, 23, X, X, X, X, 23, X,  X, X, X, X, X,
         SKILL_TYPE_CLERIC, 50, 35, 4,
	 POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_CONJ_ELEMENTAL, 22, 24, X, X, X, X, X, 20, X, X, X, X, X,
         SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_MONSUM_I, 10, 14, X, X, X, X, 14, 10, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_MONSUM_II, 15, 18, X, X, X, X, 18, 14, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_MONSUM_III, 20, 22, X, X, X, X, 22, 18, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_MONSUM_IV, 24, 26, X, X, X, X, 26, 22, X, X,  X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_MONSUM_V, 28, X, X, X, X, X, X, 26, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
         POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_FEAR, 15, X, X, X, X, 7, X, X, X, X, X, X, X,
	 SKILL_TYPE_MAGIC, 30, 10, 2,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

  spello(SPELL_REM_FEAR, X, 4, X, X, X, X, 4, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC, 30, 10, 1,
         POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_MANUAL);

  spello(SPELL_KNOCK, 7, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_MAGIC, 40, 20, 2,
	 POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_HOLD_PERSON, 14, 10, X, X, X, X, 10, X, X, X, X, X, X,
	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 70, 40, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

  spello(SPELL_PLANESHIFT, X, 18, X, X, X, X, 18, X, X, 18, X, X, X,
	 SKILL_TYPE_CLERIC, 80, 40, 2,
         POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_MIRROR_IMAGE, 6, X, X, X, X, X, X, 8, X, X, 8, X, X,
	 SKILL_TYPE_MAGIC, 60, 40, 1,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_BLINK, 10, X, X, X, X, X, X, X, X, X, 10, X, X,
	 SKILL_TYPE_MAGIC, 50, 35, 1,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_HASTE, 13, X, X, X, X, X, X, X, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC, 50, 35, 1,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_DISPEL_MAGIC, 14, 11, X, X, X, X, 11, 14, X, 1, X, X, X,
	 SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 80, 40, 2,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV, FALSE, MAG_MANUAL);

  spello(SPELL_ANIMATE_DEAD, 23, 12, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC | SKILL_TYPE_E_CLERIC, 100, 80, 1,
	 POS_STANDING, TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

  spello(SPELL_CLONE, 29, X, X, X, X, X, X, X, X, X, 27, X, X,
	 SKILL_TYPE_MAGIC, 100, 100, 0,
	 POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_SUMMONS);

  spello(SPELL_DEATHDANCE, 21, X, X, 21, 22, 22, X, 23, 14, 14, 20, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC, 100, 100, 0,
	 POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS | MAG_POINTS);

  spello(SPELL_PROT_COLD, X, 10, X, X, X, X, 10, X, 1, 7, X, X, X,
	 SKILL_TYPE_CLERIC, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENH_HEAL, X, 25, X, X, X, X, 25, X, 20, 15, X, X, X,
	 SKILL_TYPE_E_MAGIC, 60, 30, 3,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENH_MANA, 25, X, X, X, X, X, X, 25, X, 15, X, X, X,
	 SKILL_TYPE_E_MAGIC, 60, 30, 3,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENH_MOVE, 25, 25, X, X, X, X, 25, 25, 20, 15, X, X, X,
	 SKILL_TYPE_E_MAGIC, 60, 30, 3,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  /*
   * SKILLS
   * 
   * The only parameters needed for skills are only the minimum levels for each
   * class.  The remaining 8 fields of the structure should be filled with
   * 0's.
   */

  /* Ma  Cl  Th  Wa  */
  spello(SKILL_BACKSTAB, X, X, 3, X, X, X, X, X, X, X, 1, X, X,
	 SKILL_TYPE_THIEF, 
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_BASH, X, X, X, 12, 10, X, X, X, X, 11, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_HIDE, X, X, 5, X, X, X, 11, X, X, X, 3, X, X,
	 SKILL_TYPE_THIEF,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_KICK, X, X, X, 1, 3, 2, X, X, 1, 1, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_PICK_LOCK, X, X, 2, X, X, X, X, X, X, X, 2, X, X,
	 SKILL_TYPE_THIEF,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_RESCUE, X, X, X, 3, 7, 6, X, X, X, 1, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_SNEAK, X, X, 1, X, X, X, X, 17, X, X, 1, X, X,
	 SKILL_TYPE_THIEF,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_STEAL, X, X, 4, X, X, X, X, X, X, X, 3, X, X,
	 SKILL_TYPE_THIEF,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_TRACK, X, X, 6, 9, 8, 18, 14, X, X, X, 4, X, X,
	 SKILL_TYPE_MUNDANE,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_DISARM, X, X, 10, 4, 5, 4, X, X, 3, 3, X, X, X,
	 SKILL_TYPE_MUNDANE,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_WEAPON_BREAK, X, X, X, 16, X, 14, X, X, X, X, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_SECOND_ATTACK, X, X, X, 5, 12, 5, X, X, 5, 12, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_THIRD_ATTACK, X, X, X, 20, X, X, X, X, 20, 24, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_ENH_DAMAGE, X, X, X, 15, X, 10, X, X, 10, X, X, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_DUAL_WIELD, X, X, X, 8, 10, 3, X, 20, X, X, X, X, X,
	 SKILL_TYPE_MUNDANE,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_ENH_SIGHT, 1, 1, X, 1, X, 1, X, 1, 1, 1, 1, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_PARRY, X, X, X, 1, X, 2, X, X, 3, 1, 8, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_DODGE, X, X, 4, 3, X, 4, X, X, X, 2, 10, X, X,
	 SKILL_TYPE_FIGHTER,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_FILLET, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_COOK, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SKILL_CAMP, X, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC,
	 0, 0, 0, 0, 0, 0, 0);

  spello(SPELL_IDENTIFY, 1, 13, 15, X, X, X, 11, 1, X, 1, X, X, X,
	 SKILL_TYPE_MUNDANE | SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 0, 0, 0,
	 0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_BURN, X, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC, 0, 0, 0,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_FREEZE, X, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC, 0, 0, 0,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ACID, X, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC, 0, 0, 0,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CRIT_HIT, X, X, X, X, X, X, X, X, X, X, X, X, X,
	 SKILL_TYPE_E_MAGIC, 0, 0, 0,
         0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);
}
