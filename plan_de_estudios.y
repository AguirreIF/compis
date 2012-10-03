%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	#include "funciones_plan.h"

	plan_de_estudios *pe;
	materia_t *m_aux;
	char *id_materia, *nombre_materia;

	extern int yylineno;

	void yyerror(char const *);
%}

%debug
%locations
%define api.pure
%error-verbose
%defines

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
		$carrera = $TEXTO;
		pe->nombre_carrera = strdup($TEXTO);
	};

anio_plan:
	'<'ANIO'>' ANIO_PLAN "</"ANIO'>' {
		$anio_plan = $ANIO_PLAN;
		pe->anio_del_plan = $anio_plan;
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
		$nombre = $TEXTO;
		nombre_materia = strdup($nombre);
	};

anio:
	'<'ANIO'>' DOS_DIGITOS[ANIO_MATERIA] "</"ANIO'>' {
		$anio = $ANIO_MATERIA;
		anio_t *anio_aux = crear_anio_carrera($anio, pe);
		m_aux = crear_materia(anio_aux, id_materia, nombre_materia);
	};

regimen:
	'<'REGIMEN'>' ANUAL "</"REGIMEN'>' {
		$regimen = $ANUAL;
		++pe->anuales;
		m_aux->regimen = strdup($regimen);
		m_aux->duracion = 1;
	}
	|
	'<'REGIMEN'>' CUATRIMESTRAL "</"REGIMEN'>' cuatrimestre {
		$regimen = $CUATRIMESTRAL;
		++pe->cuatrimestrales;
		m_aux->regimen = strdup($regimen);
		m_aux->duracion = 0;
	};

cuatrimestre:
	'<'CUATRIMESTRE'>' PRIMERO "</"CUATRIMESTRE'>' {
		$cuatrimestre = $PRIMERO;
		m_aux->cuatrimestre = strdup($cuatrimestre);
	}
	|
	'<'CUATRIMESTRE'>' SEGUNDO "</"CUATRIMESTRE'>' {
		$cuatrimestre = $SEGUNDO;
		m_aux->cuatrimestre = strdup($cuatrimestre);
	};

horas:
	'<'HORAS'>' DOS_DIGITOS[CANT_HORAS] "</"HORAS'>' {
		$horas = $CANT_HORAS;
		m_aux->horas = $horas;
		pe->total_horas += (strcmp(m_aux->regimen, "anual") ? \
			m_aux->horas * SEMANAS_POR_CUATRIMESTRE :         \
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

int main(void) {

	inicializar(&pe);

	int salida = -1;
	salida = yyparse();

	if (salida == 0) {
		pe->anio_carrera = ordenar_anios(pe->anio_carrera);
		apuntar_correlativas(pe);
		calcular_tiempos(pe);
		imprimir_informe(pe);
		ver_gantt(pe);
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

void yyerror(const char *str) {
	fprintf(stderr, "Error en línea %d: %s\n", yylineno, str);
}
