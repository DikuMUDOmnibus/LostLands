
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "cartograph.h"
#include "utils.h"
#include "tokendb.h"
#include "parse.h"


struct roomrec graph[MAX_ROOMS];
char map[MAX_HEIGHT + 2][MAX_WIDTH + 3];
int numlines = 0;
int numrooms = 0;

int oppositedir[4] = { BELOW, LEFT, ABOVE, RIGHT };
int dirxmod[4] = { 0, 1, 0, -1 };
int dirymod[4] = { -1, 0, 1, 0 };
char dirstr[] = "NESWUD";


void check_special(char *up, char *down, int *x, int *y, int dir)
{
    char upslide = 0, downslide = 0;
    long vector;

    vector = find_path_bitvector(map[*y][*x], oppositedir[dir]);
    if (vector & PATH_SKIP) {
	*x += dirxmod[dir];
	*y += dirymod[dir];
    }
    if (vector & PATH_UP)
	upslide = 1;
    else if (vector & PATH_DOWN)
	downslide = 1;

    if (upslide) {
	if (*down)
	    *down = 0;
	else
	    *up = 1;
    } else if (downslide) {
	if (*up)
	    *up = 0;
	else
	    *down = 1;
    }
}


int ok_location(int x, int y)
{
    if ((y > 0) && (x > 0) && (y < numlines) && (x < strlen(map[y])))
	return(1);
    return(0);
}


int follow_path(int *xl, int *yl, int *door, int dir)
{
    char up = 0, down = 0;
    char done = 0;
    int move, x, y, temp;

    x = *xl + dirxmod[dir];
    y = *yl + dirymod[dir];
    move = dir;
    *xl = *yl = -1;

    while (!done) {
	if (!ok_location(x, y))
	    return(dir);
	if (get_token_room(map[y][x])) {
	    *xl = x;
	    *yl = y;
	    done = 1;
	} else if ((temp = get_token_path(map[y][x]))) {
	    if ((move = find_new_direction(temp, oppositedir[move])) == NOWHERE)
		done = 1;
	    else {
		check_special(&up, &down, &x, &y, move);
		x += dirxmod[move];
		y += dirymod[move];
	    }
	} else if ((temp = get_token_door(map[y][x]))) {
	    if (!*door)
		*door = temp;
	    x+= dirxmod[move];
	    y+= dirymod[move];
	} else
	    done = 1;
    }

    if (up)
	return(UP);
    else if (down)
	return(DOWN);
    else
	return(dir);
}


void init_map(char *mapfile)
{
    int index, curline, linenum;
    FILE *fp;
    char buf[MAX_WIDTH + 1];
    char *comment;

    if ((fp = fopen(mapfile, "rt")) == NULL) {
	sprintf(buf, "Input file \"%s\" not found!\n", mapfile);
	error(buf);
	exit(1);
    }

    curline = linenum = 0;
    while((fgets(buf, MAX_WIDTH, fp) != NULL) && (curline <= MAX_HEIGHT)) {
	linenum++;
	comment = buf;
	while (isspace(*comment))
	    comment++;
	if (*comment == '#')
	    analyze_line(buf, linenum, mapfile);
	else {
	    /* BugFix v2.01 - 05/05/94 */
	    if ((comment = strchr(buf, '\n')) != NULL)
		*comment = 0;
	    if ((comment = strchr(buf, ';')) != NULL)
		*comment = 0;
	    sprintf(map[++curline], " %s ", buf);
	}
    }
    fclose(fp);

    if (curline > MAX_HEIGHT) {
	error("Input file too long - Raise MAX_HEIGHT constant in parse.c\n");
	exit(1);
    }

    for(index = 0; index < MAX_WIDTH; index++) {
	map[0][index] = ' ';
	map[curline + 1][index] = ' ';
    }
    numlines = curline + 1;
}


