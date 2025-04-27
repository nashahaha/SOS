#include <pthread.h>
#include <stdio.h>
#include "sat.h"

typedef int (*BoolFun)(int x[]);
int cnt= 0;
void gen_par(int x[], int i, int n, BoolFun f, int p);


void gen_sec(int x[], int i, int n, BoolFun f, int p){ //para i>=p
    if (i==n){
        if ((*f)(x))
            cnt++;
    }else{
        x[i]= 0; gen_sec(x, i+1, n, f, p);
        x[i]= 1; gen_sec(x, i+1, n, f, p);
    }
}

typedef struct {
    int *x; //x[]
    int i, n;
    BoolFun f;
    int p;
} Args;

void *thread_function(void *p) {
    Args *arg = (Args *)p; // se castea el argumento
    gen_par(arg->x, arg->i, arg->n, arg->f, arg->p); // *(arg.a) == arg->a
    return NULL;
}

void gen_par(int x[], int i, int n, BoolFun f, int p){ //para i<p
    int *a = x;
    if (i==n){
        if ((*f)(a))
            cnt++;
    }else if(i<p){
        pthread_t pid;
        Args args = {a, i+1, n, f, p/2};
        a[i]= 0; 
        pthread_create(&pid, NULL, thread_function, &args);
        
        a[i]= 1; 
        gen_par(a, i+1, n, f, p - p/2);
        pthread_join(pid, NULL);
    } else{ // i>=p
        x[i]= 0; gen_sec(x, i+1, n, f, p);
        x[i]= 1; gen_sec(x, i+1, n, f, p);
    }
}



int recuento(int n, BoolFun f, int p) {
    int x[n];
    gen_par(x, 0, n, f, p);
    return cnt;
}

