#include <nthread-impl.h>
#include "subasta.h"
#include "pss.h" // para usar colas de prioridad

// =============================================================
// Implemente a partir de aca el tipo Subasta, puede agregar
// funciones, estructuras o variables que considere necesarias
// =============================================================

struct subasta{
    int n_items; // número de items ofrecidos
    PriQueue *q; // algo para guardar las ofertas aceptadas
    State status;
};

// Se subastarán n items
// retorna una subasta
nSubasta nNuevaSubasta(int n){
    nSubasta subasta = nMalloc(sizeof(nSubasta));
    subasta->n_items = n;
    subasta->q = makePriQueue();
    return subasta;
}
typedef enum {PEND, RECHAZ, ADJUD} Resol;

typedef struct {
    nThread th;    // El hilo que hizo la oferta
    Resol estado;  // El estado de la oferta
} Oferta;

// ofrece 'oferta' a la subasta s
// retorna TRUE o FALSE
// TRUE si se adjudica un producto
// FALSE si se presentaron n oferentes con un precio mayor a la oferta realizada
int nOfrecer(nSubasta s, double oferta) {
    START_CRITICAL
    
    // Crear una nueva oferta
    Oferta *nueva_oferta = nMalloc(sizeof(Oferta));
    nueva_oferta->th = nSelf();    // Guarda el hilo actual
    nueva_oferta->estado = PEND;   // Marca la oferta como pendiente

    // Agregar la oferta a la cola con su prioridad
    priPut(s->q, nueva_oferta, oferta);

    // Si hay más ofertas que productos, rechazar la oferta más baja
    if (priLength(s->q) > s->n_items) {
        Oferta *peor_oferta = (Oferta *)priGet(s->q); // Remueve la peor oferta
        peor_oferta->estado = RECHAZ;                 // Marca como rechazada
        setReady(peor_oferta->th);                    // Reactiva el hilo rechazado

        return FALSE;                                  // Permite cambios de contexto
    }

    // Esperar a ser adjudicado o rechazado
    while (nueva_oferta->estado == PEND) {
        suspend(WAIT_SUBASTA);
        schedule();
    }

    int resultado = (nueva_oferta->estado == ADJUD);
    free(nueva_oferta);  // Liberar memoria de la oferta
    END_CRITICAL
    return resultado;
}



// cierra la subasta
// s -> subasta por cerrar
// *punid -> queda el número de unidades que no fueron vendidas (llegaron mejos oferentes que los n items ofrecidos)

// retorna el monto total recaudado
double nAdjudicar(nSubasta s, int *punid) {
    START_CRITICAL
    double monto = 0.0;
    *punid = 0;

    while (!emptyPriQueue(s->q)) {
        double oferta = priBest(s->q);
        Oferta *mejor_oferta = (Oferta *)priGet(s->q);
        mejor_oferta->estado = ADJUD;  // Adjudica el producto
        setReady(mejor_oferta->th);    // Reactiva el hilo adjudicado
        monto += oferta;
        (*punid)++;
    }

    END_CRITICAL
    return monto;
}


// destruye los recursos de una subasta
void nDestruirSubasta(nSubasta s){
    destroyPriQueue(s->q);
    free(s);
}
