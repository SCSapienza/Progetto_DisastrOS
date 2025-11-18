#ifndef DISASTROS_SEMAPHORE_H
#define DISASTROS_SEMAPHORE_H

#include "linked_list.h"  
#include "disastrOS_globals.h" 

// codici errore
#define DSOS_EUNIMPL  (-501) // non implementata
#define DSOS_EINVAL   (-502) // argomenti non validi
#define DSOS_EAGAIN   (-503) // risorsa non disponibile

typedef struct Semaphore {
    ListItem list;
    int id;
    int count;
    ListHead descriptors;    
    ListHead waiters;
} Semaphore;

extern ListHead semaphores_list;

void Semaphore_init(void);
int Semaphore_free(Semaphore* s);

Semaphore*  sem_init(int id, int value);
int         sem_destroy(int id);

Semaphore* Semaphore_byId(ListHead* head, int id);
void       Semaphore_print(Semaphore* s);

// syscall handlers
void internal_semopen();
void internal_semclose();
void internal_semwait();
void internal_sempost();

#endif 