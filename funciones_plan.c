#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gvc.h>
#include "funciones_plan.h"

anio_t *
crear_anio_carrera (const int anio, plan_de_estudios *pe)
{
	anio_t *anio_aux = pe->anio_carrera;

	// es la primera ejecución
	if (anio_aux == NULL) {
		anio_aux = (pe->anio_carrera = (anio_t *) malloc (sizeof (anio_t)));
		anio_aux->anio = anio;
		anio_aux->anterior = anio_aux->siguiente = NULL;
		anio_aux->materia = NULL;
	}
	else {
		// busca la estructura del año o crea una nueva
		while (1) {
			if (anio_aux->anio == anio)
				break;
			else if (anio_aux->siguiente != NULL)
				anio_aux = anio_aux->siguiente;
			else {
				anio_aux->siguiente = (anio_t *) malloc (sizeof (anio_t));
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

materia_t *
crear_materia (anio_t *anio_aux, const char *id_materia, const char *nombre_materia)
{
	materia_t *m_aux;
	// ya había materias cargadas en el año
	if (anio_aux->materia != NULL) {
		if (anio_aux->materia->siguiente == NULL) {
			anio_aux->materia->siguiente = (materia_t *) malloc (sizeof (materia_t));
			anio_aux->materia->siguiente->anterior = anio_aux->materia;
			m_aux = anio_aux->materia->siguiente;
		}
		else {
			m_aux = anio_aux->materia;
			while (m_aux->siguiente != NULL)
				m_aux = m_aux->siguiente;
			m_aux->siguiente = (materia_t *) malloc (sizeof (materia_t));
			m_aux->siguiente->anterior = m_aux;
			m_aux = m_aux->siguiente;
		}
	}
	else {
		anio_aux->materia = (materia_t *) malloc (sizeof (materia_t));
		anio_aux->materia->anterior = NULL;
		m_aux = anio_aux->materia;
	}
	m_aux->id = strdup (id_materia);
	m_aux->nombre = strdup (nombre_materia);
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

void
cargar_correlativa (materia_t *materia, const char *id)
{
	materia->correlativas = realloc (materia->correlativas, sizeof (char *) * (materia->cant_corr + 1));
	materia->correlativas[materia->cant_corr] = (char *) malloc (strlen (id) + 1);
	materia->correlativas[materia->cant_corr++] = strdup (id);
}

void
ordenar_anios (plan_de_estudios *pe)
{
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
	while (nodo->siguiente != NULL)
		nodo = nodo->siguiente;
	materia_t *m_aux = nodo->materia;
	while (m_aux->siguiente != NULL) {
		if (((strcmp(m_aux->regimen, "cuatrimestral") == 0) && (strcmp(m_aux->cuatrimestre, "segundo") == 0)) \
				|| (strcmp(m_aux->regimen, "anual") == 0)) {
			pe->duracion_carrera = nodo->anio << 1; // << 1 divide por 2
			break;
		}
		else
			pe->duracion_carrera = (nodo->anio << 1) - 1;
	}

	// deja el puntero al comienzo de la lista
	while (pe->anio_carrera->anterior != NULL)
		pe->anio_carrera = pe->anio_carrera->anterior;
}

void
apuntar_correlativas (plan_de_estudios *pe)
{
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *materia, *m_aux;
	while (anio_aux != NULL) {
		materia = anio_aux->materia;
		while (materia != NULL) {
			if (materia->cant_corr > 0) {
				int i = 0;
				while (i < materia->cant_corr) {
					m_aux = buscar_materia (pe, materia->correlativas[i]);
					if (m_aux == NULL) {
						printf ("Para %s no se encontró la correlativa %s\n", materia->nombre, \
								((materia_t *) materia->correlativas[i])->nombre);
						break;
					}
					// agrego la materia como correlativa
					materia->correlativas = realloc (materia->correlativas, sizeof (materia_t *) * (materia->cant_corr + 1));
					materia->correlativas[i] = (materia_t *) malloc (sizeof (materia_t));
					materia->correlativas[i] = m_aux;
					i++;
					// hago la inversa y agrego la materia como correlativa de
					m_aux->correlativa_de = realloc (m_aux->correlativa_de, sizeof (materia_t *) * (m_aux->cant_corr_de + 1));
					m_aux->correlativa_de[m_aux->cant_corr_de] = materia;
					++m_aux->cant_corr_de;
				}
			}
			materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
}

materia_t *
buscar_materia (const plan_de_estudios *pe, const char *id)
{
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *materia;
	while (anio_aux != NULL) {
		materia = anio_aux->materia;
		while (materia != NULL) {
			if (strcmp (materia->id, id) == 0)
				return materia;
			else
				materia = materia->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}
	return NULL;
}

void
calcular_cit_cft (materia_t *m_aux)
{
	if (m_aux->cant_corr > 0) {
		// Busca la correlativa con el cft (cuatrimestre de fin temprano) más grande
		int i = 0, mayor_cft = 0;
		while (i < m_aux->cant_corr) {
			if (((materia_t *)m_aux->correlativas[i])->cft == 0)
				calcular_cit_cft ((materia_t *)m_aux->correlativas[i]);
			if (((materia_t *)m_aux->correlativas[i])->cft > mayor_cft)
				mayor_cft = ((materia_t *)m_aux->correlativas[i])->cft;
			i++;
		}
		// ese valor + 1 va a ser el cit (cuatrimestre de inicio temprano) de la materia
		m_aux->cit = mayor_cft + 1;
		// Si es del 1er cuatrimestre, solo puede empezar en
		// cuatrimestres impares (comienzo de año)
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cit % 2) == 0))
			m_aux->cit += 1;
		// Si es del 2do cuatrimestre, solo puede empezar en
		// cuatrimestres pares (mitad de año)
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "segundo") == 0)) \
				&& ((m_aux->cit % 2) != 0))
			m_aux->cit += 1;
		// Si es anual, solo puede empezar en
		// cuatrimestres impares (comienzo de año)
		if ((strcmp (m_aux->regimen, "anual") == 0) && ((m_aux->cit % 2) == 0))
			m_aux->cit += 1;
	}
	// Si no tiene correlativas puede empezar en el 1er o 2do cuatrimestre de la carrera
	else {
		// Si es del primer cuatrimestre o anual puede empezar en el 1er
		// cuatrimestre (comienzo de año)
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "primero") == 0)) \
				|| (strcmp (m_aux->regimen, "anual") == 0))
			m_aux->cit = 1;
		// Si es del segundo cuatrimestre puede empezar en el 2do cuatrimestre
		// (mitad del año)
		else
			m_aux->cit = 2;
	}
	m_aux->cft = m_aux->cit + m_aux->duracion;
}

