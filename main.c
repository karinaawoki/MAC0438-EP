#define _XOPEN_SOURCE 600
#define SHARED 1
#define DEBUG  0
#define SEED   154

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>


/* VARIÁVEIS GLOBAIS */
int numBikes; /*Numero de bikes ainda na corrida */
int *bikesPorPista; /* vetor que guarda quantas bikes podem ocupar dada posição na pista */
int *posicaoBike; /* diz a posição na pista da bike i */
int *voltaBike; /* diz a volta em que a bike i se encontra */
int *chegada; /* usado para montar a chegada */
int tempo; /* variavel usada para mover as bikes */
int *estaNaMetade; /* indica se a bike moveu meia posição */
int mudou = 0; /* Usado para poder mexer em apenas uma thread */
int desclassificados; /* numero de bikes desclassificadas na volta atual */
int *parados; /* */
int *livres;  /* */
int *esperando; /* */
int contadorEsperas;
int **ultimosChegada; /* marcas os ultimos de cada chegada*/
int aleatorio = 0; /* indica se está no modo u ou v */
int d, n;



pthread_t *threads;
pthread_barrier_t barreira,barreira2; 

sem_t *pista, mutex, mutex1, mutex2, mutex3, mutex4, mutex5, mutex6;


/* FUNÇÕES */
int iniciaCorrida(int n, int d);
void *ciclista(void *i);
int chanceQuebra(int numBike);
void mataProcesso(int num);
int compare(const void* a, const void* b);
/************************/



