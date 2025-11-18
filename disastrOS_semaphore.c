#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_globals.h"
#include "disastrOS_semaphore.h"
#include "pool_allocator.h"
#include "assert.h"
#include "disastrOS_pcb.h"

#define SEMAPHORE_SIZE        sizeof(Semaphore)
#define SEMAPHORE_MEMSIZE    (sizeof(Semaphore) + sizeof(int))
#define SEMAPHORE_BUFFER_SIZE (DSOS_MAX_SEMAPHORES * SEMAPHORE_MEMSIZE)

static char _semaphore_buffer[SEMAPHORE_BUFFER_SIZE];
static PoolAllocator _semaphore_allocator;

void Semaphore_init(void) {
  int res = PoolAllocator_init(&_semaphore_allocator,
                               SEMAPHORE_SIZE,
                               DSOS_MAX_SEMAPHORES,
                               _semaphore_buffer,
                               SEMAPHORE_BUFFER_SIZE);
  assert(!res);
}

static Semaphore* Semaphore_alloc_(int id, int value) {
  Semaphore* s = (Semaphore*) PoolAllocator_getBlock(&_semaphore_allocator);
  if (!s) return 0;
  s->list.prev = s->list.next = 0;
  s->id    = id;
  s->count = value;
  List_init(&s->descriptors);
  List_init(&s->waiters);
  return s;
}

int Semaphore_free(Semaphore* s) {
  return PoolAllocator_releaseBlock(&_semaphore_allocator, s);
}

Semaphore* Semaphore_byId(ListHead* head, int id) {
  ListItem* aux = head->first;
  while (aux) {
    Semaphore* s = (Semaphore*) aux;
    if (s->id == id) return s;
    aux = aux->next;
  }
  return 0;
}

void Semaphore_print(Semaphore* s) {
  printf("sem{id=%d, count=%d, desc=%d, waiters=%d}\n",
         s->id, s->count, s->descriptors.size, s->waiters.size);
}

Semaphore* sem_init(int id, int value) {
  if (id < 0 || value < 0) return 0;

  if (Semaphore_byId(&semaphores_list, id)) {
    return 0; // già presente
  }

  Semaphore* s = Semaphore_alloc_(id, value);
  if (!s) return 0;
  List_insert(&semaphores_list, semaphores_list.last, (ListItem*) s);
  return s;
}

int sem_destroy(int id) {
  Semaphore* s = Semaphore_byId(&semaphores_list, id);
  if (!s) return DSOS_EINVAL;

  // controllo se qualcuno lo sta usando
  if (s->descriptors.size > 0 || s->waiters.size > 0) {
    return DSOS_EAGAIN;
  }

  List_detach(&semaphores_list, (ListItem*) s);
  return Semaphore_free(s);
}

static inline int argi(int idx) {
    return running->syscall_args[idx];
}

void internal_semopen(){
  int key = argi(0);
  int initial = argi(1);
  disastrOS_debug("[INTERNAL] semopen key=%d initial=%d (pid=%d)", key, initial, disastrOS_getpid());

  if(key<0 || initial<0){
      running->syscall_retvalue = DSOS_EINVAL;
      return;
  }

  Semaphore* s = Semaphore_byId(&semaphores_list, key);
  if(!s){
      s = sem_init(key, initial);
      if(!s){
          running->syscall_retvalue = DSOS_EAGAIN;
          return;
      }
  }

  // registro il processo
  PCBPtr* opener= PCBPtr_byPID(&s->descriptors, running->pid);
  if(!opener){
      opener=PCBPtr_alloc(running);
      if(!opener){
          running->syscall_retvalue = DSOS_EAGAIN;
          return;
      }
      List_insert(&s->descriptors, s->descriptors.last, (ListItem*) opener);
  }
  running->syscall_retvalue = key;
}

