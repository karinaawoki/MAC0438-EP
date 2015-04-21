#define _XOPEN_SOURCE 600
#define SHARED 1
#define DEBUG  1
#define SEED   155

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int desclassificados; /* Variavel que conta o numero de desclassificados totais por volta */

/*Pra parte 2*/
int *velocidadeBike; /*Marca 1 se o ciclista i está a 50 km/h e 0 caso esteja a 25 km/h */
int *meiaPosicao; /*Marca se o ciclista i está "ocupando" meia posição a frente da sua posicaoBike */
int aleatorio = 0;
/**************/

pthread_t *threads;
pthread_barrier_t barreira,barreira2; 

sem_t *pista, mutex, mutex1, mutex2, mutex3, mutex4, mutex5;


/* FUNÇÕES */
int iniciaCorrida(int n, int d);
void *ciclista(void *i);
int chanceQuebra(int numBike);
void mataProcesso(int num);
/**************/


int main(int argc, char*argv[])
{
  /* d n [v/u] */
  if(argc != 4){
    printf("Erro! Entrada esperada: ./programa d n u/v\n");
    return 1;
  }
  
  d = atoi(argv[1]);
  n = atoi(argv[2]);

  if(!strcmp(argv[3],"v")) aleatorio = 1;

  tempo = 0;


  iniciaCorrida(n,d);

  return 0;
}


void *ciclista(void *i)
{
  int desc = 0; /*diz se o ciclista i foi desclassificado nessa volta */
  int numBikesAntes;
  int num = *((int *) i);
  while(numBikes>1)
    {
      if(aleatorio == 1 && voltaBike[num] > 1){ puts("aff"); velocidadeBike[num] = rand()%2;}

      if(tempo%200==0 && DEBUG)
        {
	  printf("ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);
        }

      
      /* Chance de quebrar */
      if(chanceQuebra(num))
        {
	  sem_wait(&mutex5);
	  desclassificados++;
	  mataProcesso(num);
	  sem_post(&mutex5);
	  desc=1;   
        }
      
      if(velocidadeBike[num] == 1)
	{
	  sem_post(&pista[posicaoBike[num]]);

	  sem_wait(&mutex);
	  bikesPorPista[posicaoBike[num]]--;
	  posicaoBike[num] = (posicaoBike[num]+1)%d;
	  bikesPorPista[posicaoBike[num]]++;
	  sem_post(&mutex);
	  puts("oi");
	}
      else
	{
	  puts("xau");
	  if(meiaPosicao[num] == 0) meiaPosicao[num] = 1;
	  else if(meiaPosicao[num] == 1)
	    {
	      sem_post(&pista[posicaoBike[num]]);

	      sem_wait(&mutex);
	      bikesPorPista[posicaoBike[num]]--;
	      posicaoBike[num] = (posicaoBike[num]+1)%d;
	      bikesPorPista[posicaoBike[num]]++;
	      sem_post(&mutex);

	      meiaPosicao[num] = 0;
	    }
	}
      

      if(posicaoBike[num]==0 && desc == 0)
        {
	  voltaBike[num]++;

	  sem_wait(&mutex1);
	  chegada[voltaBike[num]-1]++;
	  /* A volta começa no 1 */
	  if(chegada[voltaBike[num]-1] == numBikes)
	    {
	      sem_wait(&mutex5);
	      mataProcesso(num);
	      desclassificados++;
	      sem_post(&mutex5);
	      desc=1;
	    }
	  sem_post(&mutex1);
        }
      sem_wait(&pista[posicaoBike[num]]);

      /*------------------------------------
	BARREIRA 1 --------------------------
        --------------------------------------
        -------------------------------------*/
      pthread_barrier_wait(&barreira);

      sem_wait(&mutex2);  
      if(mudou == numBikes-1)
	{
	  pthread_barrier_destroy(&barreira);
                
	  tempo++;
	  mudou=0;
	  numBikesAntes = numBikes;
	  numBikes -= desclassificados;
	  pthread_barrier_init(&barreira, NULL, numBikes);
	  desclassificados = 0;
	}
      else 
	{
	  numBikesAntes = numBikes;
	  mudou++;
	}
      sem_post(&mutex2);

      /*------------------------------------------
	BARREIRA 2 
        ---------------------------------------------
        --------------------------------------------*/
      pthread_barrier_wait(&barreira2);
      sem_wait(&mutex3);
      if(mudou == numBikesAntes-1)
	{
	  pthread_barrier_destroy(&barreira2);
	  pthread_barrier_init(&barreira2, NULL, numBikes);
	  mudou=0;
	}
      else
	{
	  mudou++;
	}
      sem_post(&mutex3);


      if(desc)
	pthread_exit(NULL);
      printf("passou %d\n", num);

      while(mudou!=0) /*printf("tamo aqui\n")*/ ;
      puts("");
    }
  return NULL;
}





int iniciaCorrida(int n, int d)
{
  int i, r;
  int *thread_args;
  pthread_barrier_init(&barreira,NULL,n);
  pthread_barrier_init(&barreira2,NULL,n);

  numBikes = n;

  bikeDesclassificada = malloc(n*sizeof(int));
  chegada = malloc(n*sizeof(int));
  voltaBike = malloc(n*sizeof(int));
  thread_args = malloc(n*sizeof(int));
  posicaoBike = malloc(n*sizeof(int));
  velocidadeBike = malloc(n*sizeof(int));
  meiaPosicao = malloc(n*sizeof(int));

  threads = malloc(n*sizeof(pthread_t));
    
  bikesPorPista = malloc(d*sizeof(int)); 
  pista = malloc(d*sizeof(sem_t));

  sem_init(&mutex, SHARED, 1);
  sem_init(&mutex1, SHARED, 1);
  sem_init(&mutex2, SHARED, 1);
  sem_init(&mutex3, SHARED, 1);
  sem_init(&mutex4, SHARED, 1);
  sem_init(&mutex5, SHARED, 1);

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
      if(aleatorio) velocidadeBike[i] = 0;
      else velocidadeBike[i] = 1;
      meiaPosicao[i] = 0;
      thread_args[i] = i;

      while(bikesPorPista[r=((int)rand()%d)] != 4); /*checa se já existe algum ciclista naquela posição */
      bikesPorPista[r]--;
      posicaoBike[i] = r;
      sem_wait(&pista[r]);

      printf("posição inicial %d : %d\n",i,r);
    }

  for(i = 0; i < n; i++)   
    /* Criando n threads */
    if(pthread_create(&threads[i], NULL, ciclista, (void *) &thread_args[i])) abort();
        
    
  
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
  bikesPorPista[posicaoBike[num]]++;
  sem_post(&pista[posicaoBike[num]]);
}