int main(int argc, char*argv[])
{
  /* d n [v/u] */

  if(argc != 4)
    {
      printf("Erro! Entrada esperada: ./programa d n [u/v]\n");
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
  int desclassificada1 = 0; /*indica se a thread vigente foi desclassificada nessa volta */
  int numBikesAntes;
  int num = *((int *) i);
  int velocidade = 1; /* indica se ele vai ou não andar */
  int espera; /* quem esta na espera */
  int j;
  int alternaBarreira = 1;
  int alteraAlternaBarreira = 1;

  while(numBikes>1)
    {
      /* Roda cada tempo */
      if(aleatorio == 1)
        {
	  if(tempo == 0)
            {
	      velocidade = 0;
            }
	  else
            {
	      velocidade = ((int)rand())%2;
            }
	  if(velocidade==0 && estaNaMetade[num] == 1)
            {
	      estaNaMetade[num] = 0;
	      velocidade = 1;
            }
	  else if(velocidade == 0 && estaNaMetade[num] == 0)
            {
	      estaNaMetade[num] = 1;
	      sem_wait(&mutex6);
	      parados[posicaoBike[num]]++;
	      livres[posicaoBike[num]]--;
	      sem_post(&mutex6);
	      espera = 0;
            }
	  if (velocidade==1)
            {
	      sem_wait(&mutex6);
	      esperando[posicaoBike[num]]++;
	      livres[posicaoBike[num]]--;
	      contadorEsperas++;
	      sem_post(&mutex6);
	      espera = 1;
            }
            

	  pthread_barrier_wait(&barreira);

	  while(contadorEsperas>0)
            {
              /* Se tem alguem esperando */
	      if(espera)
                {
		  sem_wait(&mutex6);
		  if(livres[(posicaoBike[num]+1)%d] > 0)
		    {
		      /* Tem uma posição da frente livre */
		      esperando[posicaoBike[num]]--;
		      livres[posicaoBike[num]]++;
		      livres[(posicaoBike[num]+1)%d]--;
		      parados[(posicaoBike[num]+1)%d]++;
		      espera = 0;
		    }
		  else if(parados[(posicaoBike[num]+1)%d]==4)
		    {
          /* Tudo parado em frente */
		      esperando[posicaoBike[num]]--;
		      parados[posicaoBike[num]]++;
		      espera = 0;
		      velocidade = 0;
		      estaNaMetade[num] = 1;
		    }
		  else
		    {
		      alteraAlternaBarreira = 0;
		    }
		  sem_post(&mutex6);    
                } 
	      if(alternaBarreira == 1)
                {
		  pthread_barrier_wait(&barreira2);
                }
	      else
                {
		  pthread_barrier_wait(&barreira);
                }
	      if(alteraAlternaBarreira == 1)
                {
		  sem_wait(&mutex6);
		  contadorEsperas--;
		  sem_post(&mutex6);
                }
	      alteraAlternaBarreira = 1;
	      if(alternaBarreira == 1)
                {
		  pthread_barrier_wait(&barreira2);
                }
	      else
                {
		  pthread_barrier_wait(&barreira);
                }
	      alternaBarreira = (alternaBarreira+1)%2;

            }
            /* Sincroniza barreira */
	  if(alternaBarreira==1)
            {
	      pthread_barrier_wait(&barreira2);
            }
        }

      /* Chance de quebrar */
      if(chanceQuebra(num) && numBikes>3)
        {
	  printf("O ciclista %d quebrou! :(   volta: %d   posição: %d\n", num, voltaBike[num], posicaoBike[num]);
	  mataProcesso(num);
	  desclassificada1=1;   
        }

      if(velocidade == 1)
        {
	  /* Atualiza posição */
	  sem_post(&pista[posicaoBike[num]]);

	  sem_wait(&mutex);
	    
	  bikesPorPista[posicaoBike[num]]--;
	  posicaoBike[num] = (posicaoBike[num]+1)%d;
	  bikesPorPista[posicaoBike[num]]++;
	    
	  sem_post(&mutex);


	  if(posicaoBike[num]==0)
            {
	      voltaBike[num]++;
		
	      sem_wait(&mutex1);
		
	      chegada[voltaBike[num]-1]++;
	      /* A volta começa no 1 */
	      if(chegada[voltaBike[num]-1] == numBikes)
		{
		  mataProcesso(num);
		  desclassificada1=1;
		  ultimosChegada[voltaBike[num]-1][2] = num;
		}
	      else if(chegada[voltaBike[num]-1] == numBikes-1)
		ultimosChegada[voltaBike[num]-1][1] = num;
	      else if(chegada[voltaBike[num]-1] == numBikes-2)
		ultimosChegada[voltaBike[num]-1][0] = num;

	      sem_post(&mutex1);
            }
	  sem_wait(&pista[posicaoBike[num]]);
        }

      if(tempo%200==0 && DEBUG)
        {
	  printf("ciclista %d:  volta %d  --  posição %d \n", num, voltaBike[num], posicaoBike[num]);
        }

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
	  numBikes-=desclassificados;
	  pthread_barrier_init(&barreira, NULL, numBikes);
	  desclassificados = 0;
	  for (j = 0; j<d; j++)
	    {
	      esperando[j] = 0;
	      livres[j] = 4;
	      parados[j] = 0;
	    }
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
      else mudou++;
      
      sem_post(&mutex3);


      if(desclassificada1) pthread_exit(NULL);

      while(mudou!=0);
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
  contadorEsperas = 0;
  chegada = malloc(n*sizeof(int));
  voltaBike = malloc(n*sizeof(int));
  estaNaMetade = malloc(n*sizeof(int));
  thread_args = malloc(n*sizeof(int));
  posicaoBike = malloc(n*sizeof(int));
  parados = malloc(d*sizeof(int));
  esperando = malloc(d*sizeof(int));
  livres = malloc(d*sizeof(int));
  ultimosChegada = malloc(n*sizeof(int*));

  threads = malloc(n*sizeof(pthread_t));
    
  bikesPorPista = malloc(d*sizeof(int)); 
  pista = malloc(d*sizeof(sem_t));

  sem_init(&mutex , SHARED, 1);
  sem_init(&mutex1, SHARED, 1);
  sem_init(&mutex2, SHARED, 1);
  sem_init(&mutex3, SHARED, 1);
  sem_init(&mutex4, SHARED, 1);
  sem_init(&mutex5, SHARED, 1);
  sem_init(&mutex6, SHARED, 1);

  for(i = 0; i < d; i++)
    {
      /*Temos zero bikes em cada pista, inicialmente*/
      bikesPorPista[i] = 4;
      esperando[i] = 0;
      livres[i] = 4;
      parados[i] = 0;

      /* Semaforos inicializados com 4 */
      sem_init(&pista[i], SHARED, 4);
    }

  /* LARGADA */   
  srand(SEED);
  for(i = 0; i<n; i++)
    {
      ultimosChegada[i] = malloc(3*sizeof(int));
      ultimosChegada[i][0] = -1;
      ultimosChegada[i][1] = -1;
      ultimosChegada[i][2] = -1;

      /* Todas as bikes estão rodando inicialmente */
      voltaBike[i] = 0;
      chegada[i] = 0;
      thread_args[i] = i;
      estaNaMetade[i] = 0;

      r=((int)rand()%n);
      while(bikesPorPista[r] !=4) r = (r+1)%n;
      bikesPorPista[r]--;
      posicaoBike[i] = r;
      sem_wait(&pista[r]);
    }

  for(i = 0; i < n; i++)
    /* Criando n threads */
    if(pthread_create(&threads[i], NULL, ciclista, (void *) &thread_args[i])) abort();
    
  for (i = 0; i < n; i++)
    pthread_join(threads[i], NULL);




  printf("\n\n IMPRIMINDO ÚLTIMOS COLOCADOS DE CADA RODADA\n");
  for(i = 0; i<n; i++)
    {
      if(ultimosChegada[i][2] != -1)
        {
	  printf("\n%dª VOLTA\n", i+1);
	  printf("Último colocado: %d\n", ultimosChegada[i][2]);
	  printf("Penúltimo colocado: %d\n", ultimosChegada[i][1]);
	  printf("Antepenúltimo colocado: %d\n", ultimosChegada[i][0]);
        }
    }


  printf("\n\n\n\n\n CLASSIFICAÇÃO:\n");

  qsort(thread_args,n,sizeof(int),compare);
    
  for(i = 0; i<n; i++)
    printf("%dº Colocado: %d\n", i+1, thread_args[n-i-1]);
    

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
  sem_wait(&mutex5);
  desclassificados++;
  printf("morreu o %d\n",num);
  bikesPorPista[posicaoBike[num]]++;
  sem_post(&mutex5);
  sem_post(&pista[posicaoBike[num]]);
}

int compare(const void* a, const void* b){
  int A = voltaBike[*(int*)a];
  int B = voltaBike[*(int*)b];

  if(A > B) return 1;
  if(A < B) return -1;
  if(A == B && posicaoBike[*(int*) a] > posicaoBike[*(int*) b]) return 1;
  return -1;
}
