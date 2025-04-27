#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "disco.h"

void discoInit(void) {
  
}

void discoDestroy(void) {

}

// declare aca sus variables globales
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int ticket_d = 0, ticket_v = 0, dis_v=0, dis_d=0;

char *nom_dama = NULL, *nom_varon = NULL;

char *dama(char *nom) {
  pthread_mutex_lock(&m);
  int my_num = ticket_d++; //saca un número

  while(my_num != dis_d || nom_dama!=NULL){ // espera mientras no sea su turno y el espacio para el nombre esté ocupado
    pthread_cond_wait(&c, &m);
  }
  
  nom_dama = nom; // pone su nombre en la variable global
  
  while(nom_varon==NULL){ //espera mientras el varon pone su nombre en la variable global
    pthread_cond_wait(&c, &m);
  }

  char *pareja = nom_varon; //guarda el nombre de la pareja
  nom_varon = NULL; //una vez se obtiene el nombre de la pareja lo quita de la variable global
  dis_d++; //pasa el turno
  
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
  return pareja;
}

char *varon(char *nom) {
  pthread_mutex_lock(&m);
  int my_num = ticket_v++;

  while(my_num != dis_v || nom_varon!=NULL){ 
    pthread_cond_wait(&c, &m);
  }

  nom_varon = nom;

  while( nom_dama==NULL){
    pthread_cond_wait(&c, &m);
  }

  char *pareja = nom_dama;
  nom_dama = NULL;
  dis_v++;

  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);

  return pareja;
}
