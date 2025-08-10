#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_globals.h"
#include "disastrOS_semaphore.h"

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










