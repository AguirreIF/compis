#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEMANAS_POR_CUATRIMESTRE 15

typedef struct ruta_critica {
	int cantidad;
	struct materia_t **materias;
} ruta_critica;

typedef struct materia_t {
	char *nombre;
	char *id;
	char *regimen;
	char *cuatrimestre;
	int horas;
	int cant_corr;
	int cant_corr_de;
	int cit;
	int cft;
	int cita;
	int cfta;
	int duracion;
	void **correlativas;
	struct materia_t **correlativa_de;
	struct materia_t *anterior;
	struct materia_t *siguiente;
} materia_t;

typedef struct anio_t {
	int anio;
	materia_t *materia;
	struct anio_t *anterior;
	struct anio_t *siguiente;
} anio_t;

typedef struct plan_de_estudios {
	char *carrera;
	int plan_anio;
	int duracion_carrera;
	int anuales;
	int cuatrimestrales;
	int total_horas;
	anio_t *anio_carrera;
} plan_de_estudios;

// Prepara la estructura del plan de estudios.
void inicializar(plan_de_estudios **);

// Guarda la el ID de la correlativa en el array
// de materias.
void cargar_correlativa(materia_t *, char *);

// Reemplaza los apuntadores a char de la estructura
// de correlativas a apuntadores a las materias.
void apuntar_correlativas(plan_de_estudios *);

// Recibe el ID de una materia y devuelve un apuntador a ella.
materia_t *buscar_materia(plan_de_estudios *, char *);

// Calcula los cuatrimestres de inicio temprano y tardío de
// cada materia. Luego calcula la ruta crítica.
void calcular_ruta_critica(plan_de_estudios *, ruta_critica **);

// Imprime una tabla con todas las materias y sus
// correlatividades
void ver_gantt(plan_de_estudios *);

// Imprime toda la información del plan de estudios
void imprimir_informe(plan_de_estudios *, ruta_critica *);


// prepara la estructura del plan de estudios
void inicializar(plan_de_estudios **pe) {
	*pe = (plan_de_estudios *) malloc(sizeof(plan_de_estudios));
	(*pe)->duracion_carrera = 0;
	(*pe)->anuales = 0;
	(*pe)->cuatrimestrales = 0;
	(*pe)->total_horas = 0;
	(*pe)->anio_carrera = NULL;
}

void cargar_correlativa(materia_t *materia, char *id) {
	materia->correlativas = realloc(materia->correlativas, sizeof(char *) * (materia->cant_corr + 1));
	materia->correlativas[materia->cant_corr] = (char *) malloc(strlen(id) + 1);
	materia->correlativas[materia->cant_corr++] = strdup(id);
}

