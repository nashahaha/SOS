#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>

#include "spinlocks.h"
#include "postulacion.h"

#pragma GCC diagnostic ignored "-Wunused-function"

static int cerrarThreadFinalizado[N];
static int postularTrabajoThreadFinalizado[N];

static void msleep(int delay) {
  usleep(delay*1000);
}

// ==================================================
// Funcion para reportar un error y detener la ejecucion

void fatalError( char *procname, char *format, ... ) {
  va_list ap;

  fprintf(stderr,"Error Fatal en la rutina %s y la tarea \n", procname);
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  exit(1); /* shutdown */
}

// ==================================================
// Funciones para reportar nombre de los threads

__thread char *threadName= NULL;

static void setThreadName(char *name) {
  threadName= name;
}

static char *getThreadName(void) {
  return threadName;
}

// print_msg:
//   1   => con mensajes y delay
//   0   => sin mensajes, con delay
//   -1  => sin mensajes, sin delay

#define NUM_ESTUDIANTES N
#define NUM_TRABAJOS N

static void resetVariablesGlobales() {
    for (int i = 0; i < N; i++) {
        cerrarThreadFinalizado[i] = 0;
        postularTrabajoThreadFinalizado[i] = 0;
    }
}


static void *estudiante_thread_test1(void *arg) {
    Estudiante *estudiante = (Estudiante*) arg;
    int trabajos_de_interes[NUM_TRABAJOS];
    double ranking[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_de_interes[i] = 1; // Inicialmente todos los trabajos son de interés
        ranking[i] = 1.0;
    }
    postularTrabajo(estudiante, trabajos_de_interes, ranking);
    postularTrabajoThreadFinalizado[estudiante->id] = 1; // Marcar que el estudiante ha postulado
    free(estudiante); // Liberar la memoria del estudiante
    //printf("Estudiante %d ha conseguido el trabajo %d.\n", estudiante->id, estudiante->trabajo_id);
    return NULL;
}

