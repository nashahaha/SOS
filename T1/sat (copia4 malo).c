#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef int (*BoolFun)(int x[]);

int gen_par(int x[], int i, int n, BoolFun f, int p);

int gen_sec(int x[], int i, int n, BoolFun f, int p) {
    if (i == n) {
        return (*f)(x) ? 1 : 0;
    }
    x[i] = 0;
    int r1 = gen_sec(x, i + 1, n, f, p);
    x[i] = 1;
    int r2 = gen_sec(x, i + 1, n, f, p);
    return r1 + r2;
}

typedef struct {
    int *x;
    int i, n;
    BoolFun f;
    int p;
    int res;
} Args;

void *thread_function(void *arg) {
    Args *args = (Args *)arg;
    args->res = gen_par(args->x, args->i, args->n, args->f, args->p);
    return NULL;
}

int gen_par(int x[], int i, int n, BoolFun f, int p) {
    if (i == n) {
        return (*f)(x) ? 1 : 0;
    }
    
    if (i < p) {
        pthread_t thread1, thread2;
        Args args1 = {x, i + 1, n, f, p, 0};
        Args args2 = {x, i + 1, n, f, p, 0};

        // Assign values before creating threads
        x[i] = 0;
        pthread_create(&thread1, NULL, thread_function, &args1);
        
        x[i] = 1;
        pthread_create(&thread2, NULL, thread_function, &args2);

        // Wait for both threads to finish
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        return args1.res + args2.res;
    } else {
        return gen_sec(x, i, n, f, p);
    }
}

int recuento(int n, BoolFun f, int p) {
    int x[n];
    return gen_par(x, 0, n, f, p);
}



