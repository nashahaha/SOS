#include <pthread.h>
#include <stdio.h>
#include "sat.h"

typedef int (*BoolFun)(int x[]);
//int cnt= 0;
int gen_par(int x[], int i, int n, BoolFun f, int p);


int gen_sec(int x[], int i, int n, BoolFun f, int p){ //para i>=p
    int r1 = 0;
    int r2 = 0;
    if (i==n){
        if ((*f)(x))
            return 1;
        else
            return 0;
    }else{
        x[i]= 0; //error
        r1 += gen_sec(x, i+1, n, f, p);//error
        x[i]= 1; 
        r2 += gen_sec(x, i+1, n, f, p);
        return r1+r2;     
    }
}

typedef struct {
    int *x; //x[]
    int i, n;
    BoolFun f;
    int p;
    int res;
} Args;

void *thread_function(void *p) {
    Args *arg = (Args *)p; // se castea el argumento
    arg->res = gen_par(arg->x, arg->i, arg->n, arg->f, arg->p); // *(arg.a) == arg->a
    return NULL;
}

int gen_par(int x[], int i, int n, BoolFun f, int p){ //para i<p
    int *a = x;
    int r1 = 0;
    int r2 = 0;
    if (i==n){ // cuando ya se asignan todas las variables
        if ((*f)(a)) // se evalua en f
            return 1;
        else
            return 0;
    }else if(i<p){
        pthread_t pid;
        Args args = {a, i+1, n, f, p/2, 0};
        a[i]= 0; 
        pthread_create(&pid, NULL, thread_function, &args); //error
        
        a[i]= 1; 
        r2 += gen_par(a, i+1, n, f, p - p/2); //ERROR
        
        pthread_join(pid, NULL);
        
        return args.res + r2;
        
    } else{ // i>=p
        a[i]= 0; //ERRoR
        r1 += gen_sec(a, i+1, n, f, p); //error
        a[i]= 1; 
        r2 += gen_sec(a, i+1, n, f, p);
        return r1+r2;  
    }
}



int recuento(int n, BoolFun f, int p) {
    int x[n];
    return gen_par(x, 0, n, f, p);
}

