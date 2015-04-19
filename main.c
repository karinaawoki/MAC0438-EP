#define _XOPEN_SOURCE 600
#define SHARED 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


/* VARIÁVEIS GLOBAIS */
sem_t *pista, mutex;
int numBikes;
int *bikeDesclassificada;
int *bikesPorPista;
int tempo;
pthread_barrier_t barrera; 


/* FUNÇÕES */
int iniciaCorrida(int n, int d);
void *ciclista(void *i);



int main(int argc, char*argv[])
{
  /* d n [v/u] */

  int d = atoi(argv[1]);
  int n = atoi(argv[2]);

  tempo = 0;

  iniciaCorrida(n,d);

  return 0;
}


void *ciclista(void *i)
{
  /*
    faz coisas
  */
  int num = *((int *) i);

  while(numBikes>1 && bikeDesclassificada[num] == 0)
  {
    printf("oiii %d\n", tempo);
    
    //implementar barreira aqui
  }


  pthread_barrier_wait(&barrera);
  return NULL;
}


int iniciaCorrida(int n, int d)
{
  int i, r, *thread_args;
  pthread_t *threads;

  pthread_barrier_init(&barrera,NULL,n);

  bikeDesclassificada = malloc(n*sizeof(int));
  threads = malloc(n*sizeof(pthread_t));
  thread_args = malloc(n*sizeof(int));
  bikesPorPista = malloc(d*sizeof(int)); 
  pista = malloc(n*sizeof(sem_t));

  for(i = 0; i < d; i++)
  {
    /*Temos zero bikes em cada pista, inicialmente*/
    bikesPorPista[i] = 0;

    /* Semaforos inicializados com 4 */
    sem_init(&pista[i], SHARED, 4);
  }


  /* LARGADA */
  for(i = 0; i<n; i++)
  {
      /* Todas as bikes estão rodando inicialmente */
      bikeDesclassificada[i] = 0;

      while(bikesPorPista[r=((int)rand()%d)] == 4);
      bikesPorPista[r]++;
      sem_wait(&pista[d]);
  }


  for(i = 1; i < n; i++)
  {
    int *ii = malloc(sizeof(*ii));
    thread_args[i] = i;
    

    /* Criando n threads */
    /* pthread_create(thread, atrubuto   , função da thread, argumento passado para ciclista ) */
    if(pthread_create(&threads[i], NULL, ciclista, (void *) ii)) abort();
  }
  return 0;
}
