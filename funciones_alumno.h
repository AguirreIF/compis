#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "estructuras.h"

// TODO
// NECESITA ESTAR LA DEFINICIÓN
// definición original está en funciones_plan.h
materia_t *buscar_materia(plan_de_estudios *, char *);

// Esta función crea un puntero hacia una materia y la devuelve.
materia_t *crear_materia ();

// Esta función muestra los datos del alumno
void mostrar_alumno (alumno);

// Esta función muestra las materias que el alumno aprobó o regularizó
void mostrar_materias_alumno (alumno);

// Esta función muestra las materias que puede rendir el alumno.
void mostrar_materias_a_rendir (plan_de_estudios *, alumno);

// Esta función muestra las materias que puede cursar el alumno.
void mostrar_materias_a_cursar (plan_de_estudios *, alumno);

// Esta función verifica si el alumno tiene aprobada una materia, recibe un id y un puntero a materia
// a partir de la cual realiza la búsqueda. Devuelve 0 si no esta aprobada, o 1 si esta aprobada.
int materia_aprobada (char *, materia_t *);

// Esta función recibe el id de una materia y un puntero a la primer materia de la estructura
// de alumno. Recorre las materias de la estructura, devuelve un cero si la materia con el id
// buscado no esta, y un 1 si la materia buscada esta.
int buscar_materia_reg_o_aprob (char *, materia_t *); 

materia_t *crear_materia_alu () {
	materia_t *mat;
	mat = (materia_t *) malloc (sizeof (materia_t));
	mat->siguiente = NULL;
	return mat;	
}

void mostrar_alumno (alumno alu) {
	printf ("\nLos dados del alumno son:\n");
	printf ("Nombre: %s\n", alu.nombre);
	printf ("Apellido: %s\n", alu.apellido);
	printf ("Matrícula: %d\n", alu.matricula);
}

void mostrar_materias_alumno (alumno alu) {
	materia_t *mat;
	printf ("\nLas materias regularizadas y/o aprobadas son:\n");
	mat = alu.materia;
	while (mat) {
		printf ("id: %s, año reg: %d. fech ap: %s\n", mat->id, mat->anio_reg, mat->fecha_ap);
		mat = mat->siguiente;
	}
}

void mostrar_materias_a_rendir (plan_de_estudios *pe, alumno alu) {
	/*
	 * Busca todas las materias que no tienen fecha de aprobación dentro de la estructura alumno.
	 * Por cada materia no aprobada, realiza una búsqueda por el id de la misma, dentro de la 
	 * estructura de plan de estudios, donde se tendrá la información de las materias correlativas.
	 * Luego se verifica que el alumno tenga aprobada las correlativas, de ser así, puede rendir la 
	 * materia, caso contrario, se informa cuales son las materias que debe tener aprobada para rendir
	 * la materia en cuestión.
	 */
	
	printf ("\nBuscando materias que puede rendir el alumno...\n");
	struct materia_t *mat_alu, *mat_plan, *mat_sin_rendir = NULL, *ind_mat_sin_rendir, *mat_aux;
	int comp = 1;
	int rendir = 0; // Se usa como bandera para indicar si se pude o no rendir la mat.
	int ind;
	mat_alu = alu.materia;
	while (mat_alu) {
		comp = strcmp (mat_alu->fecha_ap , "-/-/-");
		if (comp == 0) {
			// Significa que la materia se tiene que rendir
			mat_plan = buscar_materia(pe, mat_alu->id);
			if (mat_plan) {
				// Significa que se encontró la materia en el plan de estudios.
				// Ahora hay que ver si ese materia tiene correlativas y si estas
				// están aprobadas.
				if (mat_plan->cant_corr > 0) {
					// Significa que la materia tiene correlativas.
					// Por lo tanto hay que verificar que estas correlativas estén aprobadas
					// para que se puede rendir la materia.
					rendir = 0;
					for (ind = 0; ind < mat_plan->cant_corr; ind++) {
						mat_aux = (materia_t *) mat_plan->correlativas[ind];
						// Ahora verifico si esa materia esta aprobada.
						if (materia_aprobada (mat_aux->id, alu.materia) == 1) {
							// Significa que la materia esta aprobada y pude rendir.
							rendir = 1;
						}
						else {
							rendir = 0;
							if (mat_sin_rendir != NULL) {
								mat_sin_rendir->siguiente = (materia_t *) malloc (sizeof (materia_t));
								mat_sin_rendir = mat_sin_rendir->siguiente;
								mat_sin_rendir->nombre = mat_aux->nombre;
							}
							else {
								mat_sin_rendir = (materia_t *) malloc (sizeof (materia_t));
								ind_mat_sin_rendir = mat_sin_rendir;
								mat_sin_rendir->nombre = mat_aux->nombre;
							}
							mat_sin_rendir->siguiente = NULL;
						}
					}
				}
				else
					rendir = 1;
				if (rendir)
					printf ("Puede rendir la materia: %s, id: %s\n", mat_plan->nombre, mat_plan->id);
				else {
					printf ("Para rendir la materia: %s, id: %s, debe tener aprobadas las siguientes materias:\n",
						mat_plan->nombre, mat_plan->id);
					mat_sin_rendir = ind_mat_sin_rendir;
					while (mat_sin_rendir) {
						printf ("+-> %s\n", mat_sin_rendir->nombre);
						mat_sin_rendir = mat_sin_rendir->siguiente;
					}
					if (mat_sin_rendir)
						free (mat_sin_rendir);
				}
			}
			else
				printf ("No se encontró la materia con el id: %s, en el plan de estudios.\n", mat_alu->id);
		}
		mat_alu = mat_alu->siguiente;
	}
}

