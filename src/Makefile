# CircleMUD 3.0 Makefile
#
# $Id: Makefile,v 1.2 1995/10/30 19:39:16 mcoletti Exp $
#
# This makefile should work with no changes for compiling under SunOS, Ultrix,
# NeXTSTEP, HP/UX, DG/UX, and Linux.  If you're using AIX, IRIX, some version
# of SVR4 or Solaris, uncomment the appropriate lines below.  If you're using
# some other operating system not mentioned, just type 'make' and cross your
# fingers..
#
#	You MUST ``make depend'' first to automatically set up the inter-
#	file dependencies. <1>
#
# Notes:
#	o makefile now dynamically builds dependency lists <1>
#	o makefile complexity reduced by relying on implicit pattern matching
#	  rules instead of explicit production rules
#	o consolidated like macro definitions to reduce complexity
#	o re-formatted file lists to make easier to read
# 
# To Do:
#	o need to set up CircleMUD to work with GNU autoconf utility to
#	  build this makefile so that fingers don't have to be crossed
#	  as much   =8-)
#
# ---
# <1>	you must have ``makedepend'' installed somewhere on your path, natch
#

SHELL    = /bin/sh
INCLUDES = -I.
BINDIR   = ../bin

CC       = gcc

# for AIX
# CC = cc
# for IRIX
# CC = cc -cckr -fullwarn


CFLAGS   = -ggdb3 -Wall -O

# for AIX
# CFLAGS = -g
# for IRIX
# CFLAGS = -g



# LIBS     = -lsocket -lnsl

# SVR4 or Solaris
# LIBS = -lsocket -lnsl
# IRIX
# LIBS = -lmalloc


DEFS     =


# flags for profiling (see hacker.doc for more information)
#
# (just add this to the implicit rule below if you want profiling)

PROFILE = -pg



# implicit rule for building object files

%.o : %.c
	$(CC) -c $(CFLAGS) $(DEFS) $(INCLUDES) $< 


##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

HDRS = 	boards.h				\
	comm.h					\
	db.h					\
	handler.h				\
	house.h					\
	interpreter.h				\
	mail.h					\
	olc.h					\
	screen.h				\
	shop.h					\
	spells.h				\
	structs.h				\
	utils.h


SRCS =  act.comm.c				\
	act.informative.c			\
	act.movement.c				\
	act.obj.c				\
	act.offensive.c				\
	act.other.c				\
	act.social.c				\
	act.wizard.c				\
	ban.c					\
	boards.c				\
	class.c					\
	comm.c					\
	config.c				\
	constants.c				\
	db.c					\
	fight.c					\
	graph.c					\
	handler.c				\
	house.c					\
	interpreter.c				\
	limits.c				\
	magic.c					\
	mail.c					\
	mobact.c				\
	modify.c				\
	objsave.c				\
	olc.c					\
	random.c				\
	shop.c					\
	spec_assign.c				\
	spec_procs.c				\
	spell_parser.c				\
	spells.c				\
	utils.c					\
	weather.c				\
	alias.c					\
	mobcmd.c				\
	mobprog.c				\
	iedit.c					\
	redit.c					\
	zedit.c					\
	quest.c					\
	teleport.c		

OBJS = 	comm.o 					\
	act.comm.o 				\
	act.informative.o 			\
	act.movement.o 				\
	act.obj.o				\
	act.offensive.o 			\
	act.other.o 				\
	act.social.o 				\
	act.wizard.o 				\
	ban.o 					\
	boards.o				\
	class.o 				\
	config.o 				\
	constants.o 				\
	db.o 					\
	fight.o 				\
	graph.o 				\
	handler.o				\
	house.o 				\
	interpreter.o 				\
	limits.o 				\
	magic.o 				\
	mail.o 					\
	mobact.o 				\
	modify.o				\
	objsave.o 				\
	olc.o 					\
	random.o 				\
	shop.o 					\
	spec_assign.o 				\
	spec_procs.o				\
	spell_parser.o 				\
	spells.o 				\
	utils.o 				\
	weather.o				\
	alias.o					\
	mobcmd.o				\
	mobprog.o				\
	iedit.o					\
	redit.o					\
	zedit.o					\
	quest.o					\
	teleport.o

# duh rules

.PHONY: all
all : .accepted $(BINDIR)/circle utils


