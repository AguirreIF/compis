CC       = cc
CFLAGS   = -ggdb3

LEX      = flex
LEX_BAK  = lex.backup
YACC     = bison
TAGS     = ctags
TAGS_OPT = --fields=+l --c-kinds=+p --extra=+q

all: pe alu tags

pe: plan_de_estudios.c plan_de_estudios.tab.c funciones_plan.h estructuras.h
	${CC} ${CFLAGS} -o $@ $^

alu: alumno.c alumno.tab.c
	${CC} ${CFLAGS} -o $@ $^

plan_de_estudios.c: plan_de_estudios.l plan_de_estudios.tab.h
	${LEX} $<
	mv ${LEX_BAK} plan_de_estudios.backup

alumno.c: alumno.l alumno.tab.h
	${LEX} $<
	mv ${LEX_BAK} alumno.backup

%.tab.h: %.y
	${YACC} $<

tags: alumno.l alumno.y plan_de_estudios.l plan_de_estudios.y funciones_plan.h
	${TAGS} ${TAGS_OPT} $^

clean:
	@echo "Limpiando..."
	rm -f *.tab.* *.c *.backup pe alu tags *.output

.PHONY: all clean
