#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

void printMatrix (int k, float **mat);
void printVector(int k, float *v);
void *compute(void* id);

float		*v1, *v2, **mat, *tmpv, result;
int 		k, counter = 0;

sem_t		*s;

int main(int argc, char *argv[]){
	
	srand(time(NULL));
	
	int 		i, j;
	float 		a = 1.0;
	pthread_t	*threads;
	int			*th_id;
	
	if (argc < 2){
		perror("Not enough arguments\n");
		return 1;
	}else{
		k = atoi(argv[1]);
	}
	v1 = (float*) malloc (k * sizeof(float));
	v2 = (float*) malloc (k * sizeof(float));
	tmpv = (float*) malloc (k * sizeof(float));
	mat = (float**) malloc (k * sizeof(float*));
	
	threads = (pthread_t*) malloc (k * sizeof(pthread_t));
	th_id = (int*) malloc (k * sizeof(int));
	
	s = (sem_t *) malloc(sizeof(sem_t));
	sem_init (s, 0, 1);
	
	
	for (i = 0; i < k; i++){
		mat[i] = (float*) malloc (k * sizeof(float));
	}
	
	for (i = 0; i < k; i++){
		v1[i] = (((float)rand()/(float)(RAND_MAX)) * a) - 0.5;
	}
	
	for (i = 0; i < k; i++){
		v2[i] = (((float)rand()/(float)(RAND_MAX)) * a) - 0.5;
	}
	
	for (i = 0; i < k; i++){
		for (j = 0; j < k; j++){
			mat[i][j] = (((float)rand()/(float)(RAND_MAX)) * a) - 0.5;
		}
	}
	
	printVector(k, v1);
	printf("\n");
	printVector(k, v2);
	printf("\n");
	printMatrix(k, mat);
	printf("\n");
	
	for (i = 0; i < k; i++){
		th_id[i] = i;
		pthread_create(&threads[i], NULL, compute, (void*)th_id[i]);
		
	}
	
	for (i = 0; i < k; i++){
		pthread_join(threads[i], NULL);
	}
	
	printVector(k, tmpv);
	printf("\n");
	printf("Result: %f.\n", result);
	
	sem_destroy(s);
	
	return 0;
}

void *compute(void* i){
	
	int id = (int)i;
	int j;
	
	printf("I am thread %d.\n", id);
	
	for (j = 0; j < k; j++){
		tmpv[id] += (v1[id] * mat[id][j]);
	}
	sem_wait(s);
	counter ++;
	if (counter == k){
		printf("Last thread %d.\n", id);
		for(j = 0; j < k; j++){
			result += (tmpv[j] * v2[j]);
		}
	}
	sem_post(s);
}	
	
	
void printVector(int k, float *v){
	int i;
	for (i = 0; i < k; i++){
		printf("%f\t", v[i]);
	}
	printf("\n");
}

void printMatrix (int k, float **mat){
	int j, i;
	for (j = 0; j < k; j++){
		for(i = 0; i < k; i++){
			printf("%f\t", mat[j][i]);
        }
    printf("\n");
	}
}
