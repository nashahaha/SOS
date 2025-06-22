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
int* q_est[N]; // arreglo de estudiantes , marca si están en espera
int* q_tra[N];
int mutex = OPEN;

void iniciarPostulaciones(){
  for (int i = 0; i < N; i++) {
    priQueue[i] = makePriQueue();
    postulacionTrabajos[i] = -1;
    int lk= OPEN;
    q_tra[i] = &lk; 
    q_est[i] = &lk; //R
  }
}

void destruirPostulaciones(){
  for (int i = 0; i < N; i++)
    destroyPriQueue(priQueue[i]);
}


// preferencias: puntero a un arreglo de tamaño N con valores 0 en los índices de los trabajos donde no quiere postular y 1 donde sí quiere
// rank: puntero a un arreglo double donde estarán el ranking con el cual postulará
void postularTrabajo(Estudiante *est, int *preferencias, double *rank) {
  spinLock(&mutex);
  est->trabajo_id = -1;
  Postulacion postulacion = {est, rank};

  for (int i = 0; i < N; i++) {
      if (postulacionTrabajos[i]==-1 && preferencias[i]){ // si aún no hay nadie electo para el trabajo y está dentro de sus preferencias
          priPut(priQueue[i], &postulacion, rank[i]);     // se agrega a la cola de prioridad correspondiente

          if(*q_tra[i]==CLOSED){ // despierto al trabajo si está dormido
            spinUnlock(q_tra[i]);
          }
      }
  }

  int lk = CLOSED;
  q_est[est->id] = &lk;
  if(est->trabajo_id == -1) {
    spinUnlock(&mutex);
    spinLock(&lk);
  }else
    spinUnlock(&mutex);

}



int cerrarPostulacion(int i) { // aquí i se refiere al numero del trabajo, no al id del estudiante
  spinLock(&mutex);

  if(postulacionTrabajos[i]!=-1 || emptyPriQueue(priQueue[i])){ // Si no puede asignar a nadie se va a espera
    int lk = CLOSED;
    q_tra[i] = &lk;
    spinUnlock(&mutex);
    spinLock(&lk);
  }
  
  Postulacion *ppostulacion = priGet(priQueue[i]); //sacar el primer elemento de la cola de prioridad 
      
  if (ppostulacion->est->trabajo_id == -1) { // si el estudiante aún no tiene trabajo
    postulacionTrabajos[i] = ppostulacion->est->id;
    ppostulacion->est->trabajo_id = i;
    for(int j = 0; j < N; j++){ // se elimina la postulación de todas las colas de prioridad
      priDel(priQueue[j], ppostulacion);
    }
    spinUnlock(q_est[ppostulacion->est->id]); // despierta al estudiente
  }
  
  

  int res = postulacionTrabajos[i];
  spinUnlock(&mutex);
  return res;
}
