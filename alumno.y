%{
	#include <stdio.h>
	#include <ctype.h>

	extern int yylineno;

	void yyerror(const char *str);

	struct {
		int matricula;
		char* apellido;
		char* nombre;
	} alumno;
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%locations
%error-verbose

%union {
	char *cadena;
	int entero;
}

%token <cadena> MEB "</"
%token <cadena> ALUMNO MATRICULA APELLIDO NOMBRE SITUACION_MATERIAS MATERIA ID ANIO_REGULARIZADO
				FECHA_APROBACION TEXTO_ID TEXTO FECHA OTRO
%token <entero> NRO_MATRICULA NRO_ANIO

%token END 0 "fin de archivo"

%start alumno

%%

alumno:
	'<'ALUMNO'>' matricula apellido nombre situacion_materias "</"ALUMNO'>';

matricula: '<'MATRICULA'>' NRO_MATRICULA "</"MATRICULA'>' { alumno.matricula = $4; };

apellido: '<'APELLIDO'>' TEXTO "</"APELLIDO'>' { alumno.apellido = $4; };

nombre: '<'NOMBRE'>' TEXTO "</"NOMBRE'>' { alumno.nombre = $4; };

situacion_materias: '<'SITUACION_MATERIAS'>' lista_materias "</"SITUACION_MATERIAS'>';

lista_materias:
	materia lista_materias
	|
	materia;

materia:
	'<'MATERIA'>' id anio_regularizado "</"MATERIA'>'
	|
	'<'MATERIA'>' id anio_regularizado fecha_aprobacion "</"MATERIA'>';

id:
	'<'ID'>' TEXTO_ID "</"ID'>';

anio_regularizado:
	'<'ANIO_REGULARIZADO'>' NRO_ANIO "</"ANIO_REGULARIZADO'>';

fecha_aprobacion:
	'<'FECHA_APROBACION'>' FECHA "</"FECHA_APROBACION'>';
%%

int main(void) {
	yyparse();
	printf("Matricula: %d\n", alumno.matricula);
	printf("Nombre: %s\n", alumno.nombre);
	printf("Apellido: %s\n", alumno.apellido);
	return 0;
}

void yyerror(const char *str) {
	fprintf(stderr, "Error en línea %d: %s\n", yylineno, str);
}
