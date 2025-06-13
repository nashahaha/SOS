#include <nthread-impl.h>
#include "subasta.h"

struct subasta{
    int n_items; // número de items ofrecidos
    PriQueue *q; // algo para guardar las ofertas aceptadas
};

nSubasta nNuevaSubasta(int n){
    nSubasta subasta = nMalloc(sizeof(struct subasta));
    subasta->n_items = n;
    subasta->q = makePriQueue();
    return subasta;
}

//===============================
// Funciones y estructuras extra
//===============================

typedef enum {PEND, RECHAZ, ADJUD} Resol;

typedef struct {
    nThread th;    // El hilo que hizo la oferta
    double monto;
    nSubasta subasta;
    Resol estado;  // El estado de la oferta
} Oferta;


void elimOferta(nThread this_Th){
    Oferta *oferta_ptr = (Oferta *)(this_Th-> ptr);
    if (oferta_ptr != NULL){
        priDel(oferta_ptr->subasta->q, oferta_ptr);
        this_Th->ptr = NULL;
    }
}

//================================================

int nOfrecer(nSubasta s, double oferta, int timeout){
    START_CRITICAL
    
    // Crear una nueva oferta
    Oferta *nueva_oferta = nMalloc(sizeof(Oferta));
    nueva_oferta->monto = oferta;
    nueva_oferta->subasta = s;
    nueva_oferta->th = nSelf();    // Guarda el hilo actual
    nueva_oferta->estado = PEND;   // Marca la oferta como pendiente

    // Agregar la nueva oferta el campo ptr del thread
    nThread this_th = nSelf();
    this_th -> ptr = nueva_oferta;

    // Agregar la oferta a la cola con su prioridad
    priPut(s->q, nueva_oferta, oferta);

    // Si hay más ofertas que productos, rechazar la oferta más baja
    if (priLength(s->q) > s->n_items) {
        Oferta *peor_oferta = (Oferta *)priGet(s->q); // Remueve la peor oferta
        
        // Si es el hilo actual, retorna inmediatamente
        if (peor_oferta == nueva_oferta) {
            END_CRITICAL;
            return FALSE;        // Retorna inmediatamente
        }

        // Si es otro hilo, rechazarlo y reactivarlo
        peor_oferta->estado = RECHAZ;
        if(peor_oferta->th->status == WAIT_SUBASTA_TIMEOUT){
            nth_cancelThread(peor_oferta->th);
        }

        
        setReady(peor_oferta->th);  // Reactiva el hilo rechazado
        schedule();                 // Libera el CPU para otros hilos
    }

    if(nueva_oferta->estado == PEND){
        if(timeout > 0){
            // programar timer
            nth_programTimer(timeout * 1000000LL, elimOferta);
            suspend(WAIT_SUBASTA_TIMEOUT);
        } else {
            suspend(WAIT_SUBASTA);
        }
        schedule();
    }

    int resultado = (nueva_oferta->estado == ADJUD);
    
    END_CRITICAL
    return resultado;
}

double nAdjudicar(nSubasta s,int *punid ){
    START_CRITICAL
    double monto = 0.0;
    *punid = s->n_items;
    int uVendidas = 0;

    while (!emptyPriQueue(s->q)) {
        monto += priBest(s->q);  // Suma el monto de la mejor oferta
        Oferta *mejor_oferta = (Oferta *)priGet(s->q);
        
        // Adjudica el producto
        mejor_oferta->estado = ADJUD; //para que los threads retornen true
        
        uVendidas++;

        if(mejor_oferta->th->status == WAIT_SUBASTA_TIMEOUT){
            nth_cancelThread(mejor_oferta->th);
        }

        setReady(mejor_oferta->th);   // Reactiva el hilo adjudicado para que pueda retornar
        schedule();
    }
    (*punid)-=uVendidas;
    END_CRITICAL
    return monto;
}

void nDestruirSubasta(nSubasta s){
    destroyPriQueue(s->q);
    free(s);
}
