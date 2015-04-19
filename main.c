#define _XOPEN_SOURCE 600
#define SHARED 1

#include <stdio.h>
#include <stdin.h>
#include <stdlib.h>
#include <pthread.h>



sem_t pista, mutex;
int numBikes;
int *bikesPorPista;

pthread_barrier_t barrera; 


void *ciclista(){
  /*
    faz coisas
  */


  pthread_barrier_wait(&barrera);

}

int main(int argc, char*argv[]){
  /* d n [v/u] */

  int d = argv[1];
  int n = argv[2];

  iniciaCorrida(n,d);

  return 0;
}


int iniciaCorrida(int n, int d){
  int result_code;
  int i, r;
  pthread_t threads[n];
  int thread_args[n];

  pthread_barrier_init(&barrera,NULL,n);

  /*Temos zero bikes em cada pista, inicialmente*/
  bikesPorPista = malloc(d*sizeof(int)); 
  for (i = 0; i < d; i++){
      bikesPorPista[i] = 0;
    }

  /* Semaforos inicializados com 4 */
  pista = malloc(n*sizeof(sem_t));
  for(i = 0; i < n; i++){
    sem_init(pista[i], SHARED, 4);
  }


  /* LARGADA */
  for(i = 0; i<n; i++){
      while(bikesPorPista[r=((int)rand()%d)] == 4);
      bikesPorPista[r]++;
      sem_wait(pista[d]);
    }


  for(i = 1; i < n; i++){
    thread_args[i] = i;

    /* Criando n threads */
    if(pthread_create(&threads[i], NULL, ciclista,NULL)) abort();
  }

}
