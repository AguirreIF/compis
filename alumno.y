%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>

	#include "funciones_alumno.h"

	extern int alu_lineno;

	// Defino el puntero del archivo de entrada
	extern  FILE *alu_in;

	void alu_error(const char *str);
	alumno alu;	
	materia_t *mat;
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
%locations
%name-prefix "alu_"

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
		alu.matricula = $NRO_MATRICULA;
	};

apellido:
	'<'APELLIDO'>' TEXTO "</"APELLIDO'>' {
		alu.apellido = $TEXTO;
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		alu.nombre = $TEXTO;
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
		if (!alu.materia) {
			// significa que tengo que crearla
			alu.materia = crear_materia_alu();
			mat = alu.materia;
		}
		else {
			mat->siguiente = crear_materia_alu();
			mat = mat->siguiente;
		}
		mat->id = $TEXTO_ID;
		// Inicializo la fecha de aprobación, es para que no se 
		// produzcan errores a la hora de mostrar los datos.
		// Así no hace falta verificar si tiene algún dato o no.
		mat->fecha_ap = "-/-/-";
	};

anio_regularizado:
	'<'ANIO_REGULARIZADO'>' NRO_ANIO "</"ANIO_REGULARIZADO'>' {
		mat->anio_reg = $NRO_ANIO;
	};

fecha_aprobacion:
	'<'FECHA_APROBACION'>' FECHA "</"FECHA_APROBACION'>' {
		mat->fecha_ap = $FECHA;
	};
%%

void procesar_alumno (char *alumno_xml, struct plan_de_estudios *pe) {

	alu_in = fopen(alumno_xml, "r");
	if (alu_in == NULL) {
		fprintf(stderr, "Error al intentar abrir el archivo `%s': %s\n", alumno_xml, strerror(errno));
	}
	else {
		int salida = alu_parse();
		if (salida == 0) {
			mostrar_alumno(alu);
			mostrar_materias_alumno(alu);
			mostrar_materias_a_rendir(pe, alu);
			mostrar_materias_a_cursar(pe, alu);	
		}
		else if (salida == 1)
			fprintf(stderr, "Alguna entrada inválida\n");
		else
			fprintf(stderr, "Problemas de memoria u otra cosa\n");
	}
}

void alu_error(const char *str) {
	fprintf(stderr, "Error en línea %d: %s\n", alu_lineno, str);
}
