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
  //... defina aca los campos que necesita para el tipo RWLock ...
};

typedef enum {READER, WRITER} Kind;

typedef struct {
  int ready;
  pthread_cond_t w;
} Request;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int readers = 0, writing = 0;
Queue *q_writer, *q_reader;


RWLock *makeRWLock() {
  //...
}

void destroyRWLock(RWLock *rwl) {
  //...
}

void enterRead(RWLock *rwl) {
  pthread_mutex_lock(&m);
  
  if(!writing && emptyQueue(q_writer)) // Se suma un lector (puede entrar) si no hay escritores trabajando o en espera 
    readers++; //agregamos un reader
  else
    enqueue(READER);
  
  pthread_mutex_unlock(&m);
}

void exitRead(RWLock *rwl) {
  pthread_mutex_lock(&m);

  readers--;
  if(readers==0 && !emptyQueue(q_writer)) // no quedan lectores trabajando y hay escritores en espera
    wakeup(WRITER);

  pthread_mutex_unlock(&m);
}

void enterWrite(RWLock *rwl) {
  pthread_mutex_lock(&m);

  if(readers==0 && !writing) // marca un escritor (puede entrar) solo si no hay lectores ni escritores usando el mutex
    writing = 1; //marca un escritor
  else // si no queda en espera
    enqueue(WRITER);
  
  pthread_mutex_unlock(&m);
}

void exitWrite(RWLock *rwl) {
  pthread_mutex_lock(&m);
  writing = 0; //declara que nadie está escribiendo
  if(!emptyQueue(q_reader)) // hay solicitudes de lectores
    wakeup(READER);
  else if (emptyQueue(q_reader) && !emptyQueue(q_writer)) // no hay lectores en espera y hay escritores en espera
    wakeup(WRITER);
  // WAKEUP -> lanza un signal a quien corresponda
  // si hay solicitudes de lectores se aceptan todas
  // si no hay lectores pendientes pero hay solicitudes de escritores, se acepta al escritor que lleve más tiempo esperando
  pthread_mutex_unlock(&m);
}

void enqueue(Kind kind){
  Request req = {0, PTHREAD_COND_INITIALIZER};

  if(kind==WRITER) // si es escritor espera en la cola de escritores
    put(q_writer, &req);
  else // es un lector
    put(q_reader, &req);
  
  while(!req.ready) //mientras el request no esté listo, espera
    pthread_cond_wait(&req.w, &m);
}

void wakeup(Kind kind){
  if(kind==WRITER){
    Request *r = get(q_writer);  
    if(r==NULL)
      return;
    
    writing = 1;
    r->ready = 1; //para hacer verdadero el while
    pthread_cond_signal(&r->w); //despertamos la condición que corresponda
  
  } else { // si el tipo es lector
    
    Request *r = get(q_reader);
    if(r==NULL)
      return;

    do { //entran lectores consecutivos
      r->ready = 1;
      pthread_cond_signal(&r->w);
      get(q_reader); //obtengo el siguiente
    } while (r!=NULL);
  }

}