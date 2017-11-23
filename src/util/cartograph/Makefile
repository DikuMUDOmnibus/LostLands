#compiler name
CC = gcc

#compiler flags you want to use 
MYFLAGS = -g -Wall

#flags for profiling
PROFILE = 

#remove the hash mark below if compiling under IRIX
#LIBS = -lmalloc

#############################################################################

CFLAGS = $(MYFLAGS) $(PROFILE)

OBJFILES = parse.o tokendb.o utils.o main.o

cartograph : $(OBJFILES)


parse.o : parse.c cartograph.h utils.h tokendb.h parse.h
	$(CC) -c $(CFLAGS) parse.c
tokendb.o : tokendb.c cartograph.h utils.h tokendb.h
	$(CC) -c $(CFLAGS) tokendb.c
utils.o : utils.c utils.h
	$(CC) -c $(CFLAGS) utils.c
main.o : main.c cartograph.h utils.h tokendb.h parse.h
	$(CC) -c $(CFLAGS) main.c

cartograph :
	$(CC) -o cartograph $(PROFILE) $(OBJFILES) $(LIBS)

clean :
	rm *.o cartograph
