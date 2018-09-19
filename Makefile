# Fox Makefile

CC=gcc
LD=ld
LEX=flex
YACC=bison

DEFINES=
INCLUDES=
LIBS=
CFLAGS=-Wall -g -O2 -std=c99
#LDFLAGS=-ll -ly
LDFLAGS=

SRCS=lua_l.c		\
	lua_y.c			\
	symbol.c		\
	syntax.c		\
	translator.c	\
	fox.c

OBJS=$(SRCS:.c=.o)

TARGET=fox

all: lua $(TARGET)

lua: lua_l.c lua_y.c

clean:
	rm -rf lua_l.c lua_y.c lua_y.h lua_y.output
	rm -rf *.o
	rm -rf $(TARGET)

$(TARGET): $(OBJS)
	$(CC)  -o $@ $^ $(LDFLAGS) $(LIBS)

*.o: *.c
	$(CC) -c $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

lua_y.c: lua.y
	$(YACC) -d -v -o $@ $<

lua_l.c: lua.l
	$(LEX) -o $@ $<

test: test.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

test_clean:
	rm -rf test.o test

tmp_clean:
	rm -rf *.o

.PHONY: all clean lua test test_clean