void
calcular_cita_cfta (materia_t *m_aux, const int cuatrimestre_fin)
{
	if (m_aux->cant_corr_de > 0) {
		// Busca la "correlativa de" con el cita (cuatrimestre de inicio tardío) más chico
		int i = 0;
		if (((materia_t *)m_aux->correlativa_de[0])->cita == 0)
			calcular_cita_cfta ((materia_t *)m_aux->correlativa_de[0], cuatrimestre_fin);
		int menor_cita = ((materia_t *)m_aux->correlativa_de[0])->cita;
		i++;

		while (i < m_aux->cant_corr_de) {
			if (((materia_t *)m_aux->correlativa_de[i])->cita == 0)
				calcular_cita_cfta ((materia_t *)m_aux->correlativa_de[i], cuatrimestre_fin);
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
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cfta % 2) == 0))
			--m_aux->cfta;
		// Si es del segundo cuatrimestre, solo puede terminar en
		// cuatrimestres pares (fin de año)
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "segundo") == 0)) \
				&& ((m_aux->cfta % 2) != 0))
			--m_aux->cfta;
		// Si es anual solo puede terminar en cuatrimestres
		// pares (fin de año)
		if ((strcmp (m_aux->regimen, "anual") == 0) && ((m_aux->cfta % 2) != 0))
			--m_aux->cfta;
	}
	// Si no es correlativa de alguna materia, puede terminar al final de la carrera
	else {
		m_aux->cfta = cuatrimestre_fin;
		// Si es del primer cuatrimestre, solo puede terminar
		// en cuatrimestres impares
		if (((strcmp (m_aux->regimen, "cuatrimestral") == 0) && (strcmp (m_aux->cuatrimestre, "primero") == 0)) \
				&& ((m_aux->cfta % 2) == 0))
			--m_aux->cfta;
	}
	m_aux->cita = m_aux->cfta - m_aux->duracion;
	m_aux->holgura = m_aux->cita - m_aux->cit; // (inicio tardio - inicio temprano)
}

void
calcular_tiempos (plan_de_estudios *pe)
{
	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;
	int cuatrimestre_fin = pe->duracion_carrera; // guarda el último cuatrimestre de la carrera

	// calculo los cuatrimestres de inicio y fin tempranos
	// No pregunto por NULL para dejar el puntero en el último año
	// es el punto de partido del siguiente recorrido
	while (1) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {
			calcular_cit_cft (m_aux);
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
			calcular_cita_cfta (m_aux, cuatrimestre_fin);
			m_aux = m_aux->siguiente;
		}
		anio_aux = anio_aux->anterior;
	}
}

