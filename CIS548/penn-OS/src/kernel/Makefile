PROG=pennOS
CC = clang

# Replace -O3 with -g for a debug version during development
CFLAGS = -Wall

TOPDIR = $(PWD)
LOG_FILE = \"log/log.txt\"
override CPPFLAGS += -DLOG_FILE=$(LOG_FILE)

SRCS = $(filter-out main.c, $(wildcard *.c))
OBJS = $(SRCS:.c=.o)

.PHONY : all clean

# $(PROG) : $(OBJS)
# 	$(CC) -o $@ $^ parser.o
all : $(OBJS)

clean :
	$(RM) $(OBJS) $(PROG)


