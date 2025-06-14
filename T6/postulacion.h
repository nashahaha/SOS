#define N 100

extern int postulacionTrabajos[N];

typedef struct estudiante {
    int id;
    int trabajo_id; // -1 si no tiene trabajo
} Estudiante;

void iniciarPostulaciones();
void destruirPostulaciones();
void postularTrabajo(Estudiante *est, int *preferencias, double *rank);
int cerrarPostulacion(int i);