// reemplaza los punteros a char del array de correlativas a punteros a cada meteria
void apuntar_correlativas(plan_de_estudios *pe) {
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *materia, *m;
	while (anio_aux != NULL) {
		materia = anio_aux->materia;
		while (materia != NULL) {
			if (materia->cant_corr > 0) {
				int i = 0;
				while (i < materia->cant_corr) {
					m = buscar_materia(pe, materia->correlativas[i]);
					// agrego la materia como correlativa
					materia->correlativas = realloc(materia->correlativas, sizeof(materia_t *) * (materia->cant_corr + 1));
					materia->correlativas[i] = (materia_t *) malloc(sizeof(materia_t));
					materia->correlativas[i] = m;
					i++;
					// agrego la materia como correlativa de
					m->correlativa_de = realloc(m->correlativa_de, sizeof(materia_t *) * (m->cant_corr_de + 1));
					m->correlativa_de[m->cant_corr_de] = materia;
					++m->cant_corr_de;
				}
			}
			materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
	free(anio_aux);
	free(materia);
}

// devuelve un apuntador a la materia identificada por *id
materia_t *buscar_materia(plan_de_estudios *pe, char *id) {
	anio_t *anio_aux = pe->anio_carrera;

	while (anio_aux != NULL) {

		materia_t *materia = anio_aux->materia;

		while (materia != NULL) {
			if (strcmp(materia->id, id) == 0)
				return(materia);
			else
				materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
	free(anio_aux);
}

void calcular_ruta_critica(plan_de_estudios *pe, ruta_critica **ruta_c) {
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux = NULL;
	int cuatrimestre_fin = 0;

	// calculo los cuatrimestres de inicio y fin tempranos
	while (1) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			if (m_aux->cant_corr > 0) {
				int i = 0;
				int mayor_cft = ((materia_t *)m_aux->correlativas[i])->cft;
				i++;
				while (i < m_aux->cant_corr) {
					if (((materia_t *)m_aux->correlativas[i])->cft > mayor_cft)
						mayor_cft = ((materia_t *)m_aux->correlativas[i])->cft;
					i++;
				}
				m_aux->cit = mayor_cft + 1;
				// Si es del primer cuatrimestre, solo puede empezar en
				// cuatrimestres impares (comienzo de año)
				if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
						&& ((m_aux->cit % 2) == 0))
					m_aux->cit = m_aux->cit + 1;
				// Si es del segundo cuatrimestre, solo puede empezar en
				// cuatrimestres pares (mitad de año)
				if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
						&& ((m_aux->cit % 2) != 0))
					m_aux->cit = m_aux->cit + 1;
				// Si es anual, solo puede empezar en
				// cuatrimestres impares (comienzo de año)
				if ((strcmp(m_aux->regimen, "anual") == 0) && ((m_aux->cit % 2) == 0))
					m_aux->cit = m_aux->cit + 1;
			}
			else {
				// Si es del segundo cuatrimestre, solo puede empezar en
				// cuatrimestres pares (mitad de año)
				if ((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0))
					m_aux->cit = 2;
				else
					m_aux->cit = 1;
			}
			m_aux->cft = m_aux->cit + m_aux->duracion;

			if (m_aux->cft > cuatrimestre_fin)
				cuatrimestre_fin = m_aux->cft;

			m_aux = m_aux->siguiente;
		}

		if (anio_aux->siguiente != NULL)
			anio_aux = anio_aux->siguiente;
		else
			break;
	}

	// calculo los cuatrimestres de inicio y fin tardíos
	while (1) {
		m_aux = anio_aux->materia;
		while (m_aux->siguiente != NULL)
			m_aux = m_aux->siguiente;
		while (m_aux != NULL) {
			if (m_aux->cant_corr_de > 0) {
				int i = 0;
				int menor_cita = ((materia_t *)m_aux->correlativa_de[i])->cft;
				i++;
				while (i < m_aux->cant_corr_de) {
					if (((materia_t *)m_aux->correlativa_de[i])->cita < menor_cita)
						menor_cita = ((materia_t *)m_aux->correlativa_de[i])->cita;
					i++;
				}
				m_aux->cfta = menor_cita - 1;
				// Si es del primer cuatrimestre, solo puede terminar en
				// cuatrimestres impares (mitad de año)
				if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
						&& ((m_aux->cfta % 2) == 0))
					--m_aux->cfta;
				// Si es del segundo cuatrimestre, solo puede terminar en
				// cuatrimestres pares (fin de año)
				if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
						&& ((m_aux->cit % 2) != 0))
					--m_aux->cfta;
				// Si es anual solo puede terminar en cuatrimestres
				// pares (fin de año)
				if ((strcmp(m_aux->regimen, "anual") == 0) && ((m_aux->cit % 2) == 0))
					--m_aux->cfta;
			}
			else {
				m_aux->cfta = cuatrimestre_fin;
				// Si es del primer cuatrimestre, solo puede terminar en
				// cuatrimestres impares (mitad de año)
				if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
						&& ((m_aux->cfta % 2) == 0))
					--m_aux->cfta;
			}
			m_aux->cita = m_aux->cfta - m_aux->duracion;
			m_aux = m_aux->anterior;
		}
		if (anio_aux == pe->anio_carrera)
			break;
		anio_aux = anio_aux->anterior;
	}

	// calculo las materias críticas
	*ruta_c = (ruta_critica *) malloc(sizeof(ruta_critica));
	(*ruta_c)->cantidad = 0;
	while (anio_aux != NULL) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			if (((m_aux->cft - m_aux->cfta) == 0) && ((m_aux->cft - m_aux->cfta) == 0)) {
				(*ruta_c)->materias = realloc((*ruta_c)->materias, sizeof(materia_t *) * ((*ruta_c)->cantidad + 1));
				(*ruta_c)->materias[(*ruta_c)->cantidad] = m_aux;
				++(*ruta_c)->cantidad;
			}
			m_aux = m_aux->siguiente;;
		}
		anio_aux = anio_aux->siguiente;
	}
}

