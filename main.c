#define _XOPEN_SOURCE 600
#define SHARED 1
#define DEBUG  1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/*sem_wait = p
  sem_post = v  */





/* VARIÁVEIS GLOBAIS */
int numBikes;
int *bikeDesclassificada;
int *bikesPorPista;
int *posicaoBike;
int *voltaBike;
int tempo;
int d, n;
int mudou = 0;

pthread_t *threads;
pthread_barrier_t barrera; 

sem_t *pista, mutex;




/* FUNÇÕES */
int iniciaCorrida(int n, int d);
void *ciclista(void *i);
int chanceQuebra(int numBike);







int main(int argc, char*argv[])
{
  /* d n [v/u] */

  d = atoi(argv[1]);
  n = atoi(argv[2]);

  tempo = 0;

  iniciaCorrida(n,d);

  return 0;
}




void *ciclista(void *i)
{
  int num = *((int *) i);

  while(numBikes>1 && bikeDesclassificada[num] == 0)
  {
    if(tempo%200==0)
    {
      printf("ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);
    }
    /* Chance de quebrar */
    if(chanceQuebra(num))
    {
      /*pthread_kill(threads[num], 0);*/
      sem_wait(&mutex);
      numBikes--;
      sem_post(&mutex);

      sem_post(&pista[posicaoBike[num]]);
      bikeDesclassificada[num] = 1;
      bikesPorPista[posicaoBike[num]]++;
      pthread_exit(NULL);
    }

    pthread_barrier_wait(&barrera);
    
    sem_wait(&mutex);
    if(mudou == 0)
    {
      pthread_barrier_init(&barrera, NULL, numBikes);
      mudou++;
      
    }
    else if (mudou==numBikes)
    {
      mudou=0;
    }
    else
    {
     mudou++;
    }
    sem_post(&mutex);
  }
  
  return NULL;
}





int iniciaCorrida(int n, int d)
{
  int i, r, *thread_args;
  pthread_barrier_init(&barrera,NULL,n);

  bikeDesclassificada = malloc(n*sizeof(int));
  threads = malloc(n*sizeof(pthread_t));
  thread_args = malloc(n*sizeof(int));
  bikesPorPista = malloc(d*sizeof(int)); 
  pista = malloc(n*sizeof(sem_t));

  sem_init(&mutex, SHARED, 1);

  for(i = 0; i < d; i++)
  {
    /*Temos zero bikes em cada pista, inicialmente*/
    bikesPorPista[i] = 4;

    /* Semaforos inicializados com 4 */
    sem_init(&pista[i], SHARED, 4);
  }

  /* LARGADA */
  for(i = 0; i<n; i++)
  {
      /* Todas as bikes estão rodando inicialmente */
      bikeDesclassificada[i] = 0;
      voltaBike[i] = 0;

      while(bikesPorPista[r=((int)rand()%d)] == 0);
      bikesPorPista[r]--;
      posicaoBike[i] = r;
      sem_wait(&pista[d]);
  }
  for(i = 0; i < n; i++)
  {
    int *j = malloc(sizeof(*j));
    thread_args[i] = i;
    *j = i;

    /* Criando n threads */
    /* pthread_create(thread, atrubuto   , função da thread, argumento passado para ciclista ) */
    if(pthread_create(&threads[i], NULL, ciclista, (void *) j)) abort();
  }
  return 0;
}





int chanceQuebra(int numBike)
{
  if((int)rand()%100 == 10)
  {
    if(tempo == 0 || posicaoBike[numBike] != 0)
      return 0;
    else if(voltaBike[numBike]%4 == 0)
        return 1;
    }
  }
  return 0;
}