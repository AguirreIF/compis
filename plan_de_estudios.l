%option backup read noyywrap bison-bridge bison-locations never-interactive warn
%option yylineno outfile="plan_de_estudios.c" nodefault pointer 8bit nodefault perf-report perf-report

%{
	#include <stdio.h>
	#include <ctype.h>
	#include "plan_de_estudios.tab.h"

	#undef YY_BUF_SIZE
	#define YY_BUF_SIZE 1024

	char linebuf[1024];
	int yycolumn = 1;

	/* Define una acción que se ejecuta antes del primer escaneo */
	#define YY_USER_INIT \
		yylloc->first_line = yylloc->last_line = yylloc->first_column = yylloc->last_column = 1;

	/* Provee una acción que se ejecuta antes de la acción definida por la regla */
	#define YY_USER_ACTION									\
		yylloc->first_line = yylloc->last_line = yylineno;	\
		yylloc->first_column = yycolumn;					\
		yylloc->last_column = yycolumn + yyleng -1;			\
		yycolumn += yyleng;

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
			yycolumn = 1;
			yyless(1); /* devuelve todo menos \n para re-escanear */
		}

"<!--"				{ BEGIN(COMENTARIO); }
<COMENTARIO>--		{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }
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

	[mrnhacip][_a-zñ]*	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

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

	[acps[:upper:].áéíóúñÁÉÍÓÚÑ[:alnum:] ]*	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

}

<ITAG,FTAG,EN_TAG>.|\n	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

[\t ]		/* ignora tabluadores y espacios en blanco */

"<!-"|"<!"|.	{ LOCATION_PRINT(stderr, *yylloc); yyterminate(); return -1; }

%%

/*
	never-interactive
		Indica a flex que genere un escaner que nunca considera a su entrada como interactiva.

	pointer
		Especifica que yytext es un char *.

	8bit
		para que reconozca caracteres de 8 bits.

	bison-bridge
		La declaración
			int yylex (yyscan_t scanner);
		se convierte en
			int yylex (YYSTYPE * lvalp, yyscan_t scanner);
		y si también se agrega la opción bison-locations la declaración se convierte en
			int yylex (YYSTYPE * lvalp, YYLTYPE * llocp, yyscan_t scanner);

	read
		no utiliza la librería stdio para leer la entrada, en su lugar utiliza la
		llamada al sistema read()

	yylineno
		Mantiene el registro del número de línea leído de la entrada en la variable global yylineno

	noyywrap
		No llama a yywrap cuando encuentra el fin de archivo, asume que no hay más archivos para leer

	backup
		Genera información de backup y lo guarda en el archivo lex.backup. Indica los estados del escaner que
		requieren backup.

	outfile
		Nombre del archivo a generar

	perf-report
		Genera información que provocaría una seria perdida de performance en stderr. Si se especifica dos veces
		también muestra lo que generaría una pequeña perdida de performance

	nodefault
		No genera la regla por defecto, si el escaner encuentra una entrada que no coincide con alguna regla,
		aborta con un error

	warn
		Informa de ciertas cuestiones. En particular, si la regla por defecto puede coincidir pero no ha sido especificada.
*/
