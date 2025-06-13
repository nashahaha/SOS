#define _XOPEN_SOURCE 500

#include "nthread-impl.h"

#include "disk.h"

int u = 0;
int Ocupado = 0;
PriQueue *q0;
PriQueue *q1;

void swapQueues(void) {
  PriQueue *temp = q0;
  q0 = q1;
  q1 = temp;
}

void elimTh(nThread this_Th) {
  nThread track_ptr = (this_Th -> ptr);
  if (track_ptr != NULL) {
    priDel(q0, this_Th);
    priDel(q1, this_Th);
    this_Th -> ptr = NULL;
  }
}

void iniDisk(void) {
  q0 = makePriQueue();
  q1 = makePriQueue();
}

void cleanDisk(void) {
  destroyPriQueue(q0);
  destroyPriQueue(q1);
}

int nRequestDisk(int track, int timeout) {
  START_CRITICAL;

  if (Ocupado == 1) {
    nThread this_Th = nSelf();
    this_Th -> ptr = &track;
    if (track >= u) {
      priPut(q0, this_Th, track);
      if (timeout > 0) {
        nth_programTimer(timeout * 1000000LL, elimTh);
        suspend(WAIT_REQUEST_TIMEOUT);
      } 
      else if (timeout < 0) {
        suspend(WAIT_REQUEST);
      }
    } else {
      priPut(q1, this_Th, track);
      if (timeout > 0) {
        nth_programTimer(timeout * 1000000LL, elimTh);
        suspend(WAIT_REQUEST_TIMEOUT);
      } 
      else if (timeout < 0) {
        suspend(WAIT_REQUEST);
      }
    }
    schedule();
    if (this_Th -> ptr == NULL) {
      END_CRITICAL;
      return 1;
    }
  }

  Ocupado = 1;
  u = track;
  END_CRITICAL;
  return 0;
}

void nReleaseDisk(void) {
  START_CRITICAL;
  nThread t = NULL;
  if (!emptyPriQueue(q0)) {
    t = priGet(q0);
    if (t -> status == WAIT_REQUEST_TIMEOUT) {
      nth_cancelThread(t);
    }
    setReady(t);
  } 
  else if (!emptyPriQueue(q1)) {
    swapQueues();
    t = priGet(q0);
    if (t -> status == WAIT_REQUEST_TIMEOUT) {
      nth_cancelThread(t);
    }
    setReady(t);
  } 
  else {
    Ocupado = 0;
  }
  schedule();
  END_CRITICAL;
  return;
}


