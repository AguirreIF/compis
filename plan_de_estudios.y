%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>

	#include "funciones_plan.h"

	extern int errno;

	plan_de_estudios *pe;
	materia_t *m_aux;
	char *id_materia, *nombre_materia;

	extern int plan_lineno;
	extern FILE *plan_in;

	void plan_error(char const *);
%}

%debug
%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
%locations
%name-prefix "plan_"

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
		pe->nombre_carrera = strdup($TEXTO);
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
		id_materia = strdup($id);
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		nombre_materia = strdup($TEXTO);
	};

anio:
	'<'ANIO'>' DOS_DIGITOS[ANIO_MATERIA] "</"ANIO'>' {
		anio_t *anio_aux = crear_anio_carrera($ANIO_MATERIA, pe);
		m_aux = crear_materia(anio_aux, id_materia, nombre_materia);
	};

regimen:
	'<'REGIMEN'>' ANUAL "</"REGIMEN'>' {
		++pe->anuales;
		m_aux->regimen = strdup($ANUAL);
		m_aux->duracion = 1;
	}
	|
	'<'REGIMEN'>' CUATRIMESTRAL "</"REGIMEN'>' cuatrimestre {
		++pe->cuatrimestrales;
		m_aux->regimen = strdup($CUATRIMESTRAL);
		m_aux->duracion = 0;
	};

cuatrimestre:
	'<'CUATRIMESTRE'>' PRIMERO "</"CUATRIMESTRE'>' {
		m_aux->cuatrimestre = strdup($PRIMERO);
	}
	|
	'<'CUATRIMESTRE'>' SEGUNDO "</"CUATRIMESTRE'>' {
		m_aux->cuatrimestre = strdup($SEGUNDO);
	};

horas:
	'<'HORAS'>' DOS_DIGITOS[CANT_HORAS] "</"HORAS'>' {
		m_aux->horas = $CANT_HORAS;
		pe->total_horas += (strcmp(m_aux->regimen, "anual") ? \
			m_aux->horas * SEMANAS_POR_CUATRIMESTRE :		  \
			m_aux->horas * SEMANAS_POR_CUATRIMESTRE * 2);
	};

correlativas:
	/* vacio */
	|
	'<'CORRELATIVAS'>' lista_correlativas "</"CORRELATIVAS'>';

lista_correlativas:
	id lista_correlativas {
		cargar_correlativa(m_aux, $id);
	}
	|
	id {
		cargar_correlativa(m_aux, $id);
	};
%%

int main(int argc, char **argv) {

	int opcion;
	char *plan_xml = NULL;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	if (argc == 1) {
		printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alumno1.xml> <alumno2.xml>...]\n", argv[0]);
		return 0;
	}
	else
		while ((opcion = getopt(argc, argv, "p:a:h")) != -1)
			switch (opcion) {
				case 'p':
					plan_xml = strdup(optarg);
					break;
				case 'a':
					break;
				case 'h':
					printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alumno1.xml> <alumno2.xml>...]\n", argv[0]);
					return 0;
				case '?':
					if (optopt == 'p')
						fprintf (stderr, "La opción `-%c' requiere el XML de un plan de estudios\n", optopt);
					else if (optopt == 'a') {
						fprintf (stderr, "La opción `-%c' requiere el XML de al menos un alumno\n", optopt);
						continue;
					}
					else if (isprint (optopt))
						fprintf (stderr, "Opción desconocida `-%c'\n", optopt);
					else
						fprintf (stderr, "Carácter de opción desconocido `\\x%x'\n", optopt);
					return -1;
				default:
					fprintf (stderr, "Error desconocido `\\x%x'\n", optopt);
					return -1;
			}

	// si quedaron argumentos no reconocidos
	if (argc > optind) {
		if ((argc - optind) == 1)
			printf ("Argumento desconocido: %s\n", argv[optind]);
		else {
			printf ("Argumentos desconocidos: ");
			for (opcion = optind; opcion < argc; opcion++)
				printf ("%s ", argv[opcion]);
			puts("");
		}
		return -1;
	}

	if (plan_xml == NULL) {
		printf("Debe especificar un plan de estudios con -p o --plan\n");
		return -1;
	}
	else if (strcmp(plan_xml, "-") == 0)
		plan_in = (FILE *) 0;
	else {
		plan_in = fopen(plan_xml, "r");
		if (plan_in == NULL) {
			printf("Error al intentar abrir el archivo `%s': %s\n", plan_xml, strerror(errno));
			return -1;
		}
	}
	plan_xml = NULL;

	inicializar(&pe);

	int salida = -1;
	salida = plan_parse();

	if (salida == 0) {
		ordenar_anios(pe);
		apuntar_correlativas(pe);
		calcular_tiempos(pe);
		imprimir_informe(pe);
	}
	else if (salida == 1) {
		printf("Alguna entrada inválida\n");
	}
	else {
		printf("Problemas de memoria u otra cosa\n");
	}

	if (pe)
		free(pe);
	if (m_aux)
		free(m_aux);

	return salida;
}

void plan_error(const char *str) {
	fprintf(stderr, "Error en línea %d: %s\n", plan_lineno, str);
}
