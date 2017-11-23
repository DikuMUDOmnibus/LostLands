
#include <stdlib.h>
#include <stdio.h>
#include "cartograph.h"
#include "utils.h"
#include "tokendb.h"
#include "parse.h"


void input_params(int argc, char *argv[], struct argrec *arg)
{
    char buf[100];

    if (argc > 1)
	strcpy(arg->mapfile, argv[1]);
    else {
	printf("Please enter the name of the drawing file:\n");
	gets(arg->mapfile);
    }
    if (argc > 2)
	arg->start_room = atol(argv[2]);
    else {
	printf("Please enter the virtual number of the first room:\n");
	scanf("%ld", &arg->start_room);
    }
    if (argc > 3)
	strcpy(buf, argv[3]);
    else {
	printf("Please enter the default room flag value:\n");
	scanf("%s", buf);
    }
    if ((arg->default_flags = decipher_flag_text(buf)) < 0) {
	error("Illegal default room flag\n");
	exit(1);
    }
}


void init(int argc, char *argv[], struct argrec *arg)
{

    init_tokendb(argv[0]);
    input_params(argc, argv, arg);
    init_map(arg->mapfile);
}


void shutdown()
{
/*    print_token_list();   */
    free_all_tokens();
}


int main(int argc, char *argv[])
{
    struct argrec arg;

    init(argc, argv, &arg);
    parse_map(arg.default_flags);
    save_map(arg.mapfile, arg.start_room);
    shutdown();
    return(0);
}
