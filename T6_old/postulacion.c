#include <stdlib.h>
#include <stdio.h>    // printf
#include <unistd.h>   // sleep
#include <pthread.h>
// t6.c
#include "pss.h"
#include "postulacion.h"
#include "spinlocks.h"

typedef struct {
    Estudiante *est;
    double *ranking;
} Postulacion;

PriQueue *priQueue[N];
int postulacionTrabajos[N]; // id de los estudiantes que hayan conseguido el trabajo
int* q[N]; // arreglo de 
int mutex = OPEN;

void iniciarPostulaciones(){
  for (int i = 0; i < N; i++) {
    priQueue[i] = makePriQueue();
    postulacionTrabajos[i] = -1;
  }
}

void destruirPostulaciones(){
  for (int i = 0; i < N; i++)
    destroyPriQueue(priQueue[i]);
}


// preferencias: puntero a un arreglo de tamaño N con valores 0 en los índices de los trabajos donde no quiere postular y 1 donde sí quiere
// rank: puntero a un arreglo double donde estarán el ranking con el cual postulará
void postularTrabajo(Estudiante *est, int *preferencias, double *rank) {
  //printf("El estudiante %d va a postular a algun trabajo\n", est->id);
  spinLock(&mutex);
  est->trabajo_id = -1;
  Postulacion postulacion = {est, rank};

  if(est->id==99)
    printf("-----------------Hola soy el estudinate 99\n");

  for (int i = 0; i < N; i++) {
      if (postulacionTrabajos[i]==-1 && preferencias[i]){ // si aún no hay nadie electo para el trabajo y está dentro de sus preferencias
          //printf("El estudiante %d postulo al trabajo %d, con ranking %f\n", est->id, i, *rank);
          priPut(priQueue[i], &postulacion, rank[i]);     // se agrega a la cola de prioridad correspondiente
      }
  }

  int lk = CLOSED;
  q[est->id] = &lk;
  if(est->trabajo_id == -1) {
    spinUnlock(&mutex);
    spinLock(&lk);
  }
  
}



int cerrarPostulacion(int i) { // aquí i se refiere al numero del trabajo, no al id del estudiante
  //printf("El estudiante %d cierra su postulación \n", i);
  spinLock(&mutex);
  
  
  while (postulacionTrabajos[i]==-1) {
    while(!emptyPriQueue(priQueue[i])){
      Postulacion *ppostulacion = priGet(priQueue[i]); //sacar el primer elemento de la cola de prioridad 
      
      if (ppostulacion->est->trabajo_id == -1) { // si el estudiante aún no tiene trabajo
        postulacionTrabajos[i] = ppostulacion->est->id;
        ppostulacion->est->trabajo_id = i;
        for(int j = 0; j < N; j++){ // se elimina la postulación de todas las colas de prioridad
          priDel(priQueue[j], ppostulacion);
        }
        spinUnlock(q[ppostulacion->est->id]);
        break;
      }
    } 
    
  }
  int res = postulacionTrabajos[i];
  spinUnlock(&mutex);
  return res;
}
