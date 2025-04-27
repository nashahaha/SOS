#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include "disco.h"

void discoInit(void) {
  
}

void discoDestroy(void) {
  //pthread_mutex_destroy(&m);
  //ptrhead_cond_destroy(&c);
}

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int ticket_d = 1, ticket_v = 1, display = 1, dis_v = 0, dis_d = 0;

char *nom_dama = NULL;
char *nom_varon = NULL;

char *dama(char *nom) {
    pthread_mutex_lock(&m);
    int my_num = ticket_d++;
    
    while (my_num != display) {
        pthread_cond_wait(&c, &m);
    }

    // Asignar su nombre a la variable global
    nom_dama = strcmp(nom); // Usar strdup para evitar problemas de punteros
    dis_d++;

    printf("+++++++El nombre global de dama es: %s \n", nom_dama);
    pthread_cond_broadcast(&c);

    while (dis_d != dis_v) {
        pthread_cond_wait(&c, &m);
    }

    pthread_mutex_unlock(&m);
    return nom_varon;
}

char *varon(char *nom) {
    pthread_mutex_lock(&m);
    int my_num = ticket_v++;
    
    while (my_num != display) {
        pthread_cond_wait(&c, &m);
    }

    // Asignar su nombre a la variable global
    nom_varon = strcmp(nom); // Usar strdup para evitar problemas de punteros
    dis_v++;

    printf("+++++++El nombre global de varon es: %s \n", nom_varon);

    while (dis_d != dis_v) {
        pthread_cond_wait(&c, &m);
    }

    display++;

    printf("nombre varon global: %s\n", nom_varon);
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);

    return nom_dama;
}

