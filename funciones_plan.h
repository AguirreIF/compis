#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "estructuras.h"

// Prepara la estructura del plan de estudios.
void inicializar(plan_de_estudios **);

// Crea estructuras de año dentro del plan de estudios
anio_t *crear_anio_carrera(int, plan_de_estudios *);

// Crea estructuras de materias dentro de año
materia_t *crear_materia(anio_t *, char *, char *);

// Guarda la el ID de la correlativa en el array de materias.
void cargar_correlativa(materia_t *, char *);

// Ordena los años de menor a mayor y las materias
// primero las del 1er cuatrimestre, después las del 2do
// y por último las anuales.
void ordenar_anios(plan_de_estudios *);

// Recibe el ID de una materia y devuelve un apuntador a ella.
materia_t *buscar_materia(plan_de_estudios *, char *);

// Reemplaza los apuntadores a char de la estructura
// de correlativas por apuntadores a las materias.
void apuntar_correlativas(plan_de_estudios *);

// Imprime toda la info del plan
void imprimir_informe(plan_de_estudios *);

// Calcula los cuatrimestres de inicio y fin temprano
void calcular_cit_cft(materia_t *);

// Calcula los cuatrimestres de inicio y fin tardíos
void calcular_cita_cfta(materia_t *, int);

// Calcula los cuatrimestres de inicio temprano y tardío de
// cada materia.
void calcular_tiempos(plan_de_estudios *);


void inicializar(plan_de_estudios **pe) {
	*pe = (plan_de_estudios *) malloc(sizeof(plan_de_estudios));
	(*pe)->duracion_carrera = 0;
	(*pe)->anuales = 0;
	(*pe)->cuatrimestrales = 0;
	(*pe)->total_horas = 0;
	(*pe)->anio_carrera = NULL;
}

anio_t *crear_anio_carrera(int anio, plan_de_estudios *pe) {
	anio_t *anio_aux = pe->anio_carrera;

	// es la primera ejecución
	if (anio_aux == NULL) {
		anio_aux = (pe->anio_carrera = (anio_t *) malloc(sizeof(anio_t)));
		anio_aux->anio = anio;
		anio_aux->anterior = anio_aux->siguiente = NULL;
		anio_aux->materia = NULL;
	}
	else {
		// busca la estructura del año o crea una nueva
		while (1) {
			if (anio_aux->anio == anio) {
				break;
			}
			else if (anio_aux->siguiente != NULL) {
				anio_aux = anio_aux->siguiente;
			}
			else {
				anio_aux->siguiente = (anio_t *) malloc(sizeof(anio_t));
				anio_aux->siguiente->anterior = anio_aux;
				anio_aux = anio_aux->siguiente;
				anio_aux->anio = anio;
				anio_aux->siguiente = NULL;
				anio_aux->materia = NULL;
				break;
			}
		}
	}
	return anio_aux;
}

materia_t *crear_materia(anio_t *anio_aux, char *id_materia, char *nombre_materia) {
	materia_t *m_aux;

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
	m_aux->cant_corr_de = 0;
	m_aux->cit = 0;
	m_aux->cft = 0;
	m_aux->cita = 0;
	m_aux->cfta = 0; m_aux->correlativas = NULL;
	m_aux->correlativa_de = NULL;
	m_aux->siguiente = NULL;

	return m_aux;
}

void cargar_correlativa(materia_t *materia, char *id) {
	materia->correlativas = realloc(materia->correlativas, sizeof(char *) * (materia->cant_corr + 1));
	materia->correlativas[materia->cant_corr] = (char *) malloc(strlen(id) + 1);
	materia->correlativas[materia->cant_corr++] = strdup(id);
}

