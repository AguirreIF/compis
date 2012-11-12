CC       := cc
CFLAGS   := -ggdb3 -std=gnu99 $(shell pkg-config --cflags libgvc) -Wall -Wextra -pedantic
LDFLAGS  := $(shell pkg-config --libs libgvc)

LEX      := flex
YACC     := bison
TAGS     := ctags
TAGS_OPT := --fields=+l --c-kinds=+p --extra=+q

SRC := $(wildcard *.c) alumno.c alumno.tab.c plan_de_estudios.c plan_de_estudios.tab.c
OBJ := $(SRC:.c=.o)

all: analizador tags

analizador: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} -o ../bin/$@ $^

alumno.c: alumno.l alumno.tab.h
	${LEX} $<

plan_de_estudios.c: plan_de_estudios.l plan_de_estudios.tab.h
	${LEX} $<

%.tab.c %.tab.h: %.y
	${YACC} $<

tags: ${SRC} alumno.l alumno.y plan_de_estudios.l plan_de_estudios.y
	${TAGS} ${TAGS_OPT} $^

clean:
	rm -f *.tab.[ch] alumno.c plan_de_estudios.c ../bin/analizador *.output *.png *.backup *.o

.PHONY: all clean
