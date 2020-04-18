#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


int currentProcess = 0;

void randomThread(void* randomNumbers){
	char* filename = malloc(sizeof(char*)*100);	
	sprintf(filename,"resultado%d.txt",currentProcess++);
	
	FILE* archive = fopen(filename,"w");

	int i = 0;
	int variavel;
	for(i=0 ; i <randomNumbers; i++){
		variavel = rand();
		fprintf(archive,"%d\n",variavel);
	}
	fprintf(archive,"\n");

	fclose(archive);
	free(filename);
	
}


int main(int argc, char* argv[]){
	if(argc < 3){
		printf("pattern differs\nPattern = Number-of-process quant-randmon-number type(t or f)\n");
		return 0;
	}
	printf("%s\n",argv[3] );

	int numberOfProcesses = atoi(argv[1]);
	int randomNumbers = atoi(argv[2]);
	char* type = argv[3];

	int i = 0;
	int rc;
	
	pthread_t threads[numberOfProcesses];
	if(!strcmp(type,"t")){
		for(i = 0; i < numberOfProcesses; i++){
			rc = pthread_create(&threads[i], NULL, randomThread,(void*)randomNumbers);
			if(rc){
				printf("Error\n");
			}
		}
	}else if(!strcmp(type,"f")){
		for(i = 0; i < numberOfProcesses; i++){
			rc = fork();
			if(rc ==0)
				break;
			randomThread(randomNumbers);
		}	
	}else{
		printf("Invalid Type\n");
		return(-1);
	}

	sleep(2);

	return 0;
}
