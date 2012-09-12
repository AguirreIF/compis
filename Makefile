CC       = cc
CFLAGS   = -ggdb3
LDFLAGS  =

LEX      = flex
LEX_SRC  = $(wildcard *.l)
LEX_BK   = lex.backup
YACC     = bison
YACC_SRC = $(wildcard *.y)
SRC      = ${LEX_SRC:.l=.c}
OBJ      = ${SRC:.c=.o}

all: pe alu tags

pe: plan_de_estudios.c plan_de_estudios.tab.c funciones.h
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

alu: alumno.c alumno.tab.c
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

plan_de_estudios.c: plan_de_estudios.l plan_de_estudios.tab.h
	${LEX} $<
	mv lex.backup plan_de_estudios.backup

alumno.c: alumno.l alumno.tab.h
	${LEX} $<
	mv lex.backup alumno.backup

%.tab.h: %.y
	bison $<

tags: alumno.l alumno.y plan_de_estudios.l plan_de_estudios.y funciones.h
	cscope -b $^

clean:
	@echo "limpiando..."
	rm -f *.tab.* *.c *.backup pe alu *.output cscope.out

.PHONY: all clean tags