void identify_rooms(long def_flags)
{
    int index, index2, index3, roomtype, errorcnt = 0;
    char s[256];

    for(index = 1; index < numlines; index++)
	for(index2 = 1; index2 < strlen(map[index]); index2++)
	    if ((roomtype = get_token_room(map[index][index2]))) {
		/* Sanity Check v3.01 - 11/14/94 */
		if (numrooms >= MAX_ROOMS) {
		    errorcnt++;
		    continue;
		}
		graph[numrooms].xcoord = index2;
		graph[numrooms].ycoord = index;
		get_room_data(map[index][index2], &graph[numrooms]);
		graph[numrooms].room_flags |= def_flags;
		for(index3 = NORTH; index3 <= DOWN; index3++) {
		    graph[numrooms].dir[index3].toroom = NOWHERE;
		    graph[numrooms].dir[index3].doortype = 0;
		    graph[numrooms].dir[index3].key = -1;
		    graph[numrooms].dir[index3].name = 0;
		}
		numrooms++;
	    }
    /* Sanity Check v3.01 - 11/14/94 */
    if (errorcnt) {
	sprintf(s, "Too many rooms - Map truncated.\n       "
	    "Raise MAX_ROOMS in cartograph.h to %d.\n", errorcnt + MAX_ROOMS);
	error(s);
    }
}


void connect_rooms()
{
    int curroom, dir, finaldir, dest, x, y, doornum;
    struct doorrec *door;

    for(curroom = 0; curroom < numrooms; curroom++)
	for(dir = NORTH; dir <= WEST; dir++) {
	    x = graph[curroom].xcoord;
	    y = graph[curroom].ycoord;
	    doornum = 0;
	    finaldir = follow_path(&x, &y, &doornum, dir);
	    for(dest = 0; dest < numrooms; dest++) {
		if ((graph[dest].xcoord == x) && (graph[dest].ycoord == y))
		    graph[curroom].dir[finaldir].toroom = dest;
		if (doornum) {
		    door = get_door(doornum);
		    graph[curroom].dir[finaldir].name = str_dup(door->name);
		    graph[curroom].dir[finaldir].doortype = door->doortype;
		    graph[curroom].dir[finaldir].key = door->key;
		}
	    }
	}
}


void parse_map(long def_flags)
{
    identify_rooms(def_flags);
    connect_rooms();
}


void save_map(char *mapfile, int offset)
{
    FILE *fp;
    int index, dir;
    char worldfile[256], buf[256];

    sprintf(worldfile, "%s.wld", mapfile);
    if ((fp = fopen(worldfile, "wt")) == NULL) {
	sprintf(buf, "Can not write to world file \"%s\"\n", worldfile);
	error(buf);
	exit(1);
    }

    /* Added optional room name v3.10 - 04/26/95 */
    for(index = 0; index < numrooms; index++) {
	fprintf(fp, "#%d\n", index + offset);
	if (graph[index].name)
	    fprintf(fp, "%s~\n", graph[index].name);
	else
	    fprintf(fp, "Room #%d~\n", index + offset);
	fprintf(fp, "This description is yet unfinished...\n");
	fprintf(fp, "~\n");
	fprintf(fp, "%d %ld %d\n", offset / 100,
	    graph[index].room_flags, graph[index].sector);
	printf("#: %3d   X:%4d   Y:%4d", index + offset, graph[index].xcoord,
	    graph[index].ycoord);
	for(dir = 0; dir < 6; dir++) {
	    if (graph[index].dir[dir].toroom > NOWHERE)
		graph[index].dir[dir].toroom += offset;
		printf("   %c:%4d", dirstr[dir], graph[index].dir[dir].toroom);
		if (graph[index].dir[dir].toroom > NOWHERE) {
		    fprintf(fp, "D%d\n~\n", dir);
		    if (graph[index].dir[dir].name)
			fprintf(fp, "%s", graph[index].dir[dir].name);
		    fprintf(fp, "~\n%d %d %d\n",
			graph[index].dir[dir].doortype,
			graph[index].dir[dir].key,
			graph[index].dir[dir].toroom);
		}
	}
	fprintf(fp, "S\n");
	printf("\n");
    }
    fclose(fp);
}
