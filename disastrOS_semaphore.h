#ifndef DISASTROS_SEMAPHORE_H
#define DISASTROS_SEMAPHORE_H

#include "linked_list.h"  
#include "disastrOS_globals.h" 

/* Errori base per gli stub */
#define DSOS_EUNIMPL  (-501) /* funzione non implementata */
#define DSOS_EINVAL   (-502) /* argomenti non validi*/
#define DSOS_EAGAIN   (-503) /* risorsa non trovata/creabile */

typedef struct Semaphore {
    ListItem list;
    int id;
    int count;
    ListHead descriptors;    
    ListHead waiters;
} Semaphore;

extern ListHead semaphores_list; // lista globale dei semafori

void Semaphore_init(void); //inizializza il pool allocator per i semafori
int Semaphore_free(Semaphore* s); //rilascia un blocco

Semaphore*  sem_init(int id, int value); //inizializza un semafori (fallisce se esiste già)
int         sem_destroy(int id); //distrugge se non usato

Semaphore* Semaphore_byId(ListHead* head, int id);
void       Semaphore_print(Semaphore* s); //stampa un semaforo

/* Prototipi internal_ (kernal side) - stub */
void internal_semopen();
void internal_semclose();
void internal_semwait();
void internal_sempost();

#endif 