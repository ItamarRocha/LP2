#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

#define NUM_THREADS 2

sem_t consumer;
sem_t producer;

int data = 0; 
int stop = 0;

char buffer;

void* produtor(void* i){

	char abra[] = "abracadabra simsalabin";

	printf("%s\n",abra);
	while(!stop){
		sem_wait(&producer);
		buffer = abra[data];
		data++;
		sem_post(&consumer);
		printf("enviado   [%c]\n",buffer);
		//sleep(1);
	}
	pthread_exit(NULL);
}



void* consumidor(void* i){

	int posi = 0;

	char recebido[30];
	while(!stop){
		sem_wait(&consumer);
		posi = data - 1;
		recebido[posi] = buffer;
		sleep(1);
		printf("recebido  [%c]\n",recebido[posi]);
		printf("string    [%s]\n\n\n\n",recebido );
		if(buffer == '\0')
			stop = 1;
		sem_post(&producer);
	}

	pthread_exit(NULL);
}

int main(int argc, char** argv){

	//sem_t main;

	sem_init(&producer,0,1);
	sem_init(&consumer,0,0);
	//sem_init(&main,0,0);

	pthread_t threads[NUM_THREADS];

	pthread_create(&threads[0], NULL, produtor,(void*)1 );
	pthread_create(&threads[1], NULL, consumidor,(void*)2 );

	pthread_join(threads[0],NULL);
	pthread_join(threads[1],NULL);

	//sem_wait(&main);
	
	return 0;
}
