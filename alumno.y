%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include "funciones_alumno.h"
	#include "funciones_plan.h"

	// la declaración hace falta sino da
	// warning: implicit declaration of function ‘alu_lex’
	extern int alu_lex ();
	extern int alu_lineno;
	extern FILE *alu_in;

	typedef struct YYLTYPE {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	} YYLTYPE;
	# define YYLTYPE_IS_DECLARED 1
	// supuestamente no tendría que agregar esta definición
	// ya está en alumno.tab.h

	void alu_error (YYLTYPE *alu_lloc, alumno_t *alumno, plan_de_estudios *pe, const char *str);

	cursado_t *cursado;
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
%locations
%name-prefix "alu_"
%parse-param {alumno_t *alumno}
%parse-param {plan_de_estudios *pe}

%union {
	char *cadena;
	int entero;
}

%token <cadena> MEB "</"
%token <cadena> ALUMNO MATRICULA APELLIDO NOMBRE SITUACION_MATERIAS MATERIA ID ANIO_REGULARIZADO
				FECHA_APROBACION TEXTO_ID TEXTO FECHA OTRO
%token <entero> NRO_MATRICULA NRO_ANIO

%type <cadena> apellido nombre id fecha_aprobacion
%type <entero> matricula anio_regularizado

%token END 0 "fin de archivo"

%start alumno

%%

alumno:
	'<'ALUMNO'>' matricula apellido nombre situacion_materias "</"ALUMNO'>';

matricula:
	'<'MATRICULA'>' NRO_MATRICULA "</"MATRICULA'>' {
		alumno->matricula = $NRO_MATRICULA;
	};

apellido:
	'<'APELLIDO'>' TEXTO "</"APELLIDO'>' {
		alumno->apellido = strdup ($TEXTO);
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		alumno->nombre = strdup ($TEXTO);
	};

situacion_materias:
	'<'SITUACION_MATERIAS'>' lista_materias "</"SITUACION_MATERIAS'>';

lista_materias:
	materia lista_materias
	|
	materia;

materia:
	'<'MATERIA'>' id anio_regularizado "</"MATERIA'>'
	|
	'<'MATERIA'>' id anio_regularizado fecha_aprobacion "</"MATERIA'>';

id:
	'<'ID'>' TEXTO_ID "</"ID'>' {
		if (alumno->cursado == NULL) {
			alumno->cursado = (cursado_t *) malloc (sizeof (cursado_t));
			if (alumno->cursado == NULL) {
				MALLOC_MSG;
				exit (EXIT_FAILURE);
			}
			alumno->cursado->materia = buscar_materia (pe, $TEXTO_ID, __func__, 0);
			alumno->cursado->anterior = NULL;
			cursado = alumno->cursado;
		}
		else {
			cursado->siguiente = (cursado_t *) malloc (sizeof (cursado_t));
			if (cursado->siguiente == NULL) {
				MALLOC_MSG;
				exit (EXIT_FAILURE);
			}
			cursado->siguiente->materia = buscar_materia (pe, $TEXTO_ID, __func__, 0);
			cursado->siguiente->anterior = cursado;
			cursado = cursado->siguiente;
		}
		cursado->aprobar_para_rendir = NULL;
		cursado->siguiente = NULL;
		cursado->fecha_aprobacion = NULL;
	};

anio_regularizado:
	'<'ANIO_REGULARIZADO'>' NRO_ANIO "</"ANIO_REGULARIZADO'>' {
		cursado->anio_regularizado = $NRO_ANIO;
	};

fecha_aprobacion:
	'<'FECHA_APROBACION'>' FECHA "</"FECHA_APROBACION'>' {
		cursado->fecha_aprobacion = strdup ($FECHA);
	};
%%

alumno_t *
procesar_alumno (char *alumno_xml, plan_de_estudios *pe)
{
	alu_in = fopen (alumno_xml, "r");
	alumno_t *alumno = NULL;
	if (alu_in == NULL)
		fprintf (stderr, "Error al intentar abrir el archivo `%s': %s\n", alumno_xml, strerror (errno));
	else {
		// Inicializo en NULL porque puede que tenga que analizar varios XML
		alumno = (alumno_t *) malloc (sizeof (alumno_t));
		if (alumno == NULL) {
			MALLOC_MSG;
			exit (EXIT_FAILURE);
		}
		alumno->matricula = 0;
		alumno->apellido = NULL;
		alumno->nombre = NULL;
		alumno->cursado = NULL;

		int salida = alu_parse (alumno, pe);
		if (salida == 0)
			procesar_cursado (alumno);
		else {
			alumno = NULL;
			if (salida != 1)
				fprintf (stderr, "Problemas de memoria u otra cosa\n");
		}
	}
	return alumno;
}

void
alu_error (YYLTYPE *alu_lloc, alumno_t *alumno, plan_de_estudios *pe, const char *str)
{
	fprintf (stderr, "\nError en línea [%d:%d-%d]: %s\n", alu_lineno, \
		alu_lloc->first_column, alu_lloc->last_column, str);
	if (alumno && alumno->nombre != NULL)
		printf ("De: %s %s\n", alumno->nombre, alumno->apellido);
	if (pe && pe->nombre_carrera != NULL)
		printf ("plan: %s\n", pe->nombre_carrera);
}