void
graficar_plan (const plan_de_estudios *pe, const char *archivo)
{
	// colores de relleno para las materias críticas
	char *colores[5] = {"#F0F015FF", "#336633ff", "#2B0000FF", "#3636F4FF", "#EC2F3AFF"};
	// colores de relleno para las materias no críticas
	// tienen un poco de canal alfa (transparencia)
	char *coloresa[5] = {"#F0F015D8", "#336633D8", "#2B0000D8", "#3636F4D8", "#EC2F3AD8"};
	// el color de la fuente para cada color de relleno
	char *coloresf[5] = {"#000000", "#FFFFFF", "#FFFFFF", "#FFFFFF", "#FFFFFF"};
	char *char_aux = (char *) malloc (strlen (pe->nombre_carrera));

	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;

	GVC_t *gvc;

	Agraph_t *g, *ref;
	Agnode_t *n, *m, *inicio, *fin;
	Agedge_t *e;

	/* Inicializa el contexto */
	gvc = gvContext ();

	/* Crea un gráfico dirigido estricto */
	g = agopen ("plan", AGDIGRAPHSTRICT);
	agsafeset (g, "pad", "1.5", ""); 
	agsafeset (g, "fontsize", "70", ""); 
	sprintf (char_aux, "%s (%d)", acortar_nombre (pe->nombre_carrera, 40), pe->anio_del_plan);
	agsafeset (g, "label", char_aux, ""); 
	agsafeset (g, "labelfontsize", "4", ""); 
	agsafeset (g, "labelloc", "t", ""); 
	agsafeset (g, "rankdir", "LR", ""); 
	agsafeset (g, "ranksep", "2.2", ""); 
	agsafeset (g, "clusterrank", "local", ""); 

	// Crea los nodos de INICIO y FIN
	inicio = agnode (g, "INICIO");
	agsafeset (inicio, "shape", "circle", ""); 
	agsafeset (inicio, "fixedsize", "true", ""); 
	agsafeset (inicio, "fontsize", "17", ""); 
	agsafeset (inicio, "width", "1.3", ""); 
	agsafeset (inicio, "height", "1.3", ""); 
	fin = agnode (g, "FIN");
	agsafeset (fin, "shape", "circle", ""); 
	agsafeset (fin, "fixedsize", "true", ""); 
	agsafeset (fin, "fontsize", "17", ""); 
	agsafeset (fin, "width", "1.3", ""); 
	agsafeset (fin, "height", "1.3", ""); 

	while (anio_aux != NULL) {
		m_aux = anio_aux->materia;
		while (m_aux != NULL) {

			n = agnode (g, acortar_nombre (m_aux->nombre, 10));
			agsafeset (n, "style", "filled", ""); 
			agsafeset (n, "color", "white", ""); 
			agsafeset (n, "margin", "0.2,0.2", ""); 

			if (m_aux->holgura == 0) {
				agsafeset (n, "fillcolor", colores[anio_aux->anio - 1], ""); 
				agsafeset (n, "color", "black", ""); 
			}
			else
				agsafeset (n, "fillcolor", coloresa[anio_aux->anio - 1], ""); 

			agsafeset (n, "fontcolor", coloresf[anio_aux->anio - 1], ""); 

			if (m_aux->cant_corr_de > 0) {
				int i = 0;
				while (i < m_aux->cant_corr_de) {
					m = agnode (g, acortar_nombre (((materia_t *)m_aux->correlativa_de[i])->nombre, 10));
					e = agedge (g, n, m);
					if ((m_aux->holgura == 0) && ((materia_t *)m_aux->correlativa_de[i])->holgura == 0) {
						agsafeset (e, "color", "red", ""); 
						agsafeset (e, "penwidth", "3", ""); 
					}
					else {
						agsafeset (e, "color", "#828282", ""); 
						agsafeset (e, "penwidth", "1", ""); 
					}
					i++;
				}
			}
			else {
				e = agedge (g, n, fin);
				if (m_aux->holgura == 0) {
					agsafeset (e, "color", "red", ""); 
					agsafeset (e, "penwidth", "3", ""); 
				}
				else {
					agsafeset (e, "color", "#828282", ""); 
					agsafeset (e, "penwidth", "1", ""); 
				}
			}

			if (m_aux->cant_corr > 0) {
				int i = 0;
				while (i < m_aux->cant_corr) {
					m = agnode (g, acortar_nombre (((materia_t *)m_aux->correlativas[i])->nombre, 10));
					e = agedge (g, m, n);
					if ((m_aux->holgura == 0) && ((materia_t *)m_aux->correlativas[i])->holgura == 0) {
						agsafeset (e, "color", "red", ""); 
						agsafeset (e, "penwidth", "3", ""); 
					}
					else {
						agsafeset (e, "color", "#828282", ""); 
						agsafeset (e, "penwidth", "1", ""); 
					}
					i++;
				}
			}
			else {
				e = agedge (g, inicio, n);
				if (m_aux->holgura == 0) {
					agsafeset (e, "color", "red", ""); 
					agsafeset (e, "penwidth", "3", ""); 
				}
				else {
					agsafeset (e, "color", "#828282", ""); 
					agsafeset (e, "penwidth", "1", ""); 
				}
			}

			m_aux = m_aux->siguiente;
		}
		anio_aux = anio_aux->siguiente;
	}

	/* Crea un subgráfico con los colores de referencia */
	ref = agsubg (g, "cluster_ref");
	agsafeset (ref, "label", "Años", ""); 
	agsafeset (ref, "fontsize", "25", ""); 
	agsafeset (ref, "shape", "circle", ""); 
	agsafeset (ref, "defaultdist", "0.1", ""); 
	agsafeset (ref, "style", "dotted", ""); 
	int i;
	for (i = 0; i < (pe->duracion_carrera / 2); i++) {
		sprintf(char_aux, "%d", (i + 1));
		n = agnode (ref, char_aux);
		agsafeset (n, "shape", "circle", ""); 
		agsafeset (n, "style", "filled", ""); 
		agsafeset (n, "fillcolor", colores[i], ""); 
		agsafeset (n, "fontcolor", coloresf[i], ""); 
		agsafeset (n, "fontsize", "22", ""); 
	}

	/* Computa el layout */
	gvLayout (gvc, g, "dot");
	/* Escribe el gráfico de acuerdo a las opciones -T y -o */
	if (archivo != NULL)
		gvRenderFilename (gvc, g, "png", archivo);
	else
		gvRenderFilename (gvc, g, "png", "plan.png");

	/* Libera los datos del layout */
	gvFreeLayout (gvc, g);
	/* Libera als estructuras del gráfico */
	agclose (g);
	/* Cierra el archivo, libera el contexto */
	gvFreeContext (gvc);
}

