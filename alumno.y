%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>

	#include "funciones_alumno.h"

	// la declaración hace falta sino da
	// warning: implicit declaration of function ‘alu_lex’
	extern int alu_lex();
	extern int alu_lineno;
	extern FILE *alu_in;

	void alu_error (plan_de_estudios *pe, const char *str);

	alumno_t alumno;
	cursado_t *cursado;
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
/* %locations */
%name-prefix "alu_"
%parse-param {plan_de_estudios *pe}

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

matricula:
	'<'MATRICULA'>' NRO_MATRICULA "</"MATRICULA'>' {
		alumno.matricula = $NRO_MATRICULA;
	};

apellido:
	'<'APELLIDO'>' TEXTO "</"APELLIDO'>' {
		alumno.apellido = $TEXTO;
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		alumno.nombre = $TEXTO;
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
		if (alumno.cursado == NULL) {
			alumno.cursado = (cursado_t *) malloc (sizeof (cursado_t));
			alumno.cursado->materia = buscar_materia (pe, $TEXTO_ID);
			alumno.cursado->anterior = NULL;
			cursado = alumno.cursado;
		}
		else {
			cursado->siguiente = (cursado_t *) malloc (sizeof (cursado_t));
			cursado->siguiente->materia = buscar_materia (pe, $TEXTO_ID);
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

void procesar_alumno (char *alumno_xml, plan_de_estudios *pe) {

	alu_in = fopen (alumno_xml, "r");
	if (alu_in == NULL)
		fprintf (stderr, "Error al intentar abrir el archivo `%s': %s\n", alumno_xml, strerror (errno));
	else {
		// Inicializo en NULL porque puede que tenga que analizar varios XML
		alumno.cursado = NULL;
		int salida = alu_parse (pe);

		if (salida == 0) {
			procesar_cursado (pe, alumno);
			informe_alumno (pe, alumno);
		}
		else if (salida == 1)
			fprintf(stderr, "Alguna entrada inválida\n");
		else
			fprintf(stderr, "Problemas de memoria u otra cosa\n");
	}
}

void alu_error (plan_de_estudios *pe, const char *str) {
	fprintf (stderr, "Error en línea %d: %s\n", alu_lineno, str);
}
