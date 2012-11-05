#define SEMANAS_POR_CUATRIMESTRE 15

struct materia_t {
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
};
typedef struct materia_t materia_t;

struct anio_t {
	int anio;
	materia_t *materia;
	struct anio_t *anterior;
	struct anio_t *siguiente;
};
typedef struct anio_t anio_t;

struct plan_de_estudios {
	char *nombre_carrera;
	int anio_del_plan;
	int duracion_carrera; // en cuatrimestres
	int anuales; // cantidad de materias anuales
	int cuatrimestrales; // cantidad de materias cuatrimestrales
	int total_horas; // horas de cursado de toda la carrera
	anio_t *anio_carrera;
};
typedef struct plan_de_estudios plan_de_estudios;

struct cursado_t {
	materia_t *materia;
	materia_t **aprobar_para_rendir;
	int anio_regularizado; // indica el año en que regularizó la materia.
	char *fecha_aprobacion; // indica la fecha de aprobación de la materia.
	struct cursado_t *siguiente;
	struct cursado_t *anterior;
};
typedef struct cursado_t cursado_t;

struct alumno_t {
	int matricula;
	char *apellido;
	char *nombre;
	cursado_t *cursado;
};
typedef struct alumno_t alumno_t;
