#include <stdio.h>
#include <stdin.h>
#include <stdlib.h>
#include <pthread.h>

DEFINE SHARED 1
void *ciclista()
{

}

int main(int argc, char*argv[])
{
	/* d n [v/u] */

	int d = argv[1];
	int n = argv[2];

	return 0;
}


int corrida(int n)
{
   	pthread_t threads[n];
   	int thread_args[n];
   	int result_code, i;
 	sem_t *pista;

	pista = malloc(n*sizeof(sem_t));
	/* Semaforos inicializados com 4 */
    for(i = 0; i < n; i++){
    	sem_init(pista[i],SHARED,4);
    }

   	/* create all threads one by one */
   	for (i = 1; i < n; i++) {

    	/*Passa para cada thread o seu respectivo número -- ACHO??? */
    	thread_args[i] = i;

    	/* Criand threads */
      	result_code = pthread_create(&threads[i], NULL, ciclista, (void *) &thread_args[i]);
      	
      	if (result_code != 0)
      		abort();
   	}

}