static void *estudiante_thread_test2(void *arg) {
    Estudiante *estudiante = (Estudiante*) arg;
    int trabajos_de_interes[NUM_TRABAJOS];
    double ranking[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {        
        if (i==estudiante->id) // Cada estudiante postula a su propio trabajo
        {
            trabajos_de_interes[i] = 1; 
            ranking[i] = (float) rand() / RAND_MAX; // Simulamos un rank aleatorio
        }
        else{
            trabajos_de_interes[i] = 0; // Los demás trabajos no son de interés
            ranking[i] = 0.0; // No tienen ranking
        }
    }    

    postularTrabajo(estudiante, trabajos_de_interes, ranking);
    if (estudiante->trabajo_id != estudiante->id) {
        fatalError("estudiante_thread_test2", "Error: Estudiante %d no consiguió el trabajo %d como se esperaba.\n", estudiante->id, estudiante->trabajo_id);
    }
    postularTrabajoThreadFinalizado[estudiante->id] = 1; // Marcar que el estudiante ha postulado
    free(estudiante); // Liberar la memoria del estudiante
    //printf("Estudiante %d ha conseguido el trabajo %d.\n", estudiante->id, estudiante->trabajo_id);
    return NULL;
}

static void *estudiante_thread_test3(void *arg) {
    Estudiante *estudiante = (Estudiante*) arg;
    int trabajos_de_interes[NUM_TRABAJOS];
    double ranking[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {        
        if (i==estudiante->id) // Cada estudiante postula a su propio trabajo
        {
            trabajos_de_interes[i] = 1; 
            ranking[i] = (float) rand() / RAND_MAX; // Simulamos un rank aleatorio
            //printf("Estudiante %d postula al trabajo %d.\n", estudiante->id, i);
        }
        else{
            trabajos_de_interes[i] = 0; // Los demás trabajos no son de interés
            ranking[i] = 0.0; // No tienen ranking
        }
    }    

    postularTrabajo(estudiante, trabajos_de_interes, ranking);
    
    postularTrabajoThreadFinalizado[estudiante->id] = 1; // Marcar que el estudiante ha postulado
    free(estudiante); // Liberar la memoria del estudiante
    //printf("Estudiante %d ha conseguido el trabajo %d.\n", estudiante->id, estudiante->trabajo_id);
    return NULL;
}
static void *estudiante_thread_test4(void *arg) {
    return estudiante_thread_test3(arg);
}

static void *estudiante_thread_test5(void *arg) {
    Estudiante *estudiante = (Estudiante*) arg;
    int trabajos_de_interes[NUM_TRABAJOS]; // Inicialmente todos los trabajos no son de interés
    // Cada estudiante postula a los 3 trabajos contiguos de forma cíclica
    double ranking[NUM_TRABAJOS];
    for (int i = 0; i < N; i++) {
        if (i < 3){
            trabajos_de_interes[(estudiante->id + i + 5) % NUM_TRABAJOS] = 1;
            ranking[(estudiante->id + i + 5) % NUM_TRABAJOS] = i;
        }
        else {
            trabajos_de_interes[(estudiante->id + i + 5) % NUM_TRABAJOS] = 0; // Los demás trabajos no son de interés
            ranking[(estudiante->id + i + 5) % NUM_TRABAJOS] = 0.0; // No tienen ranking
        }
    }

    postularTrabajo(estudiante, trabajos_de_interes, ranking);
    if (estudiante->trabajo_id != (estudiante->id + 5) % NUM_TRABAJOS) {
        fatalError("estudiante_thread_test5", "Error: Estudiante %d no consiguió el trabajo %d como se esperaba. Obtiene %d.\n", 
                   estudiante->id, (estudiante->id + 5) % NUM_TRABAJOS, estudiante->trabajo_id);
    }

    postularTrabajoThreadFinalizado[estudiante->id] = 1; // Marcar que el estudiante ha postulado
    //printf("Estudiante %d ha conseguido el trabajo %d.\n", estudiante->id, estudiante->trabajo_id);
    free(estudiante); // Liberar la memoria del estudiante
    return NULL;
}

static void *trabajo_thread_test1(void *arg) {
    int trabajo_id = *(int *)arg;
    cerrarPostulacion(trabajo_id);
    cerrarThreadFinalizado[trabajo_id] = 1; // Marcar que el trabajo ha sido cerrado
    return NULL;
}


static void *trabajo_thread_test2(void *arg) {
    return trabajo_thread_test1(arg);
}
static void *trabajo_thread_test3(void *arg) {
    return trabajo_thread_test1(arg);
}

static void *trabajo_thread_test4(void *arg) {
    return trabajo_thread_test1(arg);
}

static void *trabajo_thread_test5(void *arg) {
    return trabajo_thread_test1(arg);
}

// se lanzan n-1 threads estudiantes. Luego una vez cerrado los n-1 estudiantes
// debería quedar solamente un trabajo pendiente.
static int test1(){
    resetVariablesGlobales();
    pthread_t threads_estudiantes[NUM_ESTUDIANTES];
    pthread_t threads_trabajos[NUM_TRABAJOS];

    printf("Iniciando el test 1... con %d estudiantes y %d trabajos.\n", NUM_ESTUDIANTES, NUM_TRABAJOS);
    printf("\tIniciando postulaciones...\n");
    iniciarPostulaciones();

    printf("\tCreando hilos para las estudiantes...\n");
    for (int i = 0; i < NUM_ESTUDIANTES - 1; i++) {
        Estudiante *pestudiante = malloc(sizeof(Estudiante));
        pestudiante->id = i;
        pestudiante->trabajo_id = -1;
        //printf("Creando hilo de estudiante %d...\n", i);
        if (pthread_create(&threads_estudiantes[i], NULL, estudiante_thread_test1, pestudiante) != 0) {
            perror("Error al crear el hilo de estudiante");
            exit(EXIT_FAILURE);
        }
        
    }
    printf("\tEsperando 5 segundos...\n");
    sleep(5);
    printf("\tDespertando...\n");
    printf("\tCreando hilos para los trabajos...\n");
    int trabajos_index[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_index[i] = i;
        pthread_create(&threads_trabajos[i], NULL, trabajo_thread_test1, (void *)&trabajos_index[i]);
    }

    for (int i = 0; i < NUM_ESTUDIANTES-1; i++) {
        pthread_join(threads_estudiantes[i], NULL);
    }
    
    int hay_trabajos_pendientes = 0;
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            hay_trabajos_pendientes += 1;
        }
    }
    if (hay_trabajos_pendientes != 1) 
        fatalError("test1", "Error: Debía quedar solamente un trabajo no asignado, pero hay %d trabajos no asignados.\n", hay_trabajos_pendientes);

    Estudiante *ultimo = malloc(sizeof(Estudiante));
    ultimo->id = NUM_ESTUDIANTES - 1;
    ultimo->trabajo_id = -1; // Inicialmente no tiene trabajo
    pthread_create(&threads_estudiantes[NUM_ESTUDIANTES - 1], NULL, &estudiante_thread_test1, (void *)ultimo);
    pthread_join(threads_estudiantes[NUM_ESTUDIANTES - 1], NULL);
    printf("\tTodos los hilos de estudiantes han terminado.\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            fatalError("test1", "Error: Trabajo %d no debería haber quedado pendiente después de que los estudiantes postulen.\n", i);
        }
        pthread_join(threads_trabajos[i], NULL);
    }

    printf("\tDestruyendo postulaciones...\n");
    destruirPostulaciones();
    
    printf("\tTodos los hilos han terminado.\n");   
    
    return 0;
}