# silly license document that must be viewed at least once before building

.accepted:
	@./licheck


.PHONY:	utils
utils : .accepted $(BINDIR)/autowiz $(BINDIR)/delobjs $(BINDIR)/listrent $(BINDIR)/mudpasswd $(BINDIR)/purgeplay $(BINDIR)/showplay $(BINDIR)/sign $(BINDIR)/split


$(BINDIR)/circle : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(BINDIR)/circle.new

$(BINDIR)/autowiz : util/autowiz.c structs.h db.h utils.h
	$(CC) $(CFLAGS) -o $(BINDIR)/autowiz util/autowiz.c

$(BINDIR)/delobjs : util/delobjs.c structs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/delobjs util/delobjs.c

$(BINDIR)/listrent : util/listrent.c structs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/listrent util/listrent.c

$(BINDIR)/mudpasswd   : util/mudpasswd.c structs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/mudpasswd util/mudpasswd.c

$(BINDIR)/purgeplay : util/purgeplay.c structs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/purgeplay util/purgeplay.c

$(BINDIR)/showplay : util/showplay.c structs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/showplay util/showplay.c

$(BINDIR)/sign: util/sign.c
	$(CC) $(CFLAGS) -o $(BINDIR)/sign $(LIBS) util/sign.c

$(BINDIR)/split: util/split.c
	$(CC) $(CFLAGS) -o $(BINDIR)/split util/split.c


