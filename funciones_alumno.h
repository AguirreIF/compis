#pragma once
#ifndef FUNCIONES_ALUMNO_H
#define FUNCIONES_ALUMNO_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "estructuras.h"

// unción principal de proceso en alumno.y
alumno_t *procesar_alumno (char *alumno_xml, plan_de_estudios *pe);

// por cada materia no aprobada carga en aprobar_para_rendir
// las materias que debería aprobar para rendir la materia
void procesar_cursado (alumno_t *alumno);

// guarda en el array a_rendir las materias que puede rendir
void materias_a_rendir (const alumno_t *alumno, materia_t ***a_rendir);

// guarda en el array a_rendir las materias que puede rendir
void materias_que_no_puede_rendir (const alumno_t *alumno, cursado_t ***no_rendir);

// guarda en el array a_rendir las materias que puede rendir
void materias_a_cursar (const plan_de_estudios *pe, const alumno_t *alumno, materia_t ***a_cursar);

// devuelve 1 si la materia está regularizada, 0 caso contrario
// la bandera regular indica si buscar solo aprobadas o también regularizadas
int regularizada_o_aprobada (const char *id, cursado_t *cursado, const int regular);

void informe_alumno (const plan_de_estudios *pe, const alumno_t *alumno);
 
#endif /* FUNCIONES_ALUMNO_H */
