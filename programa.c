#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "funciones_alumno.h"
#include "funciones_plan.h"

// la declaro global para llevar el registro del archivo de alumno que está
// analizando a cada momento, para la función buscar_materia
char *alumno_xml = NULL;

int
main (int argc, char **argv)
{
	int graficar = 0;
	char *plan_xml = NULL, *grafico = NULL;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	int opcion;

	if (argc == 1) {
		printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alu1.xml>,<alu.xml>...] [-g <salida.png>]\n", argv[0]);
		exit (EXIT_SUCCESS);
	}
	else {
		while ((opcion = getopt (argc, argv, ":p:a:g:h")) != -1)
			switch (opcion) {
				case 'p':
					plan_xml = strdup (optarg);
					break;
				case 'a':
					alumno_xml = strdup (optarg);
					break;
				case 'g':
					graficar = 1;
					grafico = strdup (optarg);
					break;
				case 'h':
					printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alu1.xml>,<alu.xml>...] [-g <salida.png>]\n", argv[0]);
					exit (EXIT_SUCCESS);
				// opciones no reconocidas
				case '?':
					if (isprint (optopt))
						fprintf (stderr, "Opción desconocida `-%c'\n", optopt);
					else
						fprintf (stderr, "Carácter de opción desconocido `\\x%x'\n", optopt);
					exit (EXIT_FAILURE);
				// argumentos faltantes
				case ':':
					if (optopt == 'p')
						fprintf (stderr, "La opción `-%c' requiere el XML de un plan de estudios\n", optopt);
					else if (optopt == 'a')
						fprintf (stderr, "La opción `-%c' requiere el XML de al menos un alumno\n", optopt);
					// se puede no pasar argumento a -g
					// crea un archivo por defecto plan.png
					else if (optopt == 'g') {
						graficar = 1;
						break;
					}
					exit (EXIT_FAILURE);
				default:
					fprintf (stderr, "Error desconocido `\\x%x'\n", optopt);
					exit (EXIT_FAILURE);
			}
	}

	// si quedaron argumentos no reconocidos
	if (argc > optind) {
		if ((argc - optind) == 1)
			printf ("Argumento desconocido: %s\n", argv[optind]);
		else {
			printf ("Argumentos desconocidos: ");
			for (opcion = optind; opcion < argc; opcion++)
				printf ("%s ", argv[opcion]);
			puts ("");
		}
		exit (EXIT_FAILURE);
	}

	if (plan_xml == NULL) {
		puts ("Debe especificar un plan de estudios con -p o --plan");
		exit (EXIT_FAILURE);
	}
	puts ("Procesando plan de estudios...");
	plan_de_estudios *pe = procesar_plan (plan_xml);
	if (pe != NULL) {
		imprimir_informe (pe);
		if (graficar == 1) {
			puts ("\nPreparando gráfico...");
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
		puts ("El procesamiento del plan de estudios a fallado");
		exit (EXIT_FAILURE);
	}

	exit (EXIT_SUCCESS);
}