// se verá la correctitud, cada estudiante postulará solamente a un trabajo 
// y se espera que todos los estudiantes consigan el trabajo que postularon
static int test2(){
    resetVariablesGlobales();
    pthread_t threads_estudiantes[NUM_ESTUDIANTES];
    pthread_t threads_trabajos[NUM_TRABAJOS];

    printf("Iniciando el test 2... con %d estudiantes y %d trabajos.\n", NUM_ESTUDIANTES, NUM_TRABAJOS);
    printf("\tIniciando postulaciones...\n");
    iniciarPostulaciones();

    printf("\tCreando hilos para las estudiantes...\n");
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        Estudiante *pestudiante = malloc(sizeof(Estudiante));
        pestudiante->id = i;
        pestudiante->trabajo_id = -1; // Inicialmente no tiene trabajo
        if (pthread_create(&threads_estudiantes[i], NULL, estudiante_thread_test2, pestudiante) != 0) {
            perror("Error al crear el hilo de estudiante");
            exit(EXIT_FAILURE);
        }
    }
    printf("\tEsperando 5 segundos...\n");
    sleep(5); 
    printf("\tDespertando...\n");
    printf("\tCreando hilos para los trabajos...\n");
    int trabajos_index[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_index[i] = i;
        pthread_create(&threads_trabajos[i], NULL, trabajo_thread_test2, (void *)&trabajos_index[i]);
    }
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        pthread_join(threads_estudiantes[i], NULL);
    }
    printf("\tTodos los hilos de estudiantes han terminado.\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            fatalError("test2", "Error: No debían quedar trabajos pendientes, pero hay trabajos pendientes.\n");
            break;
        } 
        pthread_join(threads_trabajos[i], NULL);
    }
        
    printf("\tDestruyendo postulaciones...\n");
    destruirPostulaciones();
    printf("\tTodos los hilos han terminado.\n");   
    return 0;
}

