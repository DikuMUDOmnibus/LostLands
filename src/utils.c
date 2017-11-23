/* ************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/telnet.h>
#include <netinet/in.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"

extern struct time_data time_info;
extern struct room_data *world;
extern struct char_data *character_list;

unsigned long my_rand(void);


/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  int i = 0;
  if (from > to) {
	i = from;
	from = to;
	to = i;
  }
  to = MAX(to, 1);
  from = MAX(from, 0);
  i = ((my_rand() % (to - from + 1)) + from);
  return i;
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  if (size >= 20)
    size = 20;
  if (number >= 10)
    number = 10;
  while (number-- > 0)
    sum += ((my_rand() % size) + 1);

  return sum;
}


int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}



/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}



/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
      if (chk < 0)
	return (-1);
      else
	return (1);
  return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
      if (chk < 0)
	return (-1);
      else
	return (1);

  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];
  extern struct room_data *world;

  sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
	  world[ch->in_room].number, world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}


/* writes a string to the log */
void log(char *str)
{
  time_t ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%-19.19s :: %s\n", tmstr, str);
}


/* the "touch" command, essentially. */
int touch(char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    perror(path);
    return -1;
  } else {
    fclose(fl);
    return 0;
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(char *str, char type, sbyte level, byte file)
{
  char buf[256];
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING)) {
      tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

      if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}



void sprintbit(long vektor, char *names[], char *result)
{
  unsigned long nr;

  *result = '\0';

  if (vektor < 0) {
    strcpy(result, "SPRINTBIT ERROR!");
    return;
  }
  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcat(result, "NONE ");
}

void sprinttype(int type, char *names[], char *result)
{
  int nr;

  for (nr = 0; (*names[nr] != '\n'); nr++);
  if (type < nr)
    strcpy(result, names[type]);
  else
    strcpy(result, "UNDEFINED");
}

char * sprintbitascii(long vektor, char *result)
{
  unsigned long nr;
  int temp = 0;

  *result = '\0';

  for (nr = 0, temp = 0; nr < 32; nr++) {
    if (IS_SET(vektor, 1 << nr )) {
	if (nr < 26)
		result[temp++] = (char) ('a' + nr);
	else
		result[temp++] = (char) ('A' + nr - 26);
    }
  }
  result[temp] = '\0';
  if (!temp) {
    strcpy(result, "0");
  }
  return result;
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return now;
}



struct time_info_data age(struct char_data * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  return player_age;
}




/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  void make_corpse(struct char_data *ch);
  struct follow_type *j, *k;

  assert(ch->master);

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {			/* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;

}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;
  struct char_data *vict;

  void make_corpse(struct char_data *ch);

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    vict = k->follower;
    stop_follower(vict);
    if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_PET)) {
	die_follower(ch);
	act("Without a master, $n perishes!", FALSE, ch, 0, 0, TO_ROOM);
	make_corpse(ch);
	extract_char(ch);
    } else if (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PET)) {
	die_follower(vict);
	act("Without a master, $n perishes!", FALSE, vict, 0, 0, TO_ROOM);
	make_corpse(vict);
	extract_char(vict);
    }
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}

int get_line2(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (!*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) {
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  case ALIAS_FILE:
    prefix = "plralias";
    suffix = "alias";
    break;
  case QUEST_FILE:
    prefix = "plrquest";
    suffix = "quest";
    break;
  default:
    return 0;
    break;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }
  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}

bool CAN_MURDER(struct char_data * ch, struct char_data * victim) {
	if ( IS_NPC(ch) || IS_NPC(victim) )
		return TRUE;

/*  article 6 */
        if ( PLR_FLAGGED(victim, PLR_KILLER) )
		return TRUE;
/*  article 5 */
	if ( PRF2_FLAGGED(ch, PRF2_ASSASSIN) && (PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI) ||
		PRF2_FLAGGED(ch, PRF2_WAR_YLLANTRA)) )
		return TRUE;
/*  article 1 */
        if ( PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI) && PRF2_FLAGGED(victim, PRF2_WAR_YLLANTRA)
		&& !(PRF2_FLAGGED(ch, PRF2_RETIRED) || PRF2_FLAGGED(victim, PRF2_RETIRED)) )
		return TRUE;
        if ( PRF2_FLAGGED(victim, PRF2_WAR_DRUHARI) && PRF2_FLAGGED(ch, PRF2_WAR_YLLANTRA)
		&& !(PRF2_FLAGGED(ch, PRF2_RETIRED) || PRF2_FLAGGED(victim, PRF2_RETIRED)) )
		return TRUE;

/* article 2 */
        if ( PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT) && PRF2_FLAGGED(victim, PRF2_ASSASSIN) )
		return TRUE;
        if ( PRF2_FLAGGED(victim, PRF2_BOUNTYHUNT) && PRF2_FLAGGED(ch, PRF2_ASSASSIN) )
		return TRUE;

