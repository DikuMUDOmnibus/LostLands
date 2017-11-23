
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cartograph.h"
#include "utils.h"
#include "tokendb.h"



struct tokenrec token[MAX_TOKEN];
char pathstr[] = "NARBL\0";
char bitstr[] = "SUD\0";


void free_record(int index)
{
   switch(token[index].type) {
      case TYPE_ROOM:
	 free((struct arearec *) token[index].record);
	 break;
      case TYPE_DOOR:
	 free(((struct doorrec *) token[index].record)->name);
	 free((struct doorrec *) token[index].record);
	 break;
      case TYPE_PATH:
	 free((struct pathrec *) token[index].record);
	 break;
   }
   token[index].type = TYPE_NONE;
   token[index].record = 0;
}


int ok_token(int tokenid, int linenum, char *fn)
{
   char buf[256];

   if (tokenid < 33) {
      sprintf(buf, "Illegal ASCII token %d\n", tokenid);
      warning(buf, linenum, fn);
      return(0);
   }

   if ((struct arearec *) token[tokenid].record) {
      sprintf(buf, "Redefinition of token '%c'\n", tokenid);
      warning(buf, linenum, fn);
      free_record(tokenid);
   }
   return(1);
}


void define_new_area(char *s, int linenum, char *fn)
{
   char ptr, buf[256], flagtext[100], *temp;
   long flags;
   int sector, i, tokenid;

   if (sscanf(s, "%s %c %s %d", buf, &ptr, flagtext, &sector) < 4) {
      warning("Missing arguments for #ROOM definition\n", linenum, fn);
      return;
   }

   /* Grab possible room name v3.10 - 04/26/95 */
   temp = s;
   for(i = 0; i < 4; i++) {
      while(*temp && !isspace(*temp))
	 temp++;
      while(isspace(*temp))
	 temp++;
   }

   if (*temp == ';')
      temp = 0;
   else {
      strcpy(buf, temp);
      if ((temp = strchr(buf, ';')))
	 *temp = 0;
      while(*buf && isspace(buf[strlen(buf) - 1]))
	 buf[strlen(buf) - 1] = 0;
      temp = buf;
   } 

   if (!ok_token(ptr, linenum, fn))
      return;

   tokenid = ptr;
   if ((flags = decipher_flag_text(flagtext)) < 0)
	warning("Illegal ASCII bitvector in #ROOM definition\n", linenum, fn);
   token[tokenid].type = TYPE_ROOM;
   token[tokenid].record = (struct arearec *) malloc(sizeof(struct arearec));

   /* Set up possible room name v3.10 - 04/26/95 */
   if (temp)
      ((struct arearec *) token[tokenid].record)->name = str_dup(temp);
   else
      ((struct arearec *) token[tokenid].record)->name = 0;
   ((struct arearec *) token[tokenid].record)->room_flags = flags;
   ((struct arearec *) token[tokenid].record)->sector = sector;
}


char *skip_spaces(char *ptr)
{
   while(isspace(*ptr))
      ptr++;
   return(ptr);
}


void define_new_path(char *s, int linenum, char *fn)
{
   struct pathrec *path;
   int index, startdir, tokenid;
   char buf[256], *cnt, *ptr;


   cnt = skip_spaces(s + strlen("#PATH"));

   if (!ok_token(tokenid = *cnt++, linenum, fn))
      return;

   token[tokenid].type = TYPE_PATH;
   token[tokenid].record = (struct pathrec *) malloc(sizeof(struct pathrec));
   path = (struct pathrec *) token[tokenid].record;
   for(index = ABOVE; index <= LEFT; index++) {
      path->dir[index].newdir = NOWHERE;
      path->dir[index].bitvector = 0;
   }

   cnt = skip_spaces(cnt);
   while ((*cnt != '\n') && (*cnt != '\r') && (*cnt)) {
      if (*cnt == ';')
	 break;
      if ((ptr = strchr(pathstr, *cnt)) == NULL) {
	 sprintf(buf, "Illegal direction %c in #PATH definition\n", *cnt);
	 warning(buf, linenum, fn);
	 free_record(tokenid);
	 break;
      }
      startdir = strlen(pathstr) - strlen(ptr) - 1;

      cnt = skip_spaces(++cnt);
      if (*cnt++ != '-') {
	 warning("Illegal format in #PATH definition\n", linenum, fn);
	 free_record(tokenid);
	 break;
      }

      cnt = skip_spaces(cnt);
      while (isalpha(*cnt)) {
	 if ((ptr = strchr(pathstr, *cnt)) != NULL) {
	    if (startdir != NOWHERE) {
	       if (path->dir[startdir].newdir != NOWHERE) {
		  sprintf(buf,"Direction %c redefined in #PATH statement\n",
		   pathstr[startdir + 1]);
		  warning(buf, linenum, fn);
	       }
	       path->dir[startdir].newdir = ptr - pathstr - 1;
	    }
	 } else if ((ptr = strchr(bitstr, *cnt)) != NULL) {
	    if (startdir != NOWHERE)
	       path->dir[startdir].bitvector |= (1 << (ptr - bitstr));
	 } else {
	    sprintf(buf, "Illegal character %c in #PATH definition\n", *cnt);
	    warning(buf, linenum, fn);
	    free_record(tokenid);
	 }
	 cnt++;
      }
      cnt = skip_spaces(cnt);
   }
}


