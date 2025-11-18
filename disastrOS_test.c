#include <stdio.h>
#include "disastrOS.h"

void test_semaphores_basic(void) {
  printf("\n=== TEST 1: operazioni base ===\n");

  int sem = disastrOS_semopen(123, 3);
  printf("semopen(123, 3) = %d\n", sem);
  if (sem < 0) {
    printf("ERROR: semopen failed\n");
    return;
  }

  // un paio di wait
  int ret1 = disastrOS_semwait(sem);
  printf("semwait #1 = %d\n", ret1);

  int ret2 = disastrOS_semwait(sem);
  printf("semwait #2 = %d\n", ret2);

  int ret3 = disastrOS_sempost(sem);
  printf("sempost = %d\n", ret3);

  int ret4 = disastrOS_semclose(sem);
  printf("semclose = %d\n", ret4);

  // testo errori
  printf("\ntesto errori:\n");
  int bad1 = disastrOS_semwait(-1);
  printf("semwait(-1) = %d (expected -502)\n", bad1);

  int bad2 = disastrOS_sempost(999);
  printf("sempost(999) = %d (expected -502)\n", bad2);

  printf("TEST 1 ok\n");
}

void test_semaphores_multiple_open(void) {
  printf("\n=== TEST 2: apertura multipla ===\n");

  int sem1 = disastrOS_semopen(200, 5);
  printf("prima apertura: semopen(200, 5) = %d\n", sem1);

  // riapro stesso semaforo
  int sem2 = disastrOS_semopen(200, 10);
  printf("seconda apertura: semopen(200, 10) = %d\n", sem2);

  disastrOS_semwait(sem1);
  printf("wait su primo fd ok\n");

  disastrOS_sempost(sem2);
  printf("post su secondo fd ok\n");

  disastrOS_semclose(sem1);
  printf("close primo fd\n");

  // il semaforo esiste ancora
  disastrOS_semwait(sem2);
  printf("wait su secondo fd ancora funziona\n");

  disastrOS_semclose(sem2);
  printf("close secondo fd (semaforo eliminato)\n");

  printf("TEST 2 ok\n");
}

void test_sem_risorse(void) {
  printf("\n=== TEST 3: consumo risorse ===\n");

  int sem = disastrOS_semopen(300, 2);
  printf("semopen(300, 2) = %d\n", sem);

  // consumo le risorse
  printf("consumo risorsa 1...\n");
  int ret1 = disastrOS_semwait(sem);
  printf("ret = %d\n", ret1);

  printf("consumo risorsa 2...\n");
  int ret2 = disastrOS_semwait(sem);
  printf("ret = %d\n", ret2);

  printf("semaforo vuoto\n");

  printf("rilascio risorsa...\n");
  int ret3 = disastrOS_sempost(sem);
  printf("ret = %d\n", ret3);

  printf("consumo di nuovo...\n");
  int ret4 = disastrOS_semwait(sem);
  printf("ret = %d\n", ret4);

  disastrOS_semclose(sem);
  printf("TEST 3 ok\n");
}

void initFunction(void* args) {
  printf("\nTest semafori - PID: %d\n", disastrOS_getpid());

  test_semaphores_basic();
  test_semaphores_multiple_open();
  test_sem_risorse();

  printf("\ntutti i test completati!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv) {
  char* logfilename = 0;
  if (argc > 1) {
    logfilename = argv[1];
  }
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