// se llamará a cerrar los trabajos primero, pero estos no se deben haber cerrado
// porque los estudiantes no han postulado a ningún trabajo.
// Luego se postularán los estudiantes a los trabajos y ahí los trabajos se cerrarán
static int test3(){
    resetVariablesGlobales();
    pthread_t threads_estudiantes[NUM_ESTUDIANTES];
    pthread_t threads_trabajos[NUM_TRABAJOS];
    printf("Iniciando el test 3... con %d estudiantes y %d trabajos.\n", NUM_ESTUDIANTES, NUM_TRABAJOS);
    printf("\tIniciando postulaciones...\n");
    iniciarPostulaciones();
    printf("\tCreando hilos para los trabajos...\n");
    int trabajos_index[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_index[i] = i;
        pthread_create(&threads_trabajos[i], NULL, trabajo_thread_test3, (void *)&trabajos_index[i]);
    }
    printf("\tEsperando 5 segundos...\n");
    sleep(5);
    printf("\tDespertando...\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (cerrarThreadFinalizado[i] == 1) {
            fatalError("test3", "Error: Trabajo %d no debería haber sido cerrado antes de que los estudiantes postulen.\n", i);
        }
    }
    printf("\tCreando hilos para las estudiantes...\n");
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        Estudiante *pestudiante = malloc(sizeof(Estudiante));
        pestudiante->id = i;
        pestudiante->trabajo_id = -1;
        //printf("Creando hilo de estudiante %d...\n", i);
        if (pthread_create(&threads_estudiantes[i], NULL, estudiante_thread_test3, pestudiante) != 0) {
            perror("Error al crear el hilo de estudiante");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        //printf("Join estudiente %d\n", i);
        pthread_join(threads_estudiantes[i], NULL);
    }
    printf("\tTodos los hilos de estudiantes han terminado.\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            fatalError("test3", "Error: Trabajo %d no debería haber quedado pendiente después de que los estudiantes postulen.\n", i);
        }
        pthread_join(threads_trabajos[i], NULL);
    }
    printf("\tTodos los hilos de trabajos han terminado.\n");
    printf("\tDestruyendo postulaciones...\n");
    destruirPostulaciones();
    printf("\tTodos los hilos de trabajos han terminado.\n");
    return 0;
}

// se verá que ningún estudiante que haya postulado a un trabajo
// haya quedado en un trabajo cuando aún no se han cerrado los trabajos
static int test4(){
    resetVariablesGlobales();
    int trabajos_index[NUM_TRABAJOS];
    pthread_t threads_estudiantes[NUM_ESTUDIANTES];
    pthread_t threads_trabajos[NUM_TRABAJOS];
    printf("Iniciando el test 4... con %d estudiantes y %d trabajos.\n", NUM_ESTUDIANTES, NUM_TRABAJOS);
    printf("\tIniciando postulaciones...\n");
    iniciarPostulaciones();
    printf("\tCreando hilos para las estudiantes...\n");
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        Estudiante *pestudiante = malloc(sizeof(Estudiante));
        pestudiante->id = i;
        pestudiante->trabajo_id = -1; // Inicialmente no tiene trabajo
        if (pthread_create(&threads_estudiantes[i], NULL, estudiante_thread_test2, pestudiante) != 0) {
            perror("Error al crear el hilo de estudiante");
            exit(EXIT_FAILURE);
        }
    }
    printf("\tEsperando 5 segundos...\n");
    sleep(5);
    printf("\tDespertando...\n");
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        if (postularTrabajoThreadFinalizado[i] == 1) {
            fatalError("test4", "Error: Estudiante %d no debería haber conseguido un trabajo antes de que se cierren los trabajos.\n", i);
        }
    }
    printf("\tCreando hilos para los trabajos...\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_index[i] = i;
        pthread_create(&threads_trabajos[i], NULL, trabajo_thread_test4, (void *)&trabajos_index[i]);
    }
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        pthread_join(threads_estudiantes[i], NULL);
    }
    printf("\tTodos los hilos de estudiantes han terminado.\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            fatalError("test4", "Error: Trabajo %d no debería haber quedado pendiente después de que los estudiantes postulen.\n", i);
        }
        pthread_join(threads_trabajos[i], NULL);
    }
    printf("\tDestruyendo postulaciones...\n");
    destruirPostulaciones();
    printf("\tTodos los hilos de trabajos han terminado.\n");
    return 0;
}