/* article 3 */
	if ( PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT) && PLR_FLAGGED(victim, PLR_THIEF) )
		return TRUE;

/* article 4 */
	if ( (IS_GOODGNOME(ch) && IS_DARKGNOME(victim)) ||
	     (IS_GOODGNOME(victim) && IS_DARKGNOME(ch)) )
		return TRUE;

        return FALSE;
}	
		
bool CAN_DAMAGE(struct char_data *ch, struct char_data * vic)
{
   struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
   int affectlevel = 0;

   /* 	affectlevel:   0  == none
	affectlevel:   1  == silver
	affectlevel:   2  == plus 1
	affectlevel:   3  == plus 2
	affectlevel:   4  == plus 3
	affectlevel:   5  == plus 4
	affectlevel:   6  == plus 5
   */

   if (IS_MONK(ch)) {
	affectlevel = MAX(0, lvD3(ch) - 4);
   }
   else if (wielded) {
	if (IS_OBJ_STAT(wielded, ITEM_PLUSFIVE))
		affectlevel = 6;
	else if (IS_OBJ_STAT(wielded, ITEM_PLUSFOUR))
		affectlevel = 5;
	else if (IS_OBJ_STAT(wielded, ITEM_PLUSTHREE))
		affectlevel = 4;
	else if (IS_OBJ_STAT(wielded, ITEM_PLUSTWO))
		affectlevel = 3;
	else if (IS_OBJ_STAT(wielded, ITEM_PLUSONE))
		affectlevel = 2;
	else if (IS_OBJ_STAT(wielded, ITEM_SILVER))
		affectlevel = 1;
        if (IS_SAMURAI(ch) || IS_NINJA(ch))
		affectlevel += MAX(0, lvD10(ch) - 2);
   } else if (IS_MOB(ch)) {
	if (AFF_FLAGGED(ch, AFF_PLUSFIVE))
		affectlevel = 6;
	else if (AFF_FLAGGED(ch, AFF_PLUSFOUR))
		affectlevel = 5;
	else if (AFF_FLAGGED(ch, AFF_PLUSFOUR))
		affectlevel = 4;
	else if (AFF_FLAGGED(ch, AFF_PLUSTHREE))
		affectlevel = 3;
	else if (AFF_FLAGGED(ch, AFF_PLUSTWO))
		affectlevel = 2;
	else if (AFF_FLAGGED(ch, AFF_PLUSONE))
		affectlevel = 1;
   }

   if (AFF_FLAGGED(vic, AFF_PLUSFIVE))
	affectlevel -= 6;
   else if (AFF_FLAGGED(vic, AFF_PLUSFOUR))
	affectlevel -= 5;
   else if (AFF_FLAGGED(vic, AFF_PLUSTHREE))
	affectlevel -= 4;
   else if (AFF_FLAGGED(vic, AFF_PLUSTWO))
	affectlevel -= 3;
   else if (AFF_FLAGGED(vic, AFF_PLUSONE))
	affectlevel -= 2;
   else if (AFF_FLAGGED(vic, AFF_SILVER))
	affectlevel -= 1;

   return (affectlevel >= 0);
}


int GET_REINCARN(struct char_data *ch)
{
   int temp = 0;

   if (PRF2_FLAGGED(ch, PRF2_REINCARN1))
	temp += 1;
   if (PRF2_FLAGGED(ch, PRF2_REINCARN2))
	temp += 2;
   if (PRF2_FLAGGED(ch, PRF2_REINCARN3))
	temp += 4;

   return (temp + 1);
}

void ADD_REINCARN(struct char_data *ch)
{
   if (GET_REINCARN(ch) >= 8)
	return;

   if (!PRF2_FLAGGED(ch, PRF2_REINCARN1))
	SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN1);
   else {
	REMOVE_BIT(PRF2_FLAGS(ch), PRF2_REINCARN1);
	if (!PRF2_FLAGGED(ch, PRF2_REINCARN2))
		SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN2);
	else {
		REMOVE_BIT(PRF2_FLAGS(ch), PRF2_REINCARN2);
		SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN3);
	}
   }
}

int get_alignment_type(const int temp) {
	if (temp <= -2500)
		return 0;
	else if (temp <= -2000)
		return 1;
	else if (temp <= -1500)
		return 2;
	else if (temp <= -1000)
		return 3;
	else if (temp <= -500)
		return 4;
	else if (temp < 500)
		return 5;
	else if (temp < 1000)
		return 6;
	else if (temp < 1500)
		return 7;
	else if (temp < 2000)
		return 8;
	else if (temp < 2500)
		return 9;
	else 
		return 10;
}
