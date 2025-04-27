#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "pss.h"
#include "rwlock.h"

struct rwlock {
  pthread_mutex_t m;
  int readers, writing;
  Queue *q_writer, *q_reader;
  //... defina aca los campos que necesita para el tipo RWLock ...
};

RWLock *makeRWLock() {
  RWLock *lock = malloc(sizeof(RWLock));

  pthread_mutex_init(&lock->m, NULL);
  lock->readers = 0, lock->writing = 0;
  lock->q_writer = makeQueue();
  lock->q_reader = makeQueue();

  return lock;
}

void destroyRWLock(RWLock *rwl) {
  pthread_mutex_destroy(&rwl->m);
}

typedef enum {READER, WRITER} Kind;

typedef struct {
  int ready;
  pthread_cond_t w;
} Request;

void enqueue(RWLock *rwl, Kind kind){
  Request req = {0, PTHREAD_COND_INITIALIZER};

  if(kind==WRITER) // si es escritor espera en la cola de escritores
    put(rwl->q_writer, &req);
  else // es un lector
    put(rwl->q_reader, &req);
  
  while(!req.ready) //mientras el request no esté listo, espera
    pthread_cond_wait(&req.w, &rwl->m);
}

void wakeup(RWLock *rwl, Kind kind){
  if(kind==WRITER){
    Request *r = get(rwl->q_writer);  
    if(r==NULL)
      return;
    
    rwl->writing = 1;
    r->ready = 1; //para hacer verdadero el while
    pthread_cond_signal(&r->w); //despertamos la condición que corresponda
  
  } else if(kind==READER) { // si el tipo es lector
    Request *r = get(rwl->q_reader);

    printf("Aquí debería haber un lector, su ready es %d", r->ready);

    if(r==NULL)
      return;

    do { //entran lectores consecutivos
      r->ready = 1;
      pthread_cond_signal(&r->w);
      get(rwl->q_reader); //obtengo el siguiente
    } while (r!=NULL);
  }

}


void enterRead(RWLock *rwl) {
  pthread_mutex_lock(&rwl->m);
  
  if(!(rwl->writing) && emptyQueue(rwl->q_writer)) // Se suma un lector (puede entrar) si no hay escritores trabajando o en espera 
  rwl->readers++; //agregamos un reader
  else
    enqueue(rwl, READER);
  
  pthread_mutex_unlock(&rwl->m);
}

void exitRead(RWLock *rwl) {
  pthread_mutex_lock(&rwl->m);

  rwl->readers--;
  if(rwl->readers==0 && !emptyQueue(rwl->q_writer)) // no quedan lectores trabajando y hay escritores en espera
    wakeup(rwl, WRITER);

  pthread_mutex_unlock(&rwl->m);
}

void enterWrite(RWLock *rwl) {
  pthread_mutex_lock(&rwl->m);

  if(rwl->readers==0 && !rwl->writing) // marca un escritor (puede entrar) solo si no hay lectores ni escritores usando el mutex
    rwl->writing = 1; //marca un escritor
  else // si no queda en espera
    enqueue(rwl, WRITER);
  
  pthread_mutex_unlock(&rwl->m);
}

void exitWrite(RWLock *rwl) {
  pthread_mutex_lock(&rwl->m);
  rwl->writing = 0; //declara que nadie está escribiendo

  printf("La cola de escritores es %d\n", queueLength(rwl->q_writer));
  printf("La cola de lectores es %d\n", queueLength(rwl->q_reader));

  if(!emptyQueue(rwl->q_reader)) {// hay solicitudes de lectores
    printf("Aquí deberían despertar los lectores, actualmente hay %d leyendo\n", rwl->readers);
    wakeup(rwl, READER);
  }
  else if (emptyQueue(rwl->q_reader) && !emptyQueue(rwl->q_writer)) // no hay lectores en espera y hay escritores en espera
    wakeup(rwl, WRITER);
  // WAKEUP -> lanza un signal a quien corresponda
  // si hay solicitudes de lectores se aceptan todas
  // si no hay lectores pendientes pero hay solicitudes de escritores, se acepta al escritor que lleve más tiempo esperando
  pthread_mutex_unlock(&rwl->m);
}