void define_new_door(char *s, int linenum, char *fn)
{
   char *cnt, *ptr;
   int tokenid;
   struct doorrec *door;

   cnt = skip_spaces(s + strlen("#DOOR"));
   if (!ok_token(tokenid = *cnt++, linenum, fn))
      return;

   token[tokenid].type = TYPE_DOOR;
   token[tokenid].record = (struct doorrec *) malloc(sizeof(struct doorrec));
   door = (struct doorrec *) token[tokenid].record;
   cnt = skip_spaces(cnt);

   if (sscanf(cnt, "%d %d", &(door->doortype), &(door->key)) != 2) {
      warning("Missing parameters in #DOOR definition\n", linenum, fn);
      free_record(tokenid);
      return;
   }
   while (!isspace(*cnt))
      cnt++;
   cnt = skip_spaces(cnt);
   while (!isspace(*cnt))
      cnt++;
   cnt = skip_spaces(cnt);
   if ((ptr = strchr(cnt, ';')))
      *ptr = 0;
   if ((ptr = strchr(cnt, '~')))
      *ptr = 0;
   if ((ptr = strchr(cnt, '\n')))
      *ptr = 0;
   while (isspace(cnt[strlen(cnt) - 1]))
      cnt[strlen(cnt) - 1] = 0;
   door->name = str_dup(cnt);
}


void analyze_line(char *s, int linenum, char *fn)
{
   char cmd[256];
   char *ptr;

   if ((ptr = strchr(s, '\n')))
      *ptr = 0;
   strcpy(cmd, s);
   if (!(ptr = strtok(cmd, " \t")))
      return;

   if (!strcmp(ptr, "#ROOM"))
      define_new_area(s, linenum, fn);
   else if (!strcmp(ptr, "#PATH"))
      define_new_path(s, linenum, fn);
   else if (!strcmp(ptr, "#DOOR"))
      define_new_door(s, linenum, fn);
}


void read_token_types(char *arg0)
{
   FILE *fp;
   char s[256];
   int linenum = 0;

   if ((fp = fopen(ARCHETYPE_FILE, "rt")) == NULL) {
      /* Look at argv[0] to find archetype file v3.10 - 05/04/95 */
      strcpy(s, arg0);
      while(*s && (s[strlen(s) - 1] != '/'))
	s[strlen(s) - 1] = 0;
      strcat(s, ARCHETYPE_FILE);
      if ((fp = fopen(s, "rt")) == NULL) {
	error("Couldn't find startup file.\n\n");
	exit(1);
      }
   }

   while(fgets(s, 255, fp) != NULL)
      analyze_line(s, ++linenum, ARCHETYPE_FILE);

   fclose(fp);
}


void print_dir_string(int tokenid, int dir)
{
   char special[10];
   int index;

   *special = 0;
   for(index = 0; bitstr[index]; index++)
      if (((struct pathrec *) token[tokenid].record)->dir[dir].bitvector &
       (1 << index))
	 sprintf(special + strlen(special), "%c", bitstr[index]);

   if (((struct pathrec *) token[tokenid].record)->dir[dir].newdir != NOWHERE)
      printf("%c-%s%c  ", pathstr[dir + 1], special, pathstr[
       ((struct pathrec *) token[tokenid].record)->dir[dir].newdir + 1]);
}


void print_token_list()
{
   int index, dir;

   for(index = 0; index < MAX_TOKEN; index++)
      if ((struct arearec *) token[index].record) {
	 printf("%c: ", index);
	 switch(token[index].type) {
	    case TYPE_ROOM:
	       printf("Flags: %ld  Sector: %d\n",
		((struct arearec *) token[index].record)->room_flags,
		((struct arearec *) token[index].record)->sector);
	       break;
	    case TYPE_DOOR:
	       printf("Name %s  Type: %d  Key: %d\n",
		((struct doorrec *) token[index].record)->name,
		((struct doorrec *) token[index].record)->doortype,
		((struct doorrec *) token[index].record)->key);
	       break;
	    case TYPE_PATH:
	       for(dir = ABOVE; dir <= LEFT; dir++)
		  print_dir_string(index, dir);
	       printf("\n");
	       break;
	 }
      }
}


void init_tokendb(char *arg0)
{
   int index;

   for(index = 0; index < MAX_TOKEN; index++) {
      token[index].type = TYPE_NONE;
      token[index].record = 0;
   }
   read_token_types(arg0);
}


void free_all_tokens()
{
   int index;

   for(index = 0; index < MAX_TOKEN; index++)
      if ((struct arearec *) token[index].record)
	 free_record(index);
}


int get_token_room(int tokenid)
{
   if (token[tokenid].type != TYPE_ROOM)
      return(0);
   return(tokenid);
}


int get_token_path(int tokenid)
{
   if (token[tokenid].type != TYPE_PATH)
      return(0);
   return(tokenid);
}


int get_token_door(int tokenid)
{
   if (token[tokenid].type != TYPE_DOOR)
      return(0);
   return(tokenid);
}


struct doorrec *get_door(int tokenid)
{
   return((struct doorrec *) token[tokenid].record);
}


int find_new_direction(int tokenid, int fromdir)
{
   return(((struct pathrec *) token[tokenid].record)->dir[fromdir].newdir);
}


long find_path_bitvector(int tokenid, int fromdir)
{
   return(((struct pathrec *) token[tokenid].record)->dir[fromdir].bitvector);
}


void get_room_data(int tokenid, struct roomrec *room)
{
  room->name = ((struct arearec *) token[tokenid].record)->name;
  room->room_flags = ((struct arearec *) token[tokenid].record)->room_flags;
  room->sector = ((struct arearec *) token[tokenid].record)->sector;
}