void imprimir_informe(plan_de_estudios *pe, ruta_critica *ruta_c) {
	printf("Carrera: %s\n", pe->carrera);
	printf("Año del plan: %d\n", pe->plan_anio);
	printf("Años de cursado: %d\n", pe->duracion_carrera);
	printf("\nMATERIAS: %d\n", pe->anuales + pe->cuatrimestrales);
	printf("\tCuatrimestrales: %d\n", pe->cuatrimestrales);
	printf("\t\t\tAnuales: %d\n", pe->anuales);

	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;

	while (anio_aux != NULL) {

		m_aux = anio_aux->materia;
		printf("Año: %d\n", anio_aux->anio);

		while (m_aux != NULL) {

			pe->total_horas += (strcmp(m_aux->regimen, "anual") ? \
				m_aux->horas * SEMANAS_POR_CUATRIMESTRE :         \
				m_aux->horas * SEMANAS_POR_CUATRIMESTRE * 2);

			if (strcmp(m_aux->regimen, "anual") == 0)
				printf("\n\t%s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->horas);
			else
				printf("\n\t%s - %s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->cuatrimestre, m_aux->horas);

			printf("\t\tCuat. de in. temp.: %d\n\t\tCuat. de fin temp.: %d\n", m_aux->cit, m_aux->cft);
			printf("\t\tCuat. de in. tardío: %d\n\t\tCuat. de fin tardío: %d\n", m_aux->cita, m_aux->cfta);

			if (m_aux->cant_corr_de > 0) {
				printf("\t\tEs correlativa de: %d\n", m_aux->cant_corr_de);

				int i = 0;

				while (i < m_aux->cant_corr_de) {
					printf("\t\t\t(%s) %s\n", ((materia_t *)m_aux->correlativa_de[i])->id, \
							((materia_t *)m_aux->correlativa_de[i])->nombre);
					i++;
				}
			}

			if (m_aux->cant_corr > 0) {
				printf("\t\tTiene como correlativas a: %d\n", m_aux->cant_corr);

				int i = 0;

				while (i < m_aux->cant_corr) {
					printf("\t\t\t(%s) %s\n", ((materia_t *)m_aux->correlativas[i])->id, \
							((materia_t *)m_aux->correlativas[i])->nombre);
					i++;
				}
			}

			m_aux = m_aux->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}

	printf("\nTotal de horas: %d\n", pe->total_horas);

	int i = 0;
	printf("\nMaterias críticas: %d\n", ruta_c->cantidad);
	while(i < ruta_c->cantidad) {
		printf("\t%s\n", ruta_c->materias[i]->nombre);
		i++;
	}

	free(anio_aux);
	free(m_aux);
}

void ver_gantt(plan_de_estudios *pe) {
	anio_t *anio_aux = pe->anio_carrera;

	printf("\n | Materia | Correlativas | Duración |\n");
	printf(" -------------------------------------\n");

	while (anio_aux != NULL) {

		while (anio_aux->materia != NULL) {

			printf(" |%7s  |", anio_aux->materia->id);

			if (anio_aux->materia->cant_corr > 0) {
				int i = 0;
				while (i < anio_aux->materia->cant_corr)
					printf("%s,", ((materia_t *)anio_aux->materia->correlativas[i++])->id);
			}
			else
				printf("|      -      |");

			printf("%d\t|\n", anio_aux->materia->duracion);

			anio_aux->materia = anio_aux->materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
}