char *
acortar_nombre (char *cadena, const int longitud)
{
	int i = 0, c = 0;
	char *nombre = strdup (cadena);
	while (nombre[i] != '\0') {
		if ((nombre[i] == ' ') && (c > longitud)) {
			nombre[i] = '\n';
			c = 0;
		}
		else
			c++;
		i++;
	}
	return nombre;
}

void
imprimir_informe (const plan_de_estudios *pe)
{
	printf ("Carrera: %s\n", pe->nombre_carrera);
	printf ("Año del plan: %d\n", pe->anio_del_plan);
	printf ("Cuatrimestres de cursado: %d\n", pe->duracion_carrera);
	printf ("\nMATERIAS: %d\n", pe->anuales + pe->cuatrimestrales);
	printf ("\tCuatrimestrales: %d\n", pe->cuatrimestrales);
	printf ("\t\t\tAnuales: %d\n", pe->anuales);
	printf ("\nTotal de horas: %d\n", pe->total_horas);

	anio_t *anio_aux = pe->anio_carrera;
	materia_t *m_aux;

	while (anio_aux != NULL) {
		m_aux = anio_aux->materia;
		printf("Año: %d\n", anio_aux->anio);
		while (m_aux != NULL) {
			if (strcmp (m_aux->regimen, "anual") == 0)
				printf("\n\t%s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->horas);
			else
				printf ("\n\t%s - %s - %s - %s - %d\n", m_aux->id, m_aux->nombre, \
					m_aux->regimen, m_aux->cuatrimestre, m_aux->horas);

			printf ("\t\tCuat. de in. temp.: %d\n\t\tCuat. de fin temp.: %d\n", m_aux->cit, m_aux->cft);
			printf ("\t\tCuat. de in. tardío: %d\n\t\tCuat. de fin tardío: %d\n", m_aux->cita, m_aux->cfta);
			printf ("\t\tHolgura: %d\n", m_aux->holgura);

			if (m_aux->cant_corr_de > 0) {
				printf ("\t\tEs correlativa de: %d\n", m_aux->cant_corr_de);

				int i = 0;

				while (i < m_aux->cant_corr_de) {
					printf ("\t\t\t(%s) %s\n", ((materia_t *)m_aux->correlativa_de[i])->id, \
							((materia_t *)m_aux->correlativa_de[i])->nombre);
					i++;
				}
			}

			if (m_aux->cant_corr > 0) {
				printf ("\t\tTiene como correlativas a: %d\n", m_aux->cant_corr);

				int i = 0;

				while (i < m_aux->cant_corr) {
					printf ("\t\t\t(%s) %s\n", ((materia_t *)m_aux->correlativas[i])->id, \
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

	printf ("\nMaterias críticas\n");
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
	printf ("\n\tHay %d materias críticas\n", cant);
}
