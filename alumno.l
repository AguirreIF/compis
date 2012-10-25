%option read noyywrap bison-bridge noinput nounput bison-locations never-interactive warn backup yylineno
%option prefix="alu_" outfile="alumno.c" nodefault pointer 8bit nodefault perf-report perf-report

%{
	#include <stdio.h>
	#include <ctype.h>
	#include "alumno.tab.h"

	// redefine el tamaño del buffer, es de 16kb por defecto
	#undef YY_BUF_SIZE
	#define YY_BUF_SIZE 1024

	char linebuf[1024];
	int alu_column = 1;

	/* Define una acción que se ejecuta antes del primer escaneo */
	#define YY_USER_INIT \
		yylloc->first_line = yylloc->last_line = yylloc->first_column = yylloc->last_column = 1;

	/* Provee una acción que se ejecuta antes de la acción definida por la regla */
	#define YY_USER_ACTION									\
		yylloc->first_line = yylloc->last_line = yylineno;	\
		yylloc->first_column = alu_column;					\
		yylloc->last_column = alu_column + yyleng -1;			\
		alu_column += yyleng;

	/* Muestra la ubicación */
	# define LOCATION_PRINT(out, loc)																		\
	if ((loc).first_line != (loc).last_line)																\
		printf("%d.%d-%d.%d", (loc).first_line, (loc).first_column, (loc).last_line, (loc).last_column);	\
	else if ((loc).first_column < (loc).last_column)														\
		printf("%d.%d-%d", (loc).first_line, (loc).first_column, (loc).last_column);						\
	else																									\
		printf("%d.%d", (loc).first_line, (loc).first_column);												\
	printf(" token no reconocido: %s\n%s\n", yytext, linebuf);
%}

texto			[áéíóúñÁÉÍÓÚÑ[:alpha:] ]+
texto_id		[[:upper:]]"."[[:digit:]]{1,2}"."
nro_matricula	[[:digit:]]{5}
nro_anio		[[:digit:]]{4}
fecha			[[:digit:]]{1,2}"/"[[:digit:]]{1,2}"/"{nro_anio}

%x ITAG FTAG EN_TAG COMENTARIO

%%
	/* el alcance queda en la función YY_DECL, en vez de ser global */
	static int en_tag = 0;

<*>\n.*	{
			strncpy(linebuf, yytext + 1, sizeof(linebuf)); /* guarda la siguiente línea */
			alu_column = 1;
			yyless(1); /* devuelve todo menos \n para re-escanear */
		}

[\t ]		/* ignora tabluadores y espacios en blanco */

"<!--"				{ BEGIN(COMENTARIO); }
<COMENTARIO>--		{ printf("Otro: %s\n", yytext); yylval->cadena = strdup(yytext); return -1; }
<COMENTARIO>.		/* ignora comentarios */
<COMENTARIO>"-->"	{ BEGIN(0); }
<COMENTARIO><<EOF>>	{ printf("Comentario sin cerrar"); yyterminate(); }

\<					{ BEGIN(ITAG); en_tag = 1; return yytext[0]; }
"</"				{ BEGIN(ITAG); yylval->cadena = strdup(yytext); return MEB; }

<ITAG>{

	alumno				{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return ALUMNO; }
	año_regularizado	{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return ANIO_REGULARIZADO; }
	apellido			{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return APELLIDO; }
	fecha_aprobación	{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return FECHA_APROBACION; }
	id					{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return ID; }
	materia				{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return MATERIA; }
	matrícula			{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return MATRICULA; }
	nombre				{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return NOMBRE; }
	situación_materias	{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return SITUACION_MATERIAS; }

	[afimns][[:alnum:]_óíñ ]* { LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

}

<FTAG>>	{ BEGIN(en_tag == 1 ? EN_TAG : 0); en_tag = 0; return yytext[0]; }

<EN_TAG>{

	{nro_matricula}		{ BEGIN(0); yylval->entero = atoi(yytext); return NRO_MATRICULA; }
	{nro_anio}			{ BEGIN(0); yylval->entero = atoi(yytext); return NRO_ANIO; }
	{fecha}				{ BEGIN(0); yylval->cadena = strdup(yytext); return FECHA; }
	{texto_id}			{ BEGIN(0); yylval->cadena = strdup(yytext); return TEXTO_ID; }
	{texto}				{ BEGIN(0); yylval->cadena = strdup(yytext); return TEXTO; }

	[áéíóúñÁÉÍÓÚÑ[:alpha:][:upper:][:digit:]/. ]*	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

}

<ITAG,FTAG,EN_TAG>.|\n	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

"<!"-?|.	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

%%
