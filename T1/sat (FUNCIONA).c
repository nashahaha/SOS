#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef int (*BoolFun)(int x[]);

int gen_par(int x[], int i, int n, BoolFun f, int p);

int gen_sec(int x[], int i, int n, BoolFun f, int p) { // i >= p
    if (i == n) {
        return f(x) ? 1 : 0; // 1 si f(x) es true, 0 sino
    }
    
    int r1 = 0, r2 = 0;
    
    x[i] = 0;
    r1 = gen_sec(x, i + 1, n, f, p);
    
    x[i] = 1;
    r2 = gen_sec(x, i + 1, n, f, p);

    return r1 + r2;
}

typedef struct {
    int *x;
    int i, n, p;
    BoolFun f;
    int res;
} Args;

void *thread_function(void *p) {
    Args *arg = (Args *)p;
    
    int *x_copy = (int *)malloc(arg->n * sizeof(int));
    memcpy(x_copy, arg->x, arg->n * sizeof(int));

    arg->res = gen_par(x_copy, arg->i, arg->n, arg->f, arg->p);
    
    free(x_copy);
    return NULL;
}

int gen_par(int x[], int i, int n, BoolFun f, int p) {
    if (i == n) {
        return f(x) ? 1 : 0;
    }

    int r1 = 0, r2 = 0;

    if (i < p) { // Use parallel execution
        pthread_t pid1;
        Args args1 = {malloc(n * sizeof(int)), i + 1, n, p, f, 0}; 
        
        
        pthread_t pid2;
        Args args2 = {malloc(n * sizeof(int)), i + 1, n, p, f, 0};
        
        x[i] = 0;
        memcpy(args1.x, x, n * sizeof(int));
        pthread_create(&pid1, NULL, thread_function, &args1);

        x[i] = 1;
        memcpy(args2.x, x, n * sizeof(int));
        pthread_create(&pid2, NULL, thread_function, &args2);   

        pthread_join(pid1, NULL);
        r1 = args1.res;
        
        pthread_join(pid2, NULL);
        r2 = args2.res;
        
        free(args1.x);
        free(args2.x);
        
        return r1 + r2;
    } else { //i>=p
        x[i] = 0;
        r1 = gen_sec(x, i + 1, n, f, p);
        
        x[i] = 1;
        r2 = gen_sec(x, i + 1, n, f, p);

        return r1 + r2;
    }
}

int recuento(int n, BoolFun f, int p) {
    int *x = (int *)malloc(n * sizeof(int)); //revisar esto, no parece ser necesario
    memset(x, 0, n * sizeof(int));

    int result = gen_par(x, 0, n, f, p);
    
    free(x);
    return result;
}


