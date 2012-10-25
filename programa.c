#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "estructuras.h"

int main (int argc, char **argv) {

	int opcion;
	char *plan_xml = NULL, *alumno_xml = NULL;
	plan_de_estudios *pe;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	if (argc == 1) {
		printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alumno1.xml> <alumno2.xml>...]\n", argv[0]);
		return 0;
	}
	else
		while ((opcion = getopt(argc, argv, ":p:a:h")) != -1)
			switch (opcion) {
				case 'p':
					plan_xml = strdup (optarg);
					break;
				case 'a':
					alumno_xml = strdup (optarg);
					break;
				case 'h':
					printf ("Uso: %s -p <plan_de_estudios.xml> [-a <alumno1.xml> <alumno2.xml>...]\n", argv[0]);
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
		return -1;
	}

	if (plan_xml == NULL) {
		printf ("Debe especificar un plan de estudios con -p o --plan\n");
		return -1;
	}
	printf ("Procesando plan de estudios...\n");
	pe = procesar_plan (plan_xml);
	if (pe != NULL) {
		if (alumno_xml != NULL) {
			printf ("\nProcesando alumno...\n");
			// Permite varios archivos separados por coma
			char *separador = ",", *archivo;
			archivo = strtok(alumno_xml, separador);
			do {
				procesar_alumno (archivo, pe);
				archivo = strtok(NULL, separador);
			}
			while (archivo != NULL);
			printf ("\nAnálisis finalizado.\n");
		}
	}
	else
		printf("El procesamiento de plan de estudios a fallado\n");

	return 0;
}
