
#define MAX_ROOMS       400		/* Max rooms allocated */
#define MAX_HEIGHT      100		/* Max lines in map file */
#define MAX_WIDTH       150		/* Max width of map file lines */

#define NORTH           0
#define EAST            1
#define SOUTH           2
#define WEST            3
#define UP              4
#define DOWN            5

#define ABOVE		0
#define RIGHT		1
#define BELOW		2
#define LEFT		3

#define NOWHERE         -1
#define PATH_SKIP       1
#define PATH_UP         2
#define PATH_DOWN       4

/* Added to hold argument data v3.10 - 04/26/95 */
struct argrec {
	char mapfile[1024];
	long start_room;
	long default_flags;
} ;

struct roomdirrec {
	int doortype, key, toroom;
	char *name;
} ;

struct roomrec {
	char *name;
	int xcoord, ycoord;
	struct roomdirrec dir[6];
	long room_flags;
	int sector;
} ;

#define TYPE_NONE       -1
#define TYPE_ROOM       0
#define TYPE_DOOR       1
#define TYPE_PATH       2

struct arearec {
	char *name;
	long room_flags;
	int sector;
} ;

struct directionrec {
	int newdir;
	long bitvector;
} ;

struct pathrec {
	struct directionrec dir[4];
} ;

struct doorrec {
	char *name;
	int key, doortype;
} ;

struct tokenrec {
	int type;
	void *record;
} ;

