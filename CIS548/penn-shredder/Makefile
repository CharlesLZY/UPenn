######################################################################
#
#                       Author: Hannah Pan
#                       Date:   01/13/2021
#
# The autograder will run the following command to build the program:
#
#       make -B
#
# To build a version that does not call kill(2), it will run:
#
#       make -B CPPFLAGS=-DEC_NOKILL
#
######################################################################

# name of the program to build
#
PROG=penn-shredder

PROMPT='"$(PROG)\# "'
CATCHPHRASE='"Bwahaha ... Tonight, I dine on turtle soup!\n"'

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT) -DCATCHPHRASE=$(CATCHPHRASE)

CC = clang

# Replace -O1 with -g for a debug version during development
#
CFLAGS = -Wall -Werror -O1

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY : clean

$(PROG) : $(OBJS)
	$(CC) -o $@ $^

clean :
	$(RM) $(OBJS) $(PROG)
