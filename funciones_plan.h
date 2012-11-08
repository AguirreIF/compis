#pragma once
#ifndef FUNCIONES_PLAN_H
#define FUNCIONES_PLAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.h"

// función principal de proceso en plan_de_estudios.y
plan_de_estudios *procesar_plan (const char *plan_xml);

// Prepara la estructura del plan de estudios.
void inicializar (plan_de_estudios **pe);

// Crea estructuras de año dentro del plan de estudios
anio_t *crear_anio_carrera (const int anio, plan_de_estudios *pe);

// Crea estructuras de materias dentro de año
materia_t *crear_materia (anio_t *anio_aux, const char *id_materia, const char *nombre_materia);

// Guarda la el ID de la correlativa en el array de materias.
void cargar_correlativa (materia_t *materia, const char *id);

// Ordena los años de menor a mayor y las materias
// primero las del 1er cuatrimestre, después las del 2do
// y por último las anuales.
void ordenar_anios (plan_de_estudios *pe);

// Reemplaza los apuntadores a char de la estructura
// de correlativas por apuntadores a las materias.
void apuntar_correlativas (plan_de_estudios *pe);

// Recibe el ID de una materia y devuelve un apuntador a ella.
materia_t *buscar_materia (const plan_de_estudios *pe, const char *id);

// Calcula los cuatrimestres de inicio y fin temprano
void calcular_cit_cft (materia_t *m_aux);

// Calcula los cuatrimestres de inicio y fin tardíos
void calcular_cita_cfta (materia_t *m_aux, const int cuatrimestre_fin);

// Calcula los cuatrimestres de inicio temprano y tardío de
// cada materia.
void calcular_tiempos (plan_de_estudios *pe);

// Imprime toda la info del plan
void imprimir_informe (const plan_de_estudios *pe);

// Grafica con graphviz el CPM y resalta la ruta crítica
void graficar_plan (const plan_de_estudios *pe, const char *archivo);

// Función auxiliar que inserta saltos de línea en oraciones largas
char *acortar_nombre (char *cadena, const int longitud);
 
#endif /* FUNCIONES_PLAN_H */