void ordenar_anios(plan_de_estudios *pe) {
	/*
		  actual
			|   .-------siguiente
			|   |
			v   v
	  .---.---.---.---.
	  |   |   |   |   |<---siguiente siguiente
	  '---'---'---'---'
		^
		|
		|
	 anterior
	*/
	anio_t *nodo = pe->anio_carrera->siguiente, *nodo_aux;
	while (nodo != NULL) {
		nodo_aux = nodo->anterior;
		while (nodo_aux != NULL) {
			if (nodo_aux->anio > nodo->anio) {

				anio_t *p_aux, *p2_aux;

				// nodo anterior
				if (nodo_aux->anterior != NULL)
					nodo_aux->anterior->siguiente = nodo_aux->siguiente;

				// nodo siguiente siguiente
				if (nodo_aux->siguiente->siguiente != NULL)
					nodo_aux->siguiente->siguiente->anterior = nodo_aux;
				p_aux = nodo_aux->siguiente->siguiente;

				// nodo siguiente
				nodo_aux->siguiente->siguiente = nodo_aux;
				nodo_aux->siguiente->anterior = nodo_aux->anterior;

				// nodo actual
				p2_aux = nodo_aux->siguiente;
				nodo_aux->siguiente = p_aux;
				nodo_aux->anterior = p2_aux;
			}
			nodo_aux = nodo_aux->anterior;
		}

		nodo = nodo->siguiente;
	}

	// calcula el último cuatrimestre de cursado
	nodo = pe->anio_carrera;
	while (nodo->siguiente != NULL) {
		nodo = nodo->siguiente;
	}
	materia_t *m_aux = nodo->materia;
	while (m_aux->siguiente != NULL) {
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
				|| (strcmp(m_aux->regimen, "anual") == 0)) {
			pe->duracion_carrera = nodo->anio << 1;
			break;
		}
		else
			pe->duracion_carrera = (nodo->anio << 1) - 1;
	}

	// deja el puntero al comienzo de la lista
	while (pe->anio_carrera->anterior != NULL) {
		pe->anio_carrera = pe->anio_carrera->anterior;
	}
}

void apuntar_correlativas(plan_de_estudios *pe) {
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *materia, *m_aux;
	while (anio_aux != NULL) {
		materia = anio_aux->materia;
		while (materia != NULL) {
			if (materia->cant_corr > 0) {
				int i = 0;
				while (i < materia->cant_corr) {
					m_aux = buscar_materia(pe, materia->correlativas[i]);
					if (m_aux == NULL) {
						printf("Para %s no se encontró la correlativa %s\n", materia->nombre, \
								((materia_t *) materia->correlativas[i])->nombre);
						break;
					}
					// agrego la materia como correlativa
					materia->correlativas = realloc(materia->correlativas, sizeof(materia_t *) * (materia->cant_corr + 1));
					materia->correlativas[i] = (materia_t *) malloc(sizeof(materia_t));
					materia->correlativas[i] = m_aux;
					i++;
					// hago la inversa y agrego la materia como correlativa de
					m_aux->correlativa_de = realloc(m_aux->correlativa_de, sizeof(materia_t *) * (m_aux->cant_corr_de + 1));
					m_aux->correlativa_de[m_aux->cant_corr_de] = materia;
					++m_aux->cant_corr_de;
				}
			}
			materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
}

materia_t *buscar_materia(plan_de_estudios *pe, char *id) {
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *materia;
	while (anio_aux != NULL) {
		materia = anio_aux->materia;
		while (materia != NULL) {
			if (strcmp(materia->id, id) == 0)
				return(materia);
			else
				materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
	return NULL;
}

void calcular_cit_cft(materia_t *m_aux) {
	if (m_aux->cant_corr > 0) {
		// Busca la correlativa con el cft (cuatrimestre de fin temprano) más grande
		int i = 0, mayor_cft = 0;
		while (i < m_aux->cant_corr) {
			if (((materia_t *)m_aux->correlativas[i])->cft == 0)
				calcular_cit_cft((materia_t *)m_aux->correlativas[i]);
			if (((materia_t *)m_aux->correlativas[i])->cft > mayor_cft)
				mayor_cft = ((materia_t *)m_aux->correlativas[i])->cft;
			i++;
		}
		// ese valor + 1 va a ser el cit (cuatrimestre de inicio temprano) de la materia
		m_aux->cit = mayor_cft + 1;
		// Si es del 1er cuatrimestre, solo puede empezar en
		// cuatrimestres impares (comienzo de año)
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cit % 2) == 0))
			m_aux->cit += 1;
		// Si es del 2do cuatrimestre, solo puede empezar en
		// cuatrimestres pares (mitad de año)
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
				&& ((m_aux->cit % 2) != 0))
			m_aux->cit += 1;
		// Si es anual, solo puede empezar en
		// cuatrimestres impares (comienzo de año)
		if ((strcmp(m_aux->regimen, "anual") == 0) && ((m_aux->cit % 2) == 0))
			m_aux->cit += 1;
	}
	// Si no tiene correlativas puede empezar en el 1er o 2do cuatrimestre de la carrera
	else {
		// Si es del primer cuatrimestre o anual puede empezar en el 1er
		// cuatrimestre (comienzo de año)
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
				|| (strcmp(m_aux->regimen, "anual") == 0))
			m_aux->cit = 1;
		// Si es del segundo cuatrimestre puede empezar en el 2do cuatrimestre
		// (mitad del año)
		else
			m_aux->cit = 2;
	}
	m_aux->cft = m_aux->cit + m_aux->duracion;
}