// se postularan la mitad de los estudiantes con la siguiente lógica:
// El estudiante postulan a los 3 trabajos contiguos de forma cíclica aumentando su ranking (empeora la prioridad)
static int test5(){
    resetVariablesGlobales();
    pthread_t threads_estudiantes[NUM_ESTUDIANTES];
    pthread_t threads_trabajos[NUM_TRABAJOS];

    printf("Iniciando el test 5... con %d estudiantes y %d trabajos.\n", NUM_ESTUDIANTES, NUM_TRABAJOS);
    printf("\tIniciando postulaciones...\n");
    iniciarPostulaciones();
    printf("\tCreando hilos para las estudiantes...\n");
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        Estudiante *pestudiante = malloc(sizeof(Estudiante));
        pestudiante->id = i;
        pestudiante->trabajo_id = -1; // Inicialmente no tiene trabajo
        if (pthread_create(&threads_estudiantes[i], NULL, estudiante_thread_test5, pestudiante) != 0) {
            perror("Error al crear el hilo de estudiante");
            exit(EXIT_FAILURE);
        }
    }
    printf("\tEsperando 5 segundos\n");
    sleep(5); 
    printf("\tDespertando...\n");
    printf("\tCreando hilos para los trabajos...\n");
    int trabajos_index[NUM_TRABAJOS];
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        trabajos_index[i] = i;
        pthread_create(&threads_trabajos[i], NULL, trabajo_thread_test5, (void *)&trabajos_index[i]);
    }
    
    for (int i = 0; i < NUM_ESTUDIANTES; i++) {
        pthread_join(threads_estudiantes[i], NULL);
    }
    printf("\tTodos los hilos de estudiantes han terminado.\n");
    for (int i = 0; i < NUM_TRABAJOS; i++) {
        if (postulacionTrabajos[i] == -1) {
            fatalError("test5", "Error: No debían quedar trabajos pendientes, pero hay trabajos pendientes.\n");
            break;
        }
        pthread_join(threads_trabajos[i], NULL);
    }
    printf("\tDestruyendo postulaciones...\n");
    destruirPostulaciones();
    printf("\tTodos los hilos de trabajos han terminado.\n");
    return 0;

}

int main(int argc, char *argv[]) {
    int bwtop= argc <=1 ? 2 : 1;
    for (int bw= 0; bw<bwtop; bw++) { 
        if(bw==0)
            printf("Prueba con spinlocks implementados con mutex sin busywaiting\n");
        else { 
            printf("\n");
            printf("===============================================\n");
            printf("Prueba con spinlocks verdaderos con busywaiting\n");
            printf("===============================================\n\n");
            setBusyWaiting(bw);
        }
        if (!test1())
            printf("Test 1 completado exitosamente.\n");
        else
            printf("Test 1 fallido.\n");
        if (!test2())
            printf("Test 2 completado exitosamente.\n");
        else
            printf("Test 2 fallido.\n");
        
        if (!test3())
            printf("Test 3 completado exitosamente! :)\n");
        else
            printf("Test 3 fallido.\n");
        if (!test4())
            printf("Test 4 completado exitosamente! :)\n");
        else
            printf("Test 4 fallido.\n");
        
        if (!test5())
            printf("Test 5 completado exitosamente! :)\n");
        else
            printf("Test 5 fallido.\n");
        
    }
    printf("Todos los tests han finalizado. Felicidades!\n");
    return 0;
}