void internal_semclose(){
  int sem_fd = argi(0);
  disastrOS_debug("[INTERNAL] semclose fd=%d (pid=%d)\n",
                  sem_fd, disastrOS_getpid());

  // validazione
  if (sem_fd < 0){
    running->syscall_retvalue = DSOS_EINVAL;
    return;
  }

  Semaphore* s = Semaphore_byId(&semaphores_list, sem_fd);
  if (!s){
    running->syscall_retvalue = DSOS_EINVAL;
    return;
  }

  // il processo deve risultare tra gli "opener"
  PCBPtr* opener = PCBPtr_byPID(&s->descriptors, running->pid);
  if (!opener){
    running->syscall_retvalue = DSOS_EINVAL; // non aperto da questo processo
    return;
  }

  // stacca e libera la registrazione dell'opener
  List_detach(&s->descriptors, (ListItem*) opener);
  PCBPtr_free(opener);

  if (s->descriptors.size == 0 && s->waiters.size == 0){
    (void) sem_destroy(s->id); // può restituire DSOS_EAGAIN/DSOS_EINVAL
  }

  running->syscall_retvalue = 0; // close riuscita
}

void internal_semwait() {
  int sem_fd = running->syscall_args[0];
  disastrOS_debug("[INTERNAL] semwait fd=%d (pid=%d)\n", sem_fd, disastrOS_getpid());

  // validazione argomenti
  if (sem_fd < 0) {
    running->syscall_retvalue = DSOS_EINVAL;
    return;
  }
  // cerca il semaforo
  Semaphore* s = Semaphore_byId(&semaphores_list, sem_fd);
  if (!s) {
    running->syscall_retvalue = DSOS_EINVAL; // semaforo inesistente
    return;
  }
  // il processo chiamante deve risultare tra gli "opener" del semaforo
  PCBPtr* opener = PCBPtr_byPID(&s->descriptors, running->pid);
  if (!opener) {
    running->syscall_retvalue = DSOS_EINVAL; // non ha semopen su questo sem
    return;
  }
  // caso count > 0: decremento
  if (s->count > 0) {
    --(s->count);
    running->syscall_retvalue = 0;
    return;
  }
  // caso count == 0: blocco
  // - metto in coda il processo in FIFO sia nella coda del semaforo (waiters) sia nella waiting_list di sistema
  // - imposto lo stato a Waiting
  // - passo la CPU al prossimo processo pronto (pattern come sleep/wait)
  running->status = Waiting;
  List_insert(&s->waiters, s->waiters.last, (ListItem*) running);     // FIFO sui waiters del semaforo
  List_insert(&waiting_list, waiting_list.last, (ListItem*) running); // processo finisce tra i waiting globali

  // scelgo il prossimo running dalla ready_list (se esiste)
  PCB* next_running = (PCB*) List_detach(&ready_list, ready_list.first);
  running = next_running;
}

void internal_sempost(){
  int sem_fd = argi(0);
  disastrOS_debug("[INTERNAL] sempost fd=%d (pid=%d)\n",
                  sem_fd, disastrOS_getpid());

  
  if (sem_fd < 0) {
    running->syscall_retvalue = DSOS_EINVAL;
    return;
  }

  //cerca il semaforo
  Semaphore* s = Semaphore_byId(&semaphores_list, sem_fd);
  if (!s) {
    running->syscall_retvalue = DSOS_EINVAL;   
    return;
  }

 
  PCBPtr* opener = PCBPtr_byPID(&s->descriptors, running->pid);
  if (!opener) {
    running->syscall_retvalue = DSOS_EINVAL;   // non è un opener
    return;
  }

  //se ci sono processi in attesa sul semaforo, sveglia il primo (FIFO)
  if (s->waiters.first) {
    // stacchiamo il primo PCB dalla coda dei waiters del semaforo
    PCB* pcb_to_wake = (PCB*) List_detach(&s->waiters, s->waiters.first);

    // lo rimuoviamo anche dalla waiting_list globale
    List_detach(&waiting_list, (ListItem*) pcb_to_wake);

    // lo mettiamo in stato Ready e in coda alla ready_list
    pcb_to_wake->status = Ready;
    List_insert(&ready_list, ready_list.last, (ListItem*) pcb_to_wake);

  } else {
    ++(s->count);
  }
  running->syscall_retvalue = 0;
}










