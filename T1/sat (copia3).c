#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef int (*BoolFun)(int x[]);

int gen_par(int x[], int i, int n, BoolFun f, int p);

int gen_sec(int x[], int i, int n, BoolFun f, int p) { // For i >= p
    if (i == n) {
        return f(x) ? 1 : 0;
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
    
    // Make a copy of x[] to avoid race conditions
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
        pthread_t pid;
        Args args = {malloc(n * sizeof(int)), i + 1, n, p-1, f, 0}; //ERROR
        memcpy(args.x, x, n * sizeof(int));

        x[i] = 0;
        pthread_create(&pid, NULL, thread_function, &args);

        x[i] = 1;
        r2 = gen_par(x, i + 1, n, f, p-2); //ERROR

        pthread_join(pid, NULL);
        r1 = args.res;
        free(args.x);
        
        return r1 + r2;
    } else { // Sequential execution
        x[i] = 0;
        r1 = gen_sec(x, i + 1, n, f, p);
        
        x[i] = 1;
        r2 = gen_sec(x, i + 1, n, f, p);

        return r1 + r2;
    }
}

int recuento(int n, BoolFun f, int p) {
    int *x = (int *)malloc(n * sizeof(int));
    memset(x, 0, n * sizeof(int));

    int result = gen_par(x, 0, n, f, p);
    
    free(x);
    return result;
}


