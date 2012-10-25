CC       = cc
CFLAGS   = -ggdb3 -std=gnu99 -Wall -Werror -pedantic-errors

LEX      = flex
LEX_BAK  = lex.backup
YACC     = bison
TAGS     = ctags
TAGS_OPT = --fields=+l --c-kinds=+p --extra=+q

all: pe tags

pe: programa.c estructuras.h alumno.c alumno.tab.c funciones_alumno.h plan_de_estudios.c plan_de_estudios.tab.c funciones_plan.h
	${CC} ${CFLAGS} -o $@ $^

plan_de_estudios.c: plan_de_estudios.l plan_de_estudios.tab.h
	${LEX} $<
	[ -f ${LEX_BAK} ] && mv ${LEX_BAK} plan_de_estudios.backup

alumno.c: alumno.l alumno.tab.h
	${LEX} $<
	[ -f ${LEX_BAK} ] && mv ${LEX_BAK} alumno.backup

%.tab.h: %.y
	${YACC} $<

tags: programa.c estructuras.h alumno.l alumno.y funciones_alumno.h plan_de_estudios.l plan_de_estudios.y funciones_plan.h
	${TAGS} ${TAGS_OPT} $^

clean:
	rm -f *.tab.[ch] alumno.c plan_de_estudios.c *.backup pe alu *.output

.PHONY: all clean
