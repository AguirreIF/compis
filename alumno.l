%option read noyywrap bison-bridge bison-locations never-interactive warn backup yylineno
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
texto_id		[[:upper:]]"."[[:digit:]]{1,2}
nro_matricula	[[:digit:]]{6}
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

/*
	http://people.cs.aau.dk/~marius/sw/flex/Advanced-Use-of-Flex.html
	http://aquamentus.com/flex_bison.html
	http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
	http://oreilly.com/linux/excerpts/9780596155971/error-reporting-recovery.html
	http://www.lrde.epita.fr/~akim/ccmp/doc/gnuprog2/Advanced-Use-of-Bison.html
*/

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
