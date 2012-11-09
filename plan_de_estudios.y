%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include "funciones_plan.h"

	// la declaración hace falta sino da
	// warning: implicit declaration of function ‘plan_lex’
	extern int plan_lex ();
	extern int plan_lineno;
	extern FILE *plan_in;

	void plan_error (plan_de_estudios *pe, const char *str);

	materia_t *m_aux;
	char *id_materia, *nombre_materia;
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
/* %locations */
%name-prefix "plan_"
%parse-param {plan_de_estudios *pe}

%union {
	char *cadena;
	int entero;
}

%token <cadena> MEB "</"
%token <cadena> PLAN_DE_ESTUDIOS CARRERA ANIO MATERIAS MATERIA ID NOMBRE REGIMEN ANUAL CUATRIMESTRAL
%token <cadena>	PRIMERO SEGUNDO CUATRIMESTRE HORAS CORRELATIVAS TEXTO_ID TEXTO OTRO
%token <entero> ANIO_PLAN DOS_DIGITOS

%type <cadena> carrera id nombre regimen cuatrimestre
%type <entero> anio_plan anio horas

%token END 0 "fin de archivo"

%start plan

%%

plan:
	'<'PLAN_DE_ESTUDIOS'>' carrera anio_plan materias "</"PLAN_DE_ESTUDIOS'>';

carrera:
	'<'CARRERA'>' TEXTO "</"CARRERA'>' {
		pe->nombre_carrera = strdup ($TEXTO);
	};

anio_plan:
	'<'ANIO'>' ANIO_PLAN "</"ANIO'>' {
		pe->anio_del_plan = $ANIO_PLAN;
	};

materias:
	'<'MATERIAS'>' lista_materias "</"MATERIAS'>';

lista_materias:
	materia lista_materias
	|
	materia;

materia:
	'<'MATERIA'>' id nombre anio regimen horas correlativas "</"MATERIA'>';

id:
	'<'ID'>' TEXTO_ID "</"ID'>' {
		$id = $TEXTO_ID;
		id_materia = strdup ($id);
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		nombre_materia = strdup ($TEXTO);
	};

anio:
	'<'ANIO'>' DOS_DIGITOS[ANIO_MATERIA] "</"ANIO'>' {
		anio_t *anio_aux = crear_anio_carrera ($ANIO_MATERIA, pe);
		m_aux = crear_materia (anio_aux, id_materia, nombre_materia);
	};

regimen:
	'<'REGIMEN'>' ANUAL "</"REGIMEN'>' {
		++pe->anuales;
		m_aux->regimen = strdup ($ANUAL);
		m_aux->duracion = 1;
	}
	|
	'<'REGIMEN'>' CUATRIMESTRAL "</"REGIMEN'>' cuatrimestre {
		++pe->cuatrimestrales;
		m_aux->regimen = strdup ($CUATRIMESTRAL);
		m_aux->duracion = 0;
	};

cuatrimestre:
	'<'CUATRIMESTRE'>' PRIMERO "</"CUATRIMESTRE'>' {
		m_aux->cuatrimestre = strdup ($PRIMERO);
	}
	|
	'<'CUATRIMESTRE'>' SEGUNDO "</"CUATRIMESTRE'>' {
		m_aux->cuatrimestre = strdup ($SEGUNDO);
	};

horas:
	'<'HORAS'>' DOS_DIGITOS[CANT_HORAS] "</"HORAS'>' {
		m_aux->horas = $CANT_HORAS;
		pe->total_horas += (strcmp (m_aux->regimen, "anual") ? \
			m_aux->horas * SEMANAS_POR_CUATRIMESTRE :          \
			m_aux->horas * SEMANAS_POR_CUATRIMESTRE * 2);
	};

correlativas:
	/* vacio */
	|
	'<'CORRELATIVAS'>' lista_correlativas "</"CORRELATIVAS'>';

lista_correlativas:
	id lista_correlativas {
		cargar_correlativa (m_aux, $id);
	}
	|
	id {
		cargar_correlativa (m_aux, $id);
	};
%%

plan_de_estudios
*procesar_plan (const char *plan_xml)
{
	plan_in = (strcmp (plan_xml, "-") == 0) ? (FILE *) 0 : fopen (plan_xml, "r");
	// pregunto por errno porque si abro la entrada estándar, plan_in vale 0 (NULL)
	// y el 0 de errno significa success
	plan_de_estudios *pe;
	if (errno != 0)
		fprintf (stderr, "Error al intentar abrir el archivo `%s': %s\n", plan_xml, strerror (errno));
	else {
		pe = (plan_de_estudios *) malloc (sizeof (plan_de_estudios));
		if (pe == NULL) {
			MALLOC_MSG;
			exit (EXIT_FAILURE);
		}
		pe->duracion_carrera = 0;
		pe->anuales = 0;
		pe->cuatrimestrales = 0;
		pe->total_horas = 0;
		pe->anio_carrera = NULL;

		int salida = plan_parse (pe);
		if (salida == 0) {
			ordenar_anios (pe);
			apuntar_correlativas (pe);
			calcular_tiempos (pe);
		}
		else {
			if (pe)
				free (pe);
			pe = NULL;
			if (salida == 1)
				fprintf (stderr, "Alguna entrada inválida\n");
			else
				fprintf (stderr, "Problemas de memoria u otra cosa\n");
		}
	}
	return pe;
}

void
plan_error (plan_de_estudios *pe, const char *str)
{
	fprintf (stderr, "Error en línea %d: %s\n", plan_lineno, str);
	if (pe && pe->nombre_carrera != NULL)
		printf ("del plan: %s\n", pe->nombre_carrera);
}
