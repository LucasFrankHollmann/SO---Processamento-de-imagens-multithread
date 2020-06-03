#include <iostream>			//cin cout
#include <cstdio>			//f
#include <cstdlib>			//
#include <cstring>			//MacGyver
#include <bits/stdc++.h>	//
#include <pthread.h>		//threads
#include <sys/time.h>		//
#include <semaphore.h>		//semaphores

using namespace std;


#define N_THREADS 1
#define MAX_THREADS 8

#define OUT_FILENAME "output.tif"

//

FILE *f_input, *f_output;
unsigned char *vet;
int global_red, global_green, global_blue, global_toler;
int pAlt, num_threads = 0;
int v_index[MAX_THREADS+1];
double WallTime;
clock_t TEMPO;
sem_t semid;

struct timespec start, finish;
double elapsed;

void* Alpha(void* id){
	unsigned char alpha; 
	int alphar, alphag, alphab;
	int tempr, tempg, tempb;
	for (int j = v_index[(long)id]; j < v_index[(long)id+1];j+=4)
	{
		tempr = (int)vet[j];
		tempg = (int)vet[j+1];
		tempb = (int)vet[j+2];

		alphar = abs(global_red-tempr);
		alphag = abs(global_green-tempg);
		alphab = abs(global_blue-tempb);

		//printf("%d ", alphar);	

		//TODO remove defined values
		if((alphar  < global_toler*4 ) && (alphag  < global_toler*4 ) && (alphab  < global_toler*4 ) )
		{
			alpha = (alphar+alphag+alphab)/3;	
			vet[j+3]=alpha;
			
			sem_wait(&semid);
			pAlt++; // SEÇÃO CRITICA
			sem_post(&semid);
		}
	}
	return NULL;
}

void split(int tam, int n){
	int a = tam/n;
	v_index[0]=8;
	for(int j=1;j<n;j++)
	{
		v_index[j]=a*j + (4-((a*j)%4));
		//printf("%d\n", v_index[j]);
	}
	v_index[n]=tam;
}

int main(int argc, char const *argv[]){
	printf("Trabalho de SO 1 - v0.5 rc1\n\n");

	sem_init(&semid, 0, 1);
	pAlt = 0;

	int tam;
	char in_filename[256];

	pthread_t *t;

	if(argc > 1){
		sprintf(in_filename,"%s",argv[1]);
		f_input = fopen(argv[1],"r");
		if(!f_input){
			printf("Erro ao tentar abrir o arquivo.\n");
			exit(EXIT_FAILURE);
		}
	}else{
		printf("Digite o nome do arquivo:\n");
		scanf("%[^\n ]s",in_filename);
		f_input = fopen(in_filename,"r");
		while(!f_input){
			printf("Erro ao tentar abrir o arquivo\n");
			
			printf("Digite o nome do arquivo:\n");
			getchar();
			scanf("%[^\n]s",in_filename);
			f_input = fopen(in_filename,"r");
		}
	}

	//Interação com usuario | testes
	printf("Cor em RGB (0-255):\n");
	printf("Vermelho (Red): ");
	scanf("%d",&global_red);
	while(global_red < 0 || global_red > 255){
		printf("Entrada invalida.\n");
		printf("Vermelho (Red): ");
		scanf("%d",&global_red);
	}
	printf("Verde (Green): ");
	scanf("%d",&global_green);
	while(global_green < 0 || global_green > 255){
		printf("Entrada invalida.\n");
		printf("Verde (Green): ");
		scanf("%d",&global_green);
	}
	printf("Azul (Blue): ");
	scanf("%d",&global_blue);
	while(global_blue < 0 || global_blue > 255){
		printf("Entrada invalida.\n");
		printf("Azul (Blue): ");
		scanf("%d",&global_blue);
	}
	printf("Tolerancia (0-128): ");
	scanf("%d", &global_toler);
	while(global_toler < 0 || global_toler > 128){
		printf("Entrada invalida.\n");
		printf("Tolerancia (0-128): ");
		scanf("%d", &global_toler);
	}
	
	printf("Número de threads (1-8): ");
	scanf("%d", &num_threads);
	while(num_threads < 1 || num_threads > 8){
		printf("Entrada invalida.\n");
		printf("Número de threads (1-8): ");
		scanf("%d", &num_threads);
	}

	t = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);

	f_output = fopen(OUT_FILENAME, "w+");
	//dont ask, pls
	char buffer[300];
	sprintf(buffer,"cp %s %s",in_filename,OUT_FILENAME);
	system(buffer);


	fseek(f_input,4,SEEK_SET);
	fread(&tam, sizeof(int), 1, f_input);
	printf("Tamanho %d\n", tam);
	split(tam,num_threads);

	vet = (unsigned char *)malloc(sizeof(unsigned char)*tam);

	fseek(f_input,0,SEEK_SET);
	fread(vet,1,tam,f_input);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	clock_gettime(CLOCK_MONOTONIC, &start);

	for(long int j = 0; j<num_threads;j++)
	{
		pthread_create(&t[j],NULL,Alpha, (void*)j);
	}

	for(int j = 0; j<num_threads;j++)
	{
		pthread_join(t[j],NULL);
	}

	clock_gettime(CLOCK_MONOTONIC, &finish);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	sem_destroy(&semid);

	fseek(f_output,0,SEEK_SET);
	fwrite(vet,1,tam,f_output);

	fclose(f_input);
	fclose(f_output);

	printf("Tempo de execução: %lf s\n", elapsed);
	printf("Pixels alterados: %d\n\n", pAlt);

	return 0;
}
