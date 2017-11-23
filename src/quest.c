/* ************************************************************************
*   File: quest.c				   A utility to CircleMUD *
*  Usage: writing/reading player's quest bits                             *
*                                                                         *
*  Code done by Billy H. Chan  				   		  *
*                                                                         *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "structs.h"
#include "class.h"
#include "utils.h"
#include "interpreter.h"

void send_to_char(char *messg, struct char_data *ch);
struct char_data *get_char_vis(struct char_data *ch, char *arg1);

void free_quest(struct quest * q)
{
	free(q);
}

void write_quests(struct char_data *ch)
{
  FILE *file;
  char fn[127];
  struct quest *temp;

  get_filename(GET_NAME(ch),fn,QUEST_FILE);
  unlink(fn);
  if( !GET_QUESTS(ch) )
    return;

  file = fopen(fn,"wt");

  temp = GET_QUESTS(ch);
  
  while( temp )
  {
    fwrite(temp, sizeof(struct quest), 1, file);
    temp = temp->next;
  }
  fclose(file);
}

void read_quests(struct char_data *ch)
{   
  FILE *file;
  char fn[127];
  struct quest *temp, *temp2;

  get_filename(GET_NAME(ch),fn,QUEST_FILE);

  file = fopen(fn,"r");

  if( !file )
    return;
 
  CREATE(GET_QUESTS(ch),struct quest,1);
  temp = GET_QUESTS(ch); 

  do 
  {  
    fread(temp, sizeof(struct quest), 1, file);
    if( !feof(file) ){
      CREATE(temp2,struct quest,1);
      temp->next = temp2;
      temp = temp->next;
    } 
  } while( !feof(file) ); 
  
  fclose(file);
} 

struct quest *find_quest(struct char_data * ch, int zone)
{
	struct quest * previous, *temp, * next = GET_QUESTS(ch);
	bool contloop = TRUE;

	previous = NULL;

	while ((next != NULL) && contloop) {
		if (next->zone < zone) {
			previous = next;
			next = next->next;
		}
		else if (next->zone == zone)
			return next;
		else
			contloop = FALSE;
	}
	CREATE(temp, struct quest, 1);
	temp->zone = zone;
	temp->firstrow = 0;
	temp->secondrow = 0;
	temp->next = next;
	if (previous) 
		previous->next = temp;
	else
		GET_QUESTS(ch) = temp;
	return temp;
}

questrow get_quest_bits(struct char_data * ch, int zone, int row, int offset, int length)
{
	struct quest * q;
	questrow temp, accum = 0;
	int i = 0;

	q = find_quest(ch, zone);

	if (row >= 2)
		temp = q->secondrow;
	else
		temp = q->firstrow;
	
	while (i < length) {
		if (IS_SET(temp, 1 << (offset + i)))
			SET_BIT(accum, 1 << i);
		i++;
	}
	return accum;
} 	

void change_quest_bits(struct char_data * ch, int zone, int row, int offset, int length, int value)
{
	struct quest * q;
	questrow loc;
	int i = 0;

	if (offset < 0)
		offset = 0;
	if (offset > 31)
		offset = 31;
	while ((offset + length) > 32) {
		length--;		
	}
	q = find_quest(ch, zone);

	if (row >= 2) 
		loc = q->secondrow;
	else
		loc = q->firstrow;

	while (i < length) {
		if (IS_SET(value, 1 << i))
			SET_BIT(loc, 1 << (offset + i));
		else
			REMOVE_BIT(loc, 1 << (offset + i));
		i++;
	}					
	if (row >= 2) 
		q->secondrow = loc;
	else
		q->firstrow = loc;
}

void add_quest_bits(struct char_data * ch, int zone, int row, int offset, int length, int value)
{
	struct quest * q;
	questrow loc;

	if (offset < 0)
		offset = 0;
	if (offset > 31)
		offset = 31;
	while ((offset + length) >= 32)
		length --;		

	q = find_quest(ch, zone);

	if (row >= 2)
		loc = q->secondrow;
	else
		loc = q->firstrow;

	if (value >= (1 << (length + 1)))
		value -= (1 << (length + 1));

	loc += (value << offset);

	if (row >= 2) 
		q->secondrow = loc;
	else
		q->firstrow = loc;
}


ACMD(do_showquest)
{
  int zone, i, j;
  struct char_data * victim;
  questrow temp;
  char buf[256], arg1[256], arg2[256];

  if (!*argument) {
    strcpy(buf, "Showquest usage: showquest <player> <zone>\r\n");
    send_to_char(buf, ch);
    return;
  }

  strcpy(buf, two_arguments(argument, arg1, arg2));
  if (!*arg2) {
    strcpy(buf, "Showquest usage: showquest <player> <zone>\r\n");
    send_to_char(buf, ch);	
    return;
  }

  victim = get_char_vis(ch, arg1);

  zone = atoi(arg2);

  sprintf(buf, "%-35.35s01234567890123456789012345678901\r\n", GET_NAME(victim));
  send_to_char(buf, ch);
  for (j = 1; j <= 2; j++) {
     temp = get_quest_bits(victim, zone, j, 0, 32);
     sprintf(buf, "Questbits for Zone %d in Row %d is: ", zone, j);
     sprintf(buf, "%-35.35s", buf);
     for (i = 0; i < 32; i++) {
	  if (temp & 1)
		  sprintf(buf, "%s1", buf);
	  else
		  sprintf(buf, "%s0", buf);
	  temp = temp >> 1;
     }
     strcat(buf, "\r\n");
     send_to_char(buf, ch);
  }
}

ACMD(do_setquest)
{
  int zone, offset, length, row;
  long value;
  struct char_data * victim;
  questrow temp;
  char buf[256], arg1[256];

  if (!*argument) {
    if (subcmd == SCMD_SET_QUEST_BIT)
	    strcpy(buf, "setquest usage: setquest <player> <zone> <row> <offset> <length> <value>\r\n");
    else
	    strcpy(buf, "addquest usage: addquest <player> <zone> <row> <offset> <length> <value>\r\n");
    send_to_char(buf, ch);
    return;
  }

  half_chop(argument, arg1, argument);
  victim = get_char_vis(ch, arg1);

  half_chop(argument, arg1, argument);
  zone = atoi(arg1);
  if (!*arg1 || (zone < 0) || (zone > 500)) {
    strcpy(buf, "zone must be a number between 0 and 500");
    send_to_char(buf, ch);
    return;	
  }

  half_chop(argument, arg1, argument);
  row = atoi(arg1);
  if (!*arg1 || (row < 0) || (row > 2)) {
    strcpy(buf, "row must be a number between 1 and 2");
    send_to_char(buf, ch);
    return;		
  }

  half_chop(argument, arg1, argument);
  offset = atoi(arg1);
  if (!*arg1 || (offset < 0) || (offset > 31)) {
    strcpy(buf, "offset must be a number between 0 and 31");
    send_to_char(buf, ch);
    return;		
  }

  half_chop(argument, arg1, argument);
  length = atoi(arg1); 
  if (!*arg1 || (length < 0) || (length > 32)) {
    strcpy(buf, "length must be a number between 1 and 32");
    send_to_char(buf, ch);
    return;		
  }

  if (length + offset > 32) {
	strcpy(buf, "length and offset must not exceed 32");
	send_to_char(buf, ch);
	return;
  }

  half_chop(argument, arg1, argument);
  if (!*arg1) {
    strcpy(buf, "a value must be given");
    send_to_char(buf, ch);
    return;		
  }
  value = atol(arg1); 

  if (subcmd != SCMD_SET_QUEST_BIT) {
	  temp = get_quest_bits(victim, zone, row, offset, length);
	  if (value >= 0)
		value += temp;
	  else
	        value = temp & ~value;
	}
  change_quest_bits(victim, zone, row, offset, length, value);
  send_to_char("Quest bit has been modified.\r\n", ch);
}
