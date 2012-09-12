%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "funciones.h"

	plan_de_estudios *pe;
	anio_t *anio_aux;
	materia_t *m_aux;
	ruta_critica *ruta_c;
	char *id_materia, *nombre_materia;

	extern int yylineno;

	void yyerror(char const *);
%}

%verbose
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
		pe->carrera = strdup($TEXTO);
	};

anio_plan:
	'<'ANIO'>' ANIO_PLAN "</"ANIO'>' {
		$anio_plan = $ANIO_PLAN;
		pe->plan_anio = $ANIO_PLAN;
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
		id_materia = strdup($TEXTO_ID);
	};

nombre:
	'<'NOMBRE'>' TEXTO "</"NOMBRE'>' {
		$nombre = $TEXTO;
		nombre_materia = strdup($TEXTO);
	};

anio:
	'<'ANIO'>' DOS_DIGITOS[ANIO_MATERIA] "</"ANIO'>' {
		$anio = $ANIO_MATERIA;

		if ($ANIO_MATERIA > pe->duracion_carrera)
			pe->duracion_carrera = $ANIO_MATERIA;
		
		anio_aux = pe->anio_carrera;

		// es la primera ejecución
		if (anio_aux == NULL) {
			anio_aux = (pe->anio_carrera = (anio_t *) malloc(sizeof(anio_t)));
			anio_aux->anio = $ANIO_MATERIA;
			anio_aux->anterior = anio_aux->siguiente = NULL;
			anio_aux->materia = NULL;
		}
		else {
			// busca la estructura del año o crea una nueva
			while (1) {
				if (anio_aux->anio == $ANIO_MATERIA) {
					break;
				}
				else if (anio_aux->siguiente != NULL) {
						anio_aux = anio_aux->siguiente;
				}
				else {
					anio_aux->siguiente = (anio_t *) malloc(sizeof(anio_t));
					anio_aux->siguiente->anterior = anio_aux;
					anio_aux = anio_aux->siguiente;
					anio_aux->anio = $ANIO_MATERIA;
					anio_aux->siguiente = NULL;
					anio_aux->materia = NULL;
					break;
				}
			}
		}

		// ya había materias cargadas en el año
		if (anio_aux->materia != NULL) {
			if (anio_aux->materia->siguiente == NULL) {
				anio_aux->materia->siguiente = (materia_t *) malloc(sizeof(materia_t));
				anio_aux->materia->siguiente->anterior = anio_aux->materia;
				m_aux = anio_aux->materia->siguiente;
			}
			else {
				m_aux = anio_aux->materia;
				while(m_aux->siguiente != NULL)
					m_aux = m_aux->siguiente;
				m_aux->siguiente = (materia_t *) malloc(sizeof(materia_t));
				m_aux->siguiente->anterior = m_aux;
				m_aux = m_aux->siguiente;
			}
		}
		else {
			anio_aux->materia = (materia_t *) malloc(sizeof(materia_t));
			anio_aux->materia->anterior = NULL;
			m_aux = anio_aux->materia;
		}

		m_aux->id = strdup(id_materia);
		m_aux->nombre = strdup(nombre_materia);
		m_aux->cant_corr = 0;
		m_aux->siguiente = NULL;
	};

regimen:
	'<'REGIMEN'>' ANUAL "</"REGIMEN'>' {
		$regimen = $ANUAL;
		++pe->anuales;
		m_aux->regimen = strdup($ANUAL);
		m_aux->duracion = 1;
	}
	|
	'<'REGIMEN'>' CUATRIMESTRAL "</"REGIMEN'>' cuatrimestre {
		$regimen = $CUATRIMESTRAL;
		++pe->cuatrimestrales;
		m_aux->regimen = strdup($CUATRIMESTRAL);
		m_aux->duracion = 0;
	};

cuatrimestre:
	'<'CUATRIMESTRE'>' PRIMERO "</"CUATRIMESTRE'>' {
		$cuatrimestre = $PRIMERO;
		m_aux->cuatrimestre = strdup($PRIMERO);
	}
	|
	'<'CUATRIMESTRE'>' SEGUNDO "</"CUATRIMESTRE'>' {
		$cuatrimestre = $SEGUNDO;
		m_aux->cuatrimestre = strdup($SEGUNDO);
	};

horas:
	'<'HORAS'>' DOS_DIGITOS[CANT_HORAS] "</"HORAS'>' {
		$horas = $CANT_HORAS;
		m_aux->horas = $CANT_HORAS;
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
	int salida;

	// crea la estructura de plan de estudios e inicializa los campos
	inicializar(&pe);

	salida = yyparse();

	if (salida == 0) {
		apuntar_correlativas(pe);
		calcular_ruta_critica(pe, &ruta_c);
		imprimir_informe(pe, ruta_c);
		ver_gantt(pe);
	}
	else if (salida == 1) {
		printf("Alguna entrada inválida\n");
		return 1;
	}
	else {
		printf("Problemas de memoria\n");
		return 2;
	}

	free(pe);
	free(anio_aux);
	free(m_aux);

	return salida;
}

void yyerror(const char *str) {
	fprintf(stderr, "Error en línea %d: %s\n", yylineno, str);
}
