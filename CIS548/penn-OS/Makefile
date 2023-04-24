PROG=pennOS
CC = clang

# Replace -O3 with -g for a debug version during development
CFLAGS = -Wall -g

TOP_DIR = $(PWD)
BIN_DIR = $(TOP_DIR)/bin
SRC_DIR = $(TOP_DIR)/src
# LOG_DIR = $(TOP_DIR)/log

FS_DIR = $(SRC_DIR)/PennFAT
KERNEL_DIR = $(SRC_DIR)/kernel
SRC_SUBDIRS = $(SRC_DIR)/PennFAT $(SRC_DIR)/kernel
MAIN = $(KERNEL_DIR)/main.c
# LOG_FILE = \"${LOG_DIR}/log.txt\"
# override CPPFLAGS += -DLOG_FILE=$(LOG_FILE)

FS_SRCS = $(filter-out $(FS_DIR)/test-playground.c $(FS_DIR)/pennFAT.c,  $(wildcard $(FS_DIR)/*.c))
KERNEL_SRCS = $(filter-out $(KERNEL_DIR)/main.c, $(wildcard $(KERNEL_DIR)/*.c))
FS_OBJS = $(FS_SRCS:.c=.o)
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

FS_MAIN = $(FS_DIR)/pennFAT.c
FS_PROG = $(BIN_DIR)/pennFAT
PROG = $(BIN_DIR)/pennOS
CLEANS = $(KERNEL_OBJS) $(FS_OBJS) $(PROG) $(FS_PROG) log/log.txt $(FS_DIR)/pennFAT.o

.PHONY : $(SRC_SUBDIRS) clean

all : $(PROG) $(FS_PROG)
$(PROG) : $(SRC_SUBDIRS)
	$(CC) ${CFLAGS} -lm -o $@ $(MAIN) $(KERNEL_OBJS) $(FS_SRCS)

$(FS_PROG) : $(FS_SRCS)
	$(CC) ${CFLAGS} -lm -o $@ $(FS_MAIN) $(FS_SRCS) $(KERNEL_OBJS)

$(SRC_SUBDIRS) :
	$(MAKE) -C $@

clean :
	$(RM) $(CLEANS)


