#define _XOPEN_SOURCE 600
#define SHARED 1
#define DEBUG  0
#define SEED   154

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
int *chegada;
int tempo;
int d, n;
int mudou = 0; /* Ver se alguem já mudou o tamanho da barreira */

pthread_t threads[50];
pthread_barrier_t barrera; 
pthread_barrier_t barrera2; 

sem_t *pista, mutex, mutex1, mutex2, mutex3, mutex4;




/* FUNÇÕES */
int iniciaCorrida(int n, int d);
void *ciclista(void *i);
int chanceQuebra(int numBike);
void mataProcesso(int num);







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
  while(numBikes>1)
  {
    printf("passo ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);

    if(tempo%200==0 && DEBUG)
    {
      printf("ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);
    }

    printf("\n\n\n");
    /* Chance de quebrar */
    if(chanceQuebra(num))
    {
      /*pthread_kill(threads[num], 0);*/
        mataProcesso(num);
        morreu = 1;
    }

    sem_post(&pista[posicaoBike[num]]);

    sem_wait(&mutex);
    bikesPorPista[posicaoBike[num]]--;
    posicaoBike[num] = (posicaoBike[num]+1)%d;
    bikesPorPista[posicaoBike[num]]++;
    sem_post(&mutex);


    if(posicaoBike[num]==0 && morreu == 0)
    {
      voltaBike[num]++;


      sem_wait(&mutex1);
      chegada[voltaBike[num]-1]++;
      /* A volta começa no 1 */
      if(chegada[voltaBike[num]-1] == numBikes)
      {
        printf("morreeeeeeeeeeu---------------------------- %d\n", num);
        morreu = 1;
        mataProcesso(num);
      }
      sem_post(&mutex1);


    }
    sem_wait(&pista[posicaoBike[num]]);

    /* BARREIRA 1 */
    printf("toc toc\n");
    pthread_barrier_wait(&barrera);
    printf("pode entrar\n");

    sem_wait(&mutex2);
    if(mudou == numBikes+morreu)
    {
      pthread_barrier_destroy(&barrera);
      pthread_barrier_init(&barrera, NULL, numBikes);
      tempo++;
      mudou=0;
    }
    else if(mudou>numBikes)
    {
        printf("ERROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
    }
    else 
    {
      mudou++;
    }
    sem_post(&mutex2);

    /* BARREIRA 2 */
    printf("toc toc   2\n");
    pthread_barrier_wait(&barrera2);
    printf("****************************Ovalor é zerto %d\n", mudou);
    printf("numbikes = %d\n", numBikes);
    printf("pode entrar  2\n");
    sem_wait(&mutex3);
    if(mudou == numBikes)
    {
      pthread_barrier_destroy(&barrera2);
      pthread_barrier_init(&barrera2, NULL, numBikes);
      mudou=0;
    }
    else
    {
      mudou++;
    }
    sem_post(&mutex3);

    if(morreu)
      pthread_exit(NULL);


    
  }
  return NULL;
}





int iniciaCorrida(int n, int d)
{
  int i, r;
  int *thread_args;
  pthread_barrier_init(&barrera,NULL,n);
  pthread_barrier_init(&barrera2,NULL,n);

  numBikes = n;

  bikeDesclassificada = malloc(n*sizeof(int));
  chegada = malloc(n*sizeof(int));
  voltaBike = malloc(n*sizeof(int));
  thread_args = malloc(n*sizeof(int));
  posicaoBike = malloc(n*sizeof(int));
  /*threads = malloc(n*sizeof(*threads));*/
  bikesPorPista = malloc(d*sizeof(int)); 
  pista = malloc(d*sizeof(sem_t));

  sem_init(&mutex, SHARED, 1);
  sem_init(&mutex1, SHARED, 1);
  sem_init(&mutex2, SHARED, 1);
  sem_init(&mutex3, SHARED, 1);
  sem_init(&mutex4, SHARED, 1);

  for(i = 0; i < d; i++)
  {
    /*Temos zero bikes em cada pista, inicialmente*/
    bikesPorPista[i] = 4;

    /* Semaforos inicializados com 4 */
    sem_init(&pista[i], SHARED, 4);
  }

  /* LARGADA */
  srand(SEED);
  for(i = 0; i<n; i++)
  {
      /* Todas as bikes estão rodando inicialmente */
      bikeDesclassificada[i] = 0;
      voltaBike[i] = 0;
      chegada[i] = 0;
      thread_args[i] = i;

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

    if(pthread_create(&threads[i], NULL, ciclista, (void *) &thread_args[i])) 
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



void mataProcesso(int num)
{
     printf("morreu o %d\n",num);
      sem_wait(&mutex4);
      numBikes--;
      bikesPorPista[posicaoBike[num]]++;
      sem_post(&mutex4);

      sem_post(&pista[posicaoBike[num]]);
      bikeDesclassificada[num] = 1;
      
}