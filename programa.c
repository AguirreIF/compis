#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include "funciones_plan.h"
#include "funciones_alumno.h"

char *alumno_xml = NULL;

int
main (int argc, char **argv)
{
	int graficar = 0;
	char *plan_xml = NULL, *grafico = NULL;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	if (argc == 1) {
		printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alu1.xml>,<alu.xml>...] [-g <salida.png>]\n", argv[0]);
		return 0;
	}
	else {
		int opcion;
		while ((opcion = getopt (argc, argv, ":p:a:g::h")) != -1)
			switch (opcion) {
				case 'p':
					plan_xml = strdup (optarg);
					break;
				case 'a':
					alumno_xml = strdup (optarg);
					break;
				case 'g':
					graficar = 1;
					if (optarg != NULL)
						grafico = strdup (optarg);
					printf ("Optarg: %s\n", optarg);
					break;
				case 'h':
					printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alu1.xml>,<alu.xml>...] [-g <salida.png>]\n", argv[0]);
					return 0;
				// opciones no reconocidas
				case '?':
					if (isprint (optopt))
						fprintf (stderr, "Opción desconocida `-%c'\n", optopt);
					else
						fprintf (stderr, "Carácter de opción desconocido `\\x%x'\n", optopt);
					return -1;
				// argumentos faltantes
				case ':':
					if (optopt == 'p')
						fprintf (stderr, "La opción `-%c' requiere el XML de un plan de estudios\n", optopt);
					else if (optopt == 'a') {
						fprintf (stderr, "La opción `-%c' requiere el XML de al menos un alumno\n", optopt);
					}
					return -1;
				default:
					fprintf (stderr, "Error desconocido `\\x%x'\n", optopt);
					return -1;
			}
	}

	if (plan_xml == NULL) {
		puts ("Debe especificar un plan de estudios con -p o --plan");
		return -1;
	}
	puts ("Procesando plan de estudios...");
	plan_de_estudios *pe = procesar_plan (plan_xml);
	if (pe != NULL) {
		imprimir_informe (pe);
		if (graficar == 1) {
			if (grafico == NULL)
				graficar_plan (pe, NULL);
			else
				graficar_plan (pe, grafico);
		}
		if (alumno_xml != NULL) {
			puts ("\nProcesando alumno...");
			// Permite varios archivos separados por coma
			char *separador = ",", *archivo;
			archivo = strtok (alumno_xml, separador);
			do {
				alumno_t *alumno = procesar_alumno (archivo, pe);
				informe_alumno (pe, alumno);
				archivo = strtok (NULL, separador);
			}
			while (archivo != NULL);
			puts ("\nAnálisis finalizado.");
		}
	}
	else {
		puts ("El procesamiento de plan de estudios a fallado");
		return -1;
	}

	return 0;
}
