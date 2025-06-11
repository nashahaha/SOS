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
int mutex_ = OPEN;
int cond = OPEN;

// preferencias: puntero a un arreglo de tamaño N con valores 0 en los índices de los trabajos donde no quiere postular y 1 donde sí quiere
// rank: puntero a un arreglo double donde estarán el ranking con el cual postulará
void postularTrabajo(Estudiante *est, int *preferencias, double *rank) {
  
  spinLock(&mutex_);
  est->trabajo_id = -1;
  Postulacion postulacion = {est, rank};
  for (int i = 0; i < N; i++) {
      if (postulacionTrabajos[i]==-1 && preferencias[i]) // si aún no hay nadie electo para el trabajo y está dentro de sus preferencias
          priPut(priQueue[i], &postulacion, rank[i]); // se agrega a la cola de prioridad correspondiente
  }


  if (est->trabajo_id== -1)
    spinLock(&cond);

  spinUnlock(&mutex_);
}

int cerrarPostulacion(int i) {
  spinLock(&mutex_);
  while (postulacionTrabajos[i]==-1) {
    while(!emptyPriQueue(priQueue[i])){
      Postulacion *ppostulacion = priGet(priQueue[i]);
      
      if (ppostulacion->est->trabajo_id == -1) {
        postulacionTrabajos[i] = ppostulacion->est->id;
        ppostulacion->est->trabajo_id = i;
        for(int j = 0; j < N; j++)
          priDel(priQueue[j], ppostulacion);
        break;
      }
    }
    spinUnlock(&cond);
  }
  spinUnlock(&mutex_);
  return postulacionTrabajos[i];
}
