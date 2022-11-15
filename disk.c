#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "disk.h"
#include "priqueue.h"
#include "spinlocks.h"

// Le sera de ayuda la clase sobre semáforos:
// https://www.u-cursos.cl/ingenieria/2022/2/CC4302/1/novedades/detalle?id=431689
// Le serviran la solucion del productor/consumidor resuelto con el patron
// request y la solucion de los lectores/escritores, tambien resuelto con
// el patron request.  Puede substituir los semaforos de esas soluciones
// por spin-locks, porque esos semaforos almacenan a lo mas una sola ficha.


// Declare los tipos que necesite
PriQueue *mayor_queue;
PriQueue *menor_queue;
// Declare aca las variables globales que necesite
int current_track; 
int sl;
// Agregue aca las funciones requestDisk y releaseDisk

void iniDisk(void) {
  sl = OPEN;
  current_track = -1; // Comienza desocupado
  mayor_queue = makePriQueue();
  menor_queue = makePriQueue();
}

void requestDisk(int track) {
  // Garantizamos exclusion mutua
  spinLock(&sl);
  if (current_track == -1) {
    // No esta ocupado
    current_track = track;
  } else {
    // Esta ocupado, esperamos
    int w = CLOSED; 
    // Veamos prioridad
    int priority = track - current_track;
    if (track >= current_track) { 
      priPut(mayor_queue, &w, priority); // Siempre >= 0 
      spinUnlock(&sl); // suelto el spinlock mutex para que otro core pueda entrar a la zona de exclusión mutua.
      spinLock(&w); // espero sobre w. 
      current_track = track;
      return;
    } else {
      priPut(menor_queue, &w, track);
      spinUnlock(&sl); // suelto el spinlock mutex para que otro core pueda entrar a la zona de exclusión mutua.
      spinLock(&w); // espero sobre w. 
      current_track = track;
      return;
    }

  }
  spinUnlock(&sl);
}

void releaseDisk() {

  spinLock(&sl);

  int borde = 1;
  int best = -1;
  if (!emptyPriQueue(menor_queue)) {
    best = priBest(menor_queue);
    if (best > current_track) {
      borde = 0;
    }
  }

  if (!emptyPriQueue(mayor_queue) && borde) {
    int *pw = priGet(mayor_queue);
    spinUnlock(pw);
  } else if (!emptyPriQueue(menor_queue)) {
    int *pw = priGet(menor_queue);
    spinUnlock(pw);
  }else  {
    current_track = -1;
  }
  spinUnlock(&sl);
}
