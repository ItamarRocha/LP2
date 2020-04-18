#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 10
#define NUM_ARCHIVES 10
#define NUM_LINES 10

pthread_barrier_t barrier;
pthread_t threads[NUM_THREADS];

int partial_sum[10] = {};
int sum = 0; 
int mode[10] = {};

char dir[20];

void* soma(void* i){
	long p = (long) i;
	int pp = (int) p;
	int partial_mode[10] = {};
	int number;
	
	

	char group;
	group = (char)97 + pp;
	
	char archive_name[30];
	for(int j = 0; j < NUM_ARCHIVES; j++){
		
		sprintf(archive_name,"%s/%c%d.txt",dir,group,j);
		
		FILE *archive = fopen(archive_name,"r");
		for(int k = 0; k < 2*NUM_LINES ; k++){
			
			if( k % 2 == 0 ){
				number = fgetc(archive) - 48;
				partial_sum[(int)group - 97] += number;

				partial_mode[number] += 1;
			}else
				fgetc(archive);

		}
		fclose(archive);
	}

	int maior = 0;
	int current = 0;

	for(int j = 0; j < NUM_ARCHIVES; j++){
		if(partial_mode[j] > maior){
			maior = partial_mode[j];
			current = j;
		}
	} 
	
	mode[pp] = current; 

	pthread_barrier_wait(&barrier);
	pthread_exit(NULL);
}

int main(int argc, char** argv){
	
	if(argc == 2){
		sprintf(dir,"%s",argv[1]);
	}else{
		printf("wrong pattern, the right pattern is : ./exec [file_dir/]\n\n");
		return -1;
	}

	int i = 0;
	int rc;
	
	pthread_t threads[NUM_THREADS];

	pthread_barrier_init(&barrier,NULL,NUM_THREADS + 1);
	for(i = 0 ; i < NUM_THREADS; i++){
		rc = pthread_create(&threads[i], NULL, soma,(void*)i );
	}

	pthread_barrier_wait(&barrier);

	for(int j = 0; j < 10 ; j++){
		printf("GRUPO %c SOMA %d MÉDIA %.4f MODA %d\n",(char) 97 + j,partial_sum[j],(float)partial_sum[j]/(NUM_ARCHIVES*NUM_LINES), mode[j]);
		sum += partial_sum[j];

	}
	int maior = 0;
	int current = 0;
	int mode_number[10] = {};
	for(int j = 0; j < NUM_THREADS; j++){ //aqui ele vê qual dos numeros de moda das respectivas threads está aparecendo mais e adiciona ao respectivo espaço
		mode_number[mode[j]] += 1;		  //isto é : se a thread 1 teve como número maior o 9, somamos um ao local nove do mode_number	
	} 
	for(int j = 0; j < NUM_THREADS; j++){  //aqui é visto qual número é o maior no array(logo oq mais aparecia no que seria a moda das threads, assim, verificamos a moda total;
		if(mode_number[j] > maior){
			maior = mode_number[j];
			current = j;
		}
	} 

	printf("SOMA TOTAL %d MEDIA TOTAL %.4f MODA TOTAL %d\n",sum,(float)sum/(NUM_LINES*NUM_ARCHIVES*NUM_THREADS), current);




	return 0;
}
