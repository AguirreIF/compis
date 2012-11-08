#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "funciones_alumno.h"

void
procesar_cursado (alumno_t *alumno)
{
	cursado_t *cursado = alumno->cursado;
	while (cursado != NULL) {
		if (cursado->fecha_aprobacion == NULL) {
			int i = 0;
			int cantidad = 0;
			while (i < cursado->materia->cant_corr) {	
				char *id = ((materia_t *) cursado->materia->correlativas[i])->id;
				if (regularizada_o_aprobada (id, alumno->cursado, 0) == 0) {
					cursado->aprobar_para_rendir = realloc (cursado->aprobar_para_rendir, sizeof (materia_t *) * (cantidad + 1));
					cursado->aprobar_para_rendir[cantidad] = (materia_t *) cursado->materia->correlativas[i];
					cantidad++;
				}
				i++;
			}
			/* agrego un NULL para marcar final */
			if (cantidad > 0) {	
				cursado->aprobar_para_rendir = realloc (cursado->aprobar_para_rendir, sizeof (materia_t *) * (cantidad + 1));
				cursado->aprobar_para_rendir[cantidad] = NULL;
			}
		}
		cursado = cursado->siguiente;
	}
}

void
materias_a_rendir (const alumno_t *alumno, materia_t ***a_rendir)
{
	cursado_t *cursado = alumno->cursado;
	int cantidad = 0;
	while (cursado != NULL) {
		if ((cursado->fecha_aprobacion == NULL) && (cursado->aprobar_para_rendir == NULL)) {
			*a_rendir = realloc (*a_rendir, sizeof (materia_t *) * (cantidad + 1));
			(*a_rendir)[cantidad] = (materia_t *) cursado->materia;
			cantidad++;
		}
		cursado = cursado->siguiente;
	}
	if (cantidad > 0) {
		*a_rendir = realloc (*a_rendir, sizeof (materia_t *) * (cantidad + 1));
		(*a_rendir)[cantidad] = NULL;
	}
}

void
materias_que_no_puede_rendir (const alumno_t *alumno, cursado_t ***no_rendir)
{
	cursado_t *cursado = alumno->cursado;
	int cantidad = 0;
	while (cursado != NULL) {
		if ((cursado->fecha_aprobacion == NULL) && (cursado->aprobar_para_rendir != NULL)) {
			*no_rendir = realloc (*no_rendir, sizeof (cursado_t *) * (cantidad + 1));
			(*no_rendir)[cantidad] = (cursado_t *) cursado;
			cantidad++;
		}
		cursado = cursado->siguiente;
	}
	if (cantidad > 0) {
		*no_rendir = realloc (*no_rendir, sizeof (cursado_t *) * (cantidad + 1));
		(*no_rendir)[cantidad] = NULL;
	}
}

void
materias_a_cursar (const plan_de_estudios *pe, const alumno_t *alumno, materia_t ***a_cursar)
{
	anio_t *anio = pe->anio_carrera;
	int cantidad = 0;
	while (anio != NULL) {
		materia_t *mat_aux = anio->materia;
		int puede_cursar;
		while (mat_aux != NULL) {
			puede_cursar = 1;
			if (regularizada_o_aprobada (mat_aux->id, alumno->cursado, 1) == 0) {
				if (mat_aux->cant_corr > 0) {
					int i;
					for (i = 0; i < mat_aux->cant_corr; i++) {
						char *id = ((materia_t *) mat_aux->correlativas[i])->id;
						if (regularizada_o_aprobada (id, alumno->cursado, 1) == 0) {
							i = mat_aux->cant_corr; // para salir del bucle
							puede_cursar = 0;
						}
					}
				}
				if (puede_cursar == 1) {
					*a_cursar = realloc (*a_cursar, sizeof (materia_t *) * (cantidad + 1));
					(*a_cursar)[cantidad] = (materia_t *) mat_aux;
					cantidad++;
				}
			}
			mat_aux = mat_aux->siguiente;
		}
		anio = anio->siguiente;
	}
	if (cantidad > 0) {
		*a_cursar = realloc (*a_cursar, sizeof (materia_t *) * (cantidad + 1));
		(*a_cursar)[cantidad] = NULL;
	}
}

int
regularizada_o_aprobada (const char *id, cursado_t *cursado, const int regular)
{
	cursado_t *cur_aux = cursado;
	while (cur_aux != NULL) {
		if (strcmp (id, cur_aux->materia->id) == 0) {
			if (regular == 1)
				return 1;
			else
				return (cur_aux->fecha_aprobacion == NULL) ? 0 : 1;
		}
		cur_aux = cur_aux->siguiente;
	}
	return 0;
}
				
void
informe_alumno (const plan_de_estudios *pe, const alumno_t *alumno)
{
	materia_t **materias = NULL;
	materias_a_cursar (pe, alumno, &materias);
	if (materias != NULL) {
		puts ("\nPuede cursar:");
		while (*materias != NULL) {
			printf ("\t%s\n", (*materias)->nombre);
			materias++;
		}
	}
	else
		printf ("No puede seguir cursando");

	materias = NULL;
	materias_a_rendir (alumno, &materias);
	if (materias != NULL) {
		puts ("\nPuede rendir:");
		while (*materias != NULL) {
			printf ("\t%s\n", (*materias)->nombre);
			materias++;
		}
		free (*materias);
	}
	else
		puts ("No tiene materias para rendir");
	materias = NULL;

	cursado_t **no_rendir = NULL;
	materias_que_no_puede_rendir (alumno, &no_rendir);
	if (no_rendir != NULL) {
		while (*no_rendir != NULL) {
			printf ("\nPara rendir %s tiene que aprobar:\n", (*no_rendir)->materia->nombre);
			materia_t **materias = (*no_rendir)->aprobar_para_rendir;
			while (*materias != NULL) {
				printf ("\t%s\n", (*materias)->nombre);
				materias++;
			}
			no_rendir++;
		}
	}
	else
		printf ("No tiene materias para potencialmente rendir");
	no_rendir = NULL;
}
