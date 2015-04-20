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
int mudou = 0; /* Ver se alguem já mudou o tamanho da barreira */

pthread_t threads[50];
pthread_barrier_t barrera; 
pthread_barrier_t barrera2; 

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
  int morreu = 0;
  int num = *((int *) i);
  printf("oi ciclista %d -- %d -- %d\n", num,numBikes, bikeDesclassificada[num]);
  while(numBikes>1 && bikeDesclassificada[num] == 0)
  {
    printf("passo ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);

    /*if(tempo%200==0)
    {
      printf("ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);
    }*/
    printf("\n\n\n");
    /* Chance de quebrar */
    if(chanceQuebra(num))
    {
      /*pthread_kill(threads[num], 0);*/
      printf("morreu o %d\n",num);
      sem_wait(&mutex);
      numBikes--;
      sem_post(&mutex);

      sem_post(&pista[posicaoBike[num]]);
      bikeDesclassificada[num] = 1;
      bikesPorPista[posicaoBike[num]]++;
      morreu = 1;
    }

    sem_post(&pista[posicaoBike[num]]);

    sem_wait(&mutex);
    bikesPorPista[posicaoBike[num]]--;
    posicaoBike[num] = (posicaoBike[num]+1)%d;
    bikesPorPista[posicaoBike[num]]++;
    sem_post(&mutex);
    if(posicaoBike[num]==0)
    {
      voltaBike[num]++;
    }
    sem_wait(&pista[posicaoBike[num]]);

    printf("oieeee barrera %d\n", num);
    pthread_barrier_wait(&barrera);
    printf("BARREIRAAAAA-------------\n");
    

    sem_wait(&mutex);
    if(mudou == numBikes)
    {
      pthread_barrier_destroy(&barrera);
      pthread_barrier_init(&barrera, NULL, numBikes);
      tempo++;
      mudou=0;
    }
    else
    {
      mudou++;
    }
    sem_post(&mutex);

    /* BARREIRA 2 */
    printf("oieeee barrera 2 %d\n", num);
    pthread_barrier_wait(&barrera2);
    printf("BARREIRAAAAA 2-------------\n");

    sem_wait(&mutex);
    if(mudou == numBikes)
    {
      tempo++;
      pthread_barrier_destroy(&barrera2);
      pthread_barrier_init(&barrera2, NULL, numBikes);
      mudou=0;
    }
    else
    {
      mudou++;
    }
    sem_post(&mutex);

    if(morreu)
      pthread_exit(NULL);


    
  }
  return NULL;
}





int iniciaCorrida(int n, int d)
{
  int i, r;
  pthread_barrier_init(&barrera,NULL,n);
  pthread_barrier_init(&barrera2,NULL,n);

  numBikes = n;

  bikeDesclassificada = malloc(n*sizeof(int));
  voltaBike = malloc(n*sizeof(int));
  posicaoBike = malloc(n*sizeof(int));
  /*threads = malloc(n*sizeof(*threads));*/
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
      /*printf("aqui1\n");*/
      sem_wait(&pista[r]);
      /*printf("sfsdf\n");*/
  }
  /*printf("888\n");*/
  for(i = 0; i < n; i++)
  {
    /* Criando n threads */
    /* pthread_create(thread, atrubuto   , função da thread, argumento passado para ciclista ) */

    if(pthread_create(&threads[i], NULL, ciclista, (void *) &i)) 
      {
        abort();
      }
  }
  for (i = 0; i < n; i++)
    pthread_join(threads[i], NULL);

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
  return 0;
}