void calcular_cita_cfta(materia_t *m_aux, int cuatrimestre_fin) {
	if (m_aux->cant_corr_de > 0) {

		// Busca la "correlativa de" con el cita (cuatrimestre de inicio tardío) más chico
		int i = 0;
		if (((materia_t *)m_aux->correlativa_de[0])->cita == 0)
			calcular_cita_cfta((materia_t *)m_aux->correlativa_de[0], cuatrimestre_fin);
		int menor_cita = ((materia_t *)m_aux->correlativa_de[0])->cita;
		i++;

		while (i < m_aux->cant_corr_de) {
			if (((materia_t *)m_aux->correlativa_de[i])->cita == 0)
				calcular_cita_cfta((materia_t *)m_aux->correlativa_de[i], cuatrimestre_fin);
			if (((materia_t *)m_aux->correlativa_de[i])->cita < menor_cita)
				menor_cita = ((materia_t *)m_aux->correlativa_de[i])->cita;
			i++;
		}
		// La materia tiene como cfta (cuatrimestre de fin tardío) al cuatrimestre
		// anterior del menor cita (cuatrimestre de inicio tardío) de todas las materias
		// de la cual es correlativa
		m_aux->cfta = menor_cita - 1;
		// Si es del primer cuatrimestre, solo puede terminar en
		// cuatrimestres impares (mitad de año)
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cfta % 2) == 0))
			--m_aux->cfta;
		// Si es del segundo cuatrimestre, solo puede terminar en
		// cuatrimestres pares (fin de año)
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
				&& ((m_aux->cfta % 2) != 0))
			--m_aux->cfta;
		// Si es anual solo puede terminar en cuatrimestres
		// pares (fin de año)
		if ((strcmp(m_aux->regimen, "anual") == 0) && ((m_aux->cfta % 2) != 0))
			--m_aux->cfta;
	}
	// Si no es correlativa de alguna materia, puede terminar al final de la carrera
	else {
		m_aux->cfta = cuatrimestre_fin;
		// Si es del primer cuatrimestre, solo puede terminar
		// en cuatrimestres impares
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cfta % 2) == 0))
			--m_aux->cfta;
	}
	m_aux->cita = m_aux->cfta - m_aux->duracion;
	m_aux->holgura = m_aux->cita - m_aux->cit; // (inicio tardio - inicio temprano)
}

void calcular_tiempos(plan_de_estudios *pe) {
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;
	int cuatrimestre_fin = pe->duracion_carrera; // guarda el último cuatrimestre de la carrera

	// calculo los cuatrimestres de inicio y fin tempranos
	// No pregunto por NULL para dejar el puntero en el último año
	// es el punto de partido del siguiente recorrido
	while (1) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			calcular_cit_cft(m_aux);
			m_aux = m_aux->siguiente;
		}
		if (anio_aux->siguiente == NULL)
			break;
		anio_aux = anio_aux->siguiente;
	}

	// calculo los cuatrimestres de inicio y fin tardíos
	while (anio_aux != NULL) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			calcular_cita_cfta(m_aux, cuatrimestre_fin);
			m_aux = m_aux->siguiente;
		}
		anio_aux = anio_aux->anterior;
	}
}

void imprimir_informe(plan_de_estudios *pe) {
	printf("Carrera: %s\n", pe->nombre_carrera);
	printf("Año del plan: %d\n", pe->anio_del_plan);
	printf("Cuatrimestres de cursado: %d\n", pe->duracion_carrera);
	printf("\nMATERIAS: %d\n", pe->anuales + pe->cuatrimestrales);
	printf("\tCuatrimestrales: %d\n", pe->cuatrimestrales);
	printf("\t\t\tAnuales: %d\n", pe->anuales);
	printf("\nTotal de horas: %d\n", pe->total_horas);

	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;

	while (anio_aux != NULL) {

		m_aux = anio_aux->materia;
		printf("Año: %d\n", anio_aux->anio);

		while (m_aux != NULL) {

			if (strcmp(m_aux->regimen, "anual") == 0)
				printf("\n\t%s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->horas);
			else
				printf("\n\t%s - %s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->cuatrimestre, m_aux->horas);

			printf("\t\tCuat. de in. temp.: %d\n\t\tCuat. de fin temp.: %d\n", m_aux->cit, m_aux->cft);
			printf("\t\tCuat. de in. tardío: %d\n\t\tCuat. de fin tardío: %d\n", m_aux->cita, m_aux->cfta);
			printf("\t\tHolgura: %d\n", m_aux->holgura);

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

	anio_aux = pe->anio_carrera;
	int cant = 0;

	printf("\nMaterias críticas\n");
	while (anio_aux != NULL) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			if (m_aux->holgura == 0) {
				cant++;
				printf("\t%s\n", m_aux->nombre);
			}
			m_aux = m_aux->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
	printf("\n\tHay %d materias críticas\n", cant);
}
