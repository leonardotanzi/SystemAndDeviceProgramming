#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct{
	int difference;
	int value;
} newtype;

int 		numer, denom, count;
float 		result;
sem_t 		*s1, *s2, *s3, *s4;

void *numerator(void *a);
void *denominator(void *a);

int main(int argc, char* argv[]){

	int 		n, k, diff, i;
	pthread_t	*threads;
	newtype		arg[2];
	
	if (argc < 3) perror("Not enough args.\n");
	else n = atoi(argv[1]); k = atoi(argv[2]);
	
	if (k > n) perror("K must be lower than N.\n");
	
	threads = (pthread_t*) malloc (2 * sizeof(pthread_t));
	s1 = (sem_t *)malloc(sizeof(sem_t));
	s2 = (sem_t *)malloc(sizeof(sem_t));
	s3 = (sem_t *)malloc(sizeof(sem_t));
	s4 = (sem_t *)malloc(sizeof(sem_t));
	diff = n - k;
	numer = 1;
	denom = 1;
	result = 1;
	count = 0;
	sem_init(s1, 0, 1);
	sem_init(s2, 0, 2);
	sem_init(s3, 0, 1);
	sem_init(s4, 0, 1);

	
	arg[0].difference = diff;
	arg[0].value = n;
	pthread_create(&threads[0], NULL, numerator, (void*)&arg[0]);
	
	arg[1].value = k;
	pthread_create(&threads[1], NULL, denominator, (void*)&arg[1]);
	
	
	for(i = 0; i < 2; i++){
		pthread_join(threads[i], NULL);
	}

	printf("Final result is %f.\n", result);
	
	sem_destroy(s1);
	sem_destroy(s2);
	sem_destroy(s3);
	sem_destroy(s4);
	
	return 0;
}


void* numerator(void *a){
	newtype *s = (newtype*) a;
	int first_factor, second_factor, local_count = 0;
	float partial;
	
	do{
		sem_wait(s2);
		sem_wait(s4);
		if (s->value > s->difference + 1){
			first_factor = s->value--;
			second_factor = s->value--;
		}else if(s->value == s->difference + 1){
			first_factor = s->value--;
			second_factor = 1;
		}
		
		numer = first_factor * second_factor;
		
		sem_wait(s1);
		local_count = ++count;
		sem_post(s1);
		
		if(local_count == 2){
			partial = (float) numer / (float)denom;
			result *= partial;
			printf("a I am dividing %d for %d, result is %f local count %d partial %f.\n", numer, denom, result, local_count, partial);
			count = 0;
		}
		local_count = 0;
		sem_post(s2);
		sem_post(s3);
		printf("num: s value is %d and diff %d.\n", s->value, s->difference);
	}while(s->value != s->difference);
		
}


void* denominator(void *a){
	
	newtype *s = (newtype*) a;
	int first_factor, second_factor, local_count = 0;
	float partial;
	 
	do{
		sem_wait(s2);
		sem_wait(s3);
		
		if (s->value > 1){
			first_factor = s->value--;
			second_factor = s->value--;
		}else if(s->value == 1){
			first_factor = s->value--;
			second_factor = 1;
		}
		denom = first_factor * second_factor;
		
		sem_wait(s1);
		local_count = ++count;
		sem_post(s1);
		
		if(local_count == 2){
			partial = (float)numer / (float)denom;
			result *= partial;
			printf("b I am dividing %d for %d, result is %f local count %d partial %f.\n", numer, denom, result, local_count, partial);
			count = 0;	
		}
		local_count = 0;
		sem_post(s2);
		sem_post(s4);
		
		printf("den: s value is %d and diff %d.\n", s->value, s->difference);
		
	}while(s->value != 0);
	
}