void mostrar_materias_a_cursar (plan_de_estudios *pe, alumno alu) {
	printf ("\nBuscando materias que puede cursar el alumno...\n");
	anio_t *anio;
	materia_t *mat_aux, *mat_cursar, *ind_mat_cursar, *mat_corr;
	int puede_cursar = 0;
	int pudo_cursar_alguna = 0;
	int ind;
	anio = pe->anio_carrera;
	while (anio != NULL) {
		mat_aux = anio->materia;
		while (mat_aux != NULL) {
			// Verifico si la materia esta aprobada o regularizada.
			if (buscar_materia_reg_o_aprob (mat_aux->id, alu.materia) == 0) {
				// Significa que la materia no se curso.
				// Verifico si tiene correlativas.
				if (mat_aux->cant_corr > 0) {
					// Significa que la materia tiene correlativas.
					puede_cursar = 0;
					// Verificar que todas las correlativas, estén aprobadas o regularizadas.
					for (ind = 0; ind < mat_aux->cant_corr; ind++) {
						mat_corr = (materia_t *) mat_aux->correlativas[ind];
						if (buscar_materia_reg_o_aprob (mat_corr->id, alu.materia) == 0) {
							// Significa que la materia correlativa no se curso.
							// Por lo tanto, mat_aux no se puede cursar.
							puede_cursar = 0;
							break;
						}
						else {
							puede_cursar = 1;
							pudo_cursar_alguna = 1;
						}
					}
				}
				else {
					puede_cursar = 1;
					pudo_cursar_alguna = 1;
				}
				if (puede_cursar)
					printf ("Puede cursar la materia: %s, id: %s\n", mat_aux->nombre, mat_aux->id);	
			}
			mat_aux = mat_aux->siguiente;
		}
		anio = anio->siguiente;
	}
	if (pudo_cursar_alguna)
		printf ("Estas son todas las materias que puede cursar el alumno.\n");
	else
		printf ("El alumno no pude cursar ninguna materia\n");
}

int materia_aprobada (char *id, materia_t *materia) {
	int resultado = 0;
	materia_t *mat = materia;
	while (mat) {
		if (strcmp(id,mat->id) == 0) {
			// Significa que encontró la materia
			// Verifico que esté aprobada.
			if (strcmp(mat->fecha_ap, "-/-/-") != 0) {
				// Significa que la tiene fecha de aprobación, por lo tanto,
				// esta aprobada.
				resultado = 1;
			}
			break;
		}
		mat = mat->siguiente;
	}
	return resultado;
}

int buscar_materia_reg_o_aprob (char *id, materia_t *materia) {
	int resultado = 0;
	materia_t *mat = materia;
	while (mat) {
		if (strcmp(id, mat->id) == 0) {
			// Significa que se encontró la materia.
			resultado = 1;
			break;
		}
		mat = mat->siguiente;
	}
	return resultado;
}
