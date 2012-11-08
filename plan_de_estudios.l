/* %option read noyywrap noinput nounput bison-bridge bison-locations never-interactive warn yylineno */
%option read noyywrap noinput nounput bison-bridge never-interactive warn yylineno
%option prefix="plan_" outfile="plan_de_estudios.c" nodefault pointer 8bit nodefault perf-report perf-report

%{
	#include <stdio.h>
	#include <ctype.h>
	#include "plan_de_estudios.tab.h"

	// redefine el tamaño del buffer, es de 16kb por defecto
	#undef YY_BUF_SIZE
	#define YY_BUF_SIZE 1024

	char linebuf[1024];
	int plan_column = 1;

	/* Define una acción que se ejecuta antes del primer escaneo */
	/* #define YY_USER_INIT \ */
		/* yylloc->first_line = yylloc->last_line = yylloc->first_column = yylloc->last_column = 1; */

	/* Provee una acción que se ejecuta antes de la acción definida por la regla */
	/* #define YY_USER_ACTION										\ */
		/* yylloc->first_line = yylloc->last_line = plan_lineno;	\ */
		/* yylloc->first_column = plan_column;						\ */
		/* yylloc->last_column = plan_column + yyleng -1;			\ */
		/* plan_column += yyleng; */

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

texto			[áéíóúñÁÉÍÓÚÑ[:alnum:] ]+
texto_id		[[:upper:]]"."[[:digit:]]{1,2}"."
anio_plan		[[:digit:]]{4}
dos_digitos		[[:digit:]]{1,2}
	/*
	cant_horas		[[:digit:]]{1,2}
	anio_materia	[[:digit:]]
	*/

%x ITAG FTAG EN_TAG COMENTARIO

%%
	/* el alcance queda en la función YY_DECL, en vez de ser global */
	static int en_tag = 0;

<*>\n.*	{
			strncpy(linebuf, yytext + 1, sizeof(linebuf)); /* guarda la siguiente línea */
			plan_column = 1;
			yyless(1); /* devuelve todo menos \n para re-escanear */
		}

"<!--"				{ BEGIN(COMENTARIO); }
 /* <COMENTARIO>--		{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; } */
<COMENTARIO>--		{ yyterminate(); return -1; }
<COMENTARIO>.		/* ignora comentarios */
<COMENTARIO>"-->"	{ BEGIN(0); }
<COMENTARIO><<EOF>>	{ printf("Comentario sin cerrar"); yyterminate(); }

\<					{ BEGIN(ITAG); en_tag = 1; return yytext[0]; }
"</"				{ BEGIN(ITAG); yylval->cadena = strdup(yytext); return MEB; }

<ITAG>{

	plan_de_estudios	{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return PLAN_DE_ESTUDIOS; }
	carrera				{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return CARRERA; }
	año					{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return ANIO; }
	materias			{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return MATERIAS; }
	materia				{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return MATERIA; }
	id					{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return ID; }
	nombre				{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return NOMBRE; }
	regimen				{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return REGIMEN; }
	cuatrimestre		{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return CUATRIMESTRE; }
	horas				{ BEGIN(FTAG); yylval->cadena = strdup(yytext); return HORAS; }
	correlativas		{ BEGIN(FTAG); en_tag = 0; yylval->cadena = strdup(yytext); return CORRELATIVAS; }

	/* [mrnhacip][_a-zñ]*	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; } */
	[mrnhacip][_a-zñ]*	{ yyterminate(); return -1; }

}

<FTAG>>	{ BEGIN(en_tag == 1 ? EN_TAG : 0); en_tag = 0; return yytext[0]; }

<EN_TAG>{

	anual			{ BEGIN(0); yylval->cadena = strdup(yytext); return ANUAL; }
	cuatrimestral	{ BEGIN(0); yylval->cadena = strdup(yytext); return CUATRIMESTRAL; }
	primero			{ BEGIN(0); yylval->cadena = strdup(yytext); return PRIMERO; }
	segundo			{ BEGIN(0); yylval->cadena = strdup(yytext); return SEGUNDO; }
	{anio_plan}		{ BEGIN(0); yylval->entero = atoi(yytext); return ANIO_PLAN; }
	{dos_digitos}	{ BEGIN(0); yylval->entero = atoi(yytext); return DOS_DIGITOS; }
	{texto_id}		{ BEGIN(0); yylval->cadena = strdup(yytext); return TEXTO_ID; }
	{texto}			{ BEGIN(0); yylval->cadena = strdup(yytext); return TEXTO; }

	/* [acps[:upper:].áéíóúñÁÉÍÓÚÑ[:alnum:] ]*	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; } */
	[acps[:upper:].áéíóúñÁÉÍÓÚÑ[:alnum:] ]*	{ yyterminate(); return -1; }

}

 /* <ITAG,FTAG,EN_TAG>.|\n	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }  */
<ITAG,FTAG,EN_TAG>.|\n	{ yyterminate(); return -1; } 
[\t ]		/* ignora tabluadores y espacios en blanco */

 /* "<!-"|"<!"|.	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; } */
"<!-"|"<!"|.	{ yyterminate(); return -1; }

%%
