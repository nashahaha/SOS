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
  Kind kind;
  pthread_cond_t w;
} Request;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int readers = 0, writing = 0;
Queue *q;

RWLock *makeRWLock() {
  //...
}

void destroyRWLock(RWLock *rwl) {
  //...
}

void enterRead(RWLock *rwl) {
  pthread_mutex_lock(&m);

  
  pthread_mutex_unlock(&m);
}

void exitRead(RWLock *rwl) {
  pthread_mutex_lock(&m);


  pthread_mutex_unlock(&m);
}

void enterWrite(RWLock *rwl) {
  pthread_mutex_lock(&m);

  if(readers==0 && !writing) // marca un escritor (puede entrar) solo si no hay lectores ni escritores usando el mutex
    writing = 1;
  else // si no queda en espera
    enqueue(WRITER);
  
  pthread_mutex_unlock(&m);
}

void exitWrite(RWLock *rwl) {
  pthread_mutex_lock(&m);
  writing = 0; //declara que nadie está escribiendo
  //WAKEUP -> lanza un signal a quien corresponda
  pthread_mutex_unlock(&m);
}
