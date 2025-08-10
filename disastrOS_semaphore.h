#ifndef DISASTROS_SEMAPHORE_H
#define DISASTROS_SEMAPHORE_H

#include "linked_list.h"  
#include "disastrOS_globals.h" 

/* Errori base per gli stub */
#define DSOS_EUNIMPL  (-501) /* funzione non implementata */
#define DSOS_EINVAL   (-502) /* argomenti non validi*/
#define DSOS_EAGAIN   (-503) /* risorsa non trovata/creabile */

typedef struct Semaphore {
    int rid;
    int count;
    ListHead descriptors;    
    ListHead waiters;
    ListItem list;
} Semaphore;

/* Prototipi internal_ (kernal side) - stub */
void internal_semopen();
void internal_semclose();
void internal_semwait();
void internal_sempost();

#endif 