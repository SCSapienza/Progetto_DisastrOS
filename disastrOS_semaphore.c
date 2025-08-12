#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_globals.h"
#include "disastrOS_semaphore.h"
#include "pool_allocator.h"
#include "assert.h"

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
  // validazione argomenti
  if (id < 0 || value < 0) return 0; 

  // unicità
  if (Semaphore_byId(&semaphores_list, id)) {
    return 0; // già esiste: il chiamante mapperà a DSOS_EAGAIN
  }

  // alloc + inserimento in lista globale
  Semaphore* s = Semaphore_alloc_(id, value);
  if (!s) return 0;
  List_insert(&semaphores_list, semaphores_list.last, (ListItem*) s);
  return s;
}

int sem_destroy(int id) {
  //cerca
  Semaphore* s = Semaphore_byId(&semaphores_list, id);
  if (!s) return DSOS_EINVAL; // non esiste

  // controlla che non sia “in uso”
  if (s->descriptors.size > 0 || s->waiters.size > 0) {
    return DSOS_EAGAIN; // occupato (processi con fd aperto o in coda)
  }

  // stacca dalla lista globale e free
  List_detach(&semaphores_list, (ListItem*) s);
  return Semaphore_free(s);
}

static inline int argi(int idx) { //funzione helper
    return running->syscall_args[idx];
}

void internal_semopen(){

int key = argi(0);
int initial = argi(1);
disastrOS_debug("[INTERNAL] semopen key=%d initial=%d (pid=%d)", key, initial, disastrOS_getpid());
(void)key; (void)initial;// evitare warning non usato
 running->syscall_retvalue = DSOS_EUNIMPL; //codice errore non implementato 
}

  
  

void internal_semclose(){
int sem_fd = argi(0);
disastrOS_debug("[INTERNAL] semclose fd=%d (pid=%d)", sem_fd, disastrOS_getpid());
(void)sem_fd;
running->syscall_retvalue = DSOS_EUNIMPL;
}

void internal_semwait(){
int sem_fd = argi(0);
disastrOS_debug("[INTERNAL] semwait fd=%d (pid=%d)", sem_fd, disastrOS_getpid());
(void)sem_fd;
running->syscall_retvalue = DSOS_EUNIMPL; //codice errore non implementato
}

  
  

void internal_sempost(){
int sem_fd = argi(0);
disastrOS_debug("[INTERNAL] sempost fd=%d (pid=%d)", sem_fd, disastrOS_getpid());
(void)sem_fd;
running->syscall_retvalue = DSOS_EUNIMPL;
}










