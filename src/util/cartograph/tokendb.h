

#define MAX_TOKEN       400
#define ARCHETYPE_FILE  "cartograph.arc"


void analyze_line(char *s, int linenum, char *fn);
void print_token_list();
void init_tokendb(char *arg0);
void free_all_tokens();
int get_token_room(int tokenid);
int get_token_path(int tokenid);
int get_token_door(int tokenid);
struct doorrec *get_door(int tokenid);
int find_new_direction(int tokenid, int fromdir);
long find_path_bitvector(int tokenid, int fromdir);
void get_room_data(int tokenid, struct roomrec *room);

