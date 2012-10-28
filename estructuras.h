#define SEMANAS_POR_CUATRIMESTRE 15

typedef struct materia_t {
	char *nombre;
	char *id;
	char *regimen;
	char *cuatrimestre;
	int horas;
	int cant_corr; // la cantidad de correlativas que tiene
	int cant_corr_de; // de cuantas materias es correlativa
	int cit; // cuatrimestre de inicio temprano
	int cft; // cuatrimestre de fin temprano
	int cita; // cuatrimestre de inicio tardío
	int cfta; // cuatrimestre de fin tardío
	int holgura;
	int duracion; // 0 = cuatrimestral, 1 = anual
	// es void porque mientras lee el archivo arma un
	// vector de chars, y después los punteros a char
	// los reemplaza por punteros a la materia
	void **correlativas;
	struct materia_t **correlativa_de;
	struct materia_t *anterior;
	struct materia_t *siguiente;
	// Las siguientes definiciones son para analizar los datos de un alumno.
	int anio_reg; // indica el año en que regularizó la materia.
	char *fecha_ap; // indica la fecha de aprobación de la materia.
} materia_t;

typedef struct anio_t {
	int anio;
	materia_t *materia;
	struct anio_t *anterior;
	struct anio_t *siguiente;
} anio_t;

typedef struct plan_de_estudios {
	char *nombre_carrera;
	int anio_del_plan;
	int duracion_carrera; // en cuatrimestres
	int anuales; // cantidad de materias anuales
	int cuatrimestrales; // cantidad de materias cuatrimestrales
	int total_horas; // horas de cursado de toda la carrera
	anio_t *anio_carrera;
} plan_de_estudios;

typedef struct alumno {
	int matricula;
	char *apellido;
	char *nombre;
	materia_t *materia;
} alumno;