clean :
	rm -f *.o util/*.o

TAGS  : $(SRCS) $(HDRS)
	etags -T $(HDRS) $(SRCS)

# set this to the GNU C++ include directory (irrelevent?)
DEPENDINCLUDES = -I/opt/gnu/lib/g++-include

depend   : $(SRCS)
	makedepend -DMAKINGDEPEND $(DEFS) $(DEPENDINCLUDES) $(INCLUDES) $(HDRS) $(SRCS)


# DO NOT DELETE THIS LINE -- make depend depends on it.

structs.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
structs.o: /usr/include/sys/sysmacros.h
utils.o: /usr/include/memory.h
act.comm.o: /usr/include/stdio.h /usr/include/string.h
act.comm.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
act.comm.o: /usr/include/sys/sysmacros.h utils.h /usr/include/memory.h comm.h
act.comm.o: interpreter.h handler.h db.h screen.h class.h rooms.h objs.h
act.informative.o: /usr/include/stdio.h /usr/include/stdlib.h
act.informative.o: /usr/include/sys/stdtypes.h /usr/include/string.h
act.informative.o: /usr/include/ctype.h /usr/include/time.h
act.informative.o: /usr/include/errno.h /usr/include/sys/errno.h
act.informative.o: /usr/include/sys/time.h structs.h /usr/include/sys/types.h
act.informative.o: /usr/include/sys/sysmacros.h class.h rooms.h objs.h
act.informative.o: utils.h /usr/include/memory.h comm.h interpreter.h
act.informative.o: handler.h db.h spells.h screen.h
act.movement.o: /usr/include/stdio.h /usr/include/string.h
act.movement.o: /usr/include/sys/stdtypes.h structs.h
act.movement.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h
act.movement.o: rooms.h objs.h utils.h /usr/include/memory.h comm.h
act.movement.o: interpreter.h handler.h db.h spells.h house.h
act.obj.o: /usr/include/stdio.h /usr/include/stdlib.h
act.obj.o: /usr/include/sys/stdtypes.h /usr/include/string.h structs.h
act.obj.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
act.obj.o: /usr/include/memory.h comm.h interpreter.h handler.h db.h spells.h
act.obj.o: objs.h class.h rooms.h
act.offensive.o: /usr/include/stdio.h /usr/include/string.h
act.offensive.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h structs.h
act.offensive.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
act.offensive.o: class.h rooms.h objs.h utils.h /usr/include/memory.h comm.h
act.offensive.o: interpreter.h handler.h db.h spells.h
act.other.o: /usr/include/stdio.h /usr/include/stdlib.h
act.other.o: /usr/include/sys/stdtypes.h /usr/include/string.h
act.other.o: /usr/include/ctype.h /usr/include/time.h
act.other.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
act.other.o: /usr/include/sys/stat.h structs.h class.h rooms.h objs.h utils.h
act.other.o: /usr/include/memory.h comm.h interpreter.h handler.h db.h
act.other.o: spells.h screen.h house.h
act.social.o: /usr/include/stdio.h /usr/include/stdlib.h
act.social.o: /usr/include/sys/stdtypes.h /usr/include/string.h structs.h
act.social.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h
act.social.o: objs.h rooms.h utils.h /usr/include/memory.h comm.h
act.social.o: interpreter.h handler.h db.h spells.h
act.wizard.o: /usr/include/stdio.h /usr/include/stdlib.h
act.wizard.o: /usr/include/sys/stdtypes.h /usr/include/string.h
act.wizard.o: /usr/include/ctype.h /usr/include/sys/time.h
act.wizard.o: /usr/include/time.h /usr/include/sys/types.h
act.wizard.o: /usr/include/sys/sysmacros.h structs.h class.h objs.h rooms.h
act.wizard.o: utils.h /usr/include/memory.h comm.h interpreter.h handler.h
act.wizard.o: db.h spells.h house.h screen.h constants.h
ban.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/stdio.h
ban.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
ban.o: /usr/include/string.h /usr/include/ctype.h /usr/include/time.h
ban.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h structs.h
ban.o: class.h utils.h /usr/include/memory.h comm.h interpreter.h handler.h
ban.o: db.h
boards.o: /usr/include/stdio.h /usr/include/stdlib.h
boards.o: /usr/include/sys/stdtypes.h /usr/include/string.h
boards.o: /usr/include/ctype.h /usr/include/sys/time.h /usr/include/time.h
boards.o: /usr/include/unistd.h /usr/include/sys/types.h
boards.o: /usr/include/sys/sysmacros.h structs.h class.h utils.h
boards.o: /usr/include/memory.h comm.h db.h boards.h interpreter.h handler.h
class.o: /usr/include/stdio.h /usr/include/stdlib.h
class.o: /usr/include/sys/stdtypes.h /usr/include/time.h structs.h
class.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h objs.h
class.o: db.h utils.h /usr/include/memory.h spells.h interpreter.h
comm.o: /usr/include/stdio.h /usr/include/stdlib.h
comm.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
comm.o: /usr/include/limits.h /usr/include/errno.h /usr/include/sys/errno.h
comm.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
comm.o: /usr/include/sys/socket.h /usr/include/sys/wait.h
comm.o: /usr/include/sys/time.h /usr/include/time.h
comm.o: /usr/include/sys/resource.h /usr/include/netdb.h
comm.o: /usr/include/string.h /usr/include/fcntl.h
comm.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
comm.o: /usr/include/signal.h /usr/include/sys/signal.h
comm.o: /usr/include/vm/faultcode.h /usr/include/netinet/in.h structs.h
comm.o: class.h objs.h rooms.h utils.h /usr/include/memory.h comm.h
comm.o: interpreter.h handler.h db.h house.h screen.h teleport.h
config.o: structs.h /usr/include/sys/types.h /usr/include/sys/stdtypes.h
config.o: /usr/include/sys/sysmacros.h
constants.o: structs.h /usr/include/sys/types.h /usr/include/sys/stdtypes.h
constants.o: /usr/include/sys/sysmacros.h
db.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
db.o: /usr/include/string.h /usr/include/ctype.h /usr/include/time.h
db.o: /usr/include/errno.h /usr/include/sys/errno.h structs.h
db.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h rooms.h objs.h
db.o: class.h utils.h /usr/include/memory.h db.h comm.h handler.h spells.h
db.o: mail.h interpreter.h house.h
fight.o: /usr/include/stdio.h /usr/include/stdlib.h
fight.o: /usr/include/sys/stdtypes.h /usr/include/string.h
fight.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
fight.o: /usr/include/sys/sysmacros.h class.h objs.h rooms.h utils.h
fight.o: /usr/include/memory.h spells.h comm.h handler.h interpreter.h db.h
fight.o: screen.h fight.h
graph.o: /usr/include/stdio.h /usr/include/stdlib.h
graph.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
graph.o: /usr/include/sys/sysmacros.h class.h rooms.h utils.h
graph.o: /usr/include/memory.h comm.h interpreter.h handler.h db.h spells.h
handler.o: /usr/include/string.h /usr/include/sys/stdtypes.h
handler.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/ctype.h
handler.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
handler.o: /usr/include/sys/sysmacros.h class.h objs.h rooms.h utils.h
handler.o: /usr/include/memory.h comm.h db.h handler.h interpreter.h spells.h
house.o: /usr/include/stdio.h /usr/include/stdlib.h
house.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
house.o: /usr/include/string.h /usr/include/time.h /usr/include/unistd.h
house.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
house.o: /usr/include/errno.h /usr/include/sys/errno.h structs.h rooms.h
house.o: class.h comm.h handler.h db.h interpreter.h utils.h
house.o: /usr/include/memory.h house.h
interpreter.o: /usr/include/stdio.h /usr/include/stdlib.h
interpreter.o: /usr/include/sys/stdtypes.h /usr/include/string.h
interpreter.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
interpreter.o: /usr/include/sys/sysmacros.h class.h comm.h interpreter.h db.h
interpreter.o: utils.h /usr/include/memory.h spells.h handler.h mail.h
interpreter.o: screen.h
limits.o: /usr/include/stdio.h /usr/include/stdlib.h
limits.o: /usr/include/sys/stdtypes.h /usr/include/sys/types.h
limits.o: /usr/include/sys/sysmacros.h /usr/include/assert.h
limits.o: /usr/include/string.h structs.h utils.h /usr/include/memory.h
limits.o: spells.h comm.h db.h handler.h class.h objs.h rooms.h
magic.o: /usr/include/stdio.h /usr/include/string.h
magic.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
magic.o: /usr/include/sys/sysmacros.h class.h rooms.h objs.h utils.h
magic.o: /usr/include/memory.h comm.h spells.h handler.h db.h
mail.o: /usr/include/stdio.h /usr/include/stdlib.h
mail.o: /usr/include/sys/stdtypes.h /usr/include/assert.h
mail.o: /usr/include/ctype.h /usr/include/string.h /usr/include/time.h
mail.o: structs.h /usr/include/sys/types.h /usr/include/sys/sysmacros.h
mail.o: class.h objs.h utils.h /usr/include/memory.h comm.h db.h
mail.o: interpreter.h handler.h mail.h
mobact.o: /usr/include/stdio.h /usr/include/stdlib.h
mobact.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
mobact.o: /usr/include/sys/sysmacros.h class.h rooms.h objs.h utils.h
mobact.o: /usr/include/memory.h db.h comm.h interpreter.h handler.h spells.h
modify.o: /usr/include/stdio.h /usr/include/stdlib.h
modify.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
modify.o: /usr/include/string.h /usr/include/time.h structs.h
modify.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h
modify.o: utils.h /usr/include/memory.h interpreter.h handler.h db.h comm.h
modify.o: spells.h mail.h boards.h olc.h
objsave.o: /usr/include/stdio.h /usr/include/string.h
objsave.o: /usr/include/sys/stdtypes.h /usr/include/sys/time.h
objsave.o: /usr/include/time.h /usr/include/ctype.h /usr/include/unistd.h
objsave.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
objsave.o: /usr/include/errno.h /usr/include/sys/errno.h structs.h class.h
objsave.o: rooms.h objs.h comm.h handler.h db.h interpreter.h utils.h
objsave.o: /usr/include/memory.h spells.h
olc.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
olc.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h class.h
olc.o: objs.h interpreter.h comm.h utils.h /usr/include/memory.h db.h
olc.o: /usr/include/ctype.h /usr/include/stdlib.h olc.h zedit.h
shop.o: /usr/include/stdio.h /usr/include/stdlib.h
shop.o: /usr/include/sys/stdtypes.h /usr/include/string.h
shop.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
shop.o: /usr/include/sys/sysmacros.h class.h rooms.h objs.h comm.h handler.h
shop.o: db.h interpreter.h utils.h /usr/include/memory.h shop.h
spec_assign.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
spec_assign.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h db.h
spec_assign.o: interpreter.h utils.h /usr/include/memory.h rooms.h
spec_procs.o: /usr/include/stdio.h /usr/include/stdlib.h
spec_procs.o: /usr/include/sys/stdtypes.h /usr/include/string.h
spec_procs.o: /usr/include/ctype.h /usr/include/sys/types.h
spec_procs.o: /usr/include/sys/sysmacros.h structs.h class.h rooms.h objs.h
spec_procs.o: utils.h /usr/include/memory.h comm.h interpreter.h handler.h
spec_procs.o: db.h spells.h
spell_parser.o: /usr/include/stdio.h /usr/include/string.h
spell_parser.o: /usr/include/sys/stdtypes.h structs.h
spell_parser.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h
spell_parser.o: objs.h rooms.h utils.h /usr/include/memory.h interpreter.h
spell_parser.o: spells.h handler.h comm.h db.h
spells.o: /usr/include/stdio.h /usr/include/string.h
spells.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
spells.o: /usr/include/sys/sysmacros.h class.h rooms.h objs.h utils.h
spells.o: /usr/include/memory.h comm.h spells.h handler.h db.h
utils.o: /usr/include/stdio.h /usr/include/stdlib.h
utils.o: /usr/include/sys/stdtypes.h /usr/include/string.h
utils.o: /usr/include/assert.h /usr/include/limits.h /usr/include/time.h
utils.o: /usr/include/unistd.h /usr/include/sys/types.h
utils.o: /usr/include/sys/sysmacros.h /usr/include/arpa/telnet.h
utils.o: /usr/include/netinet/in.h structs.h class.h rooms.h objs.h utils.h
utils.o: /usr/include/memory.h comm.h screen.h spells.h handler.h
utils.o: interpreter.h
weather.o: /usr/include/stdio.h /usr/include/string.h
weather.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
weather.o: /usr/include/sys/sysmacros.h rooms.h utils.h /usr/include/memory.h
weather.o: comm.h handler.h interpreter.h db.h
alias.o: /usr/include/stdio.h /usr/include/stdlib.h
alias.o: /usr/include/sys/stdtypes.h /usr/include/string.h
alias.o: /usr/include/unistd.h /usr/include/sys/types.h
alias.o: /usr/include/sys/sysmacros.h structs.h class.h utils.h
alias.o: /usr/include/memory.h interpreter.h
mobcmd.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
mobcmd.o: /usr/include/sys/sysmacros.h /usr/include/stdio.h
mobcmd.o: /usr/include/string.h /usr/include/stdlib.h /usr/include/ctype.h
mobcmd.o: structs.h class.h objs.h rooms.h db.h utils.h /usr/include/memory.h
mobcmd.o: handler.h interpreter.h comm.h
mobprog.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
mobprog.o: /usr/include/sys/sysmacros.h /usr/include/stdio.h
mobprog.o: /usr/include/string.h /usr/include/stdlib.h /usr/include/ctype.h
mobprog.o: structs.h class.h objs.h rooms.h utils.h /usr/include/memory.h
mobprog.o: interpreter.h handler.h db.h
iedit.o: /usr/include/stdio.h /usr/include/stdlib.h
iedit.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h structs.h
iedit.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h class.h objs.h
iedit.o: interpreter.h comm.h spells.h utils.h /usr/include/memory.h db.h
iedit.o: boards.h olc.h
redit.o: /usr/include/stdio.h /usr/include/ctype.h structs.h
redit.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
redit.o: /usr/include/sys/sysmacros.h class.h rooms.h interpreter.h comm.h
redit.o: spells.h utils.h /usr/include/memory.h db.h /usr/include/stdlib.h
redit.o: boards.h olc.h
zedit.o: /usr/include/stdio.h /usr/include/stdlib.h
zedit.o: /usr/include/sys/stdtypes.h /usr/include/string.h
zedit.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
zedit.o: /usr/include/sys/sysmacros.h class.h interpreter.h comm.h spells.h
zedit.o: utils.h /usr/include/memory.h db.h screen.h zedit.h
quest.o: /usr/include/stdio.h /usr/include/stdlib.h
quest.o: /usr/include/sys/stdtypes.h /usr/include/string.h
quest.o: /usr/include/unistd.h /usr/include/sys/types.h
quest.o: /usr/include/sys/sysmacros.h structs.h class.h utils.h
quest.o: /usr/include/memory.h interpreter.h
teleport.o: /usr/include/stdio.h /usr/include/stdlib.h
teleport.o: /usr/include/sys/stdtypes.h /usr/include/string.h structs.h
teleport.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h rooms.h
teleport.o: objs.h class.h utils.h /usr/include/memory.h interpreter.h
teleport.o: handler.h comm.h
