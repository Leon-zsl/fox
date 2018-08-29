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

SRCS=fox.c			\
	lua_l.c			\
	lua_y.c			\
	lua_sym.c		\
	parser.c		\
	generator.c

OBJS=$(SRCS:.c=.o)

TARGET=fox

all: $(TARGET)

clean:
	rm -rf lua_y.c lua_l.c
	rm -rf *.o
	rm -rf $(TARGET)

lua: lua_y.c lua_l.c

$(TARGET): $(OBJS)
	$(CC)  -o $@ $^ $(LDFLAGS) $(LIBS)

*.o: *.c
	$(CC) -c $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

lua_y.c: lua.y
	$(YACC) -o $@ $<

lua_l.c: lua.l
	$(LEX) -o $@ $<

.PHONY: all clean lua
