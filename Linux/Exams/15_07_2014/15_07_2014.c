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

#define NT 2

int 		n, k, numer, denom;
float 		result = 1;
sem_t 		*s1, *s2;


void *numerator(void *l);
void *denominator(void *l);

int main(int argc, char *argv[]){
	
	pthread_t 	threads[NT];
	int			i, last;
	
	
	if (argc < 3)perror("Not enough args.\n");
	else{
		n = atoi(argv[1]);
		k = atoi(argv[2]);
	}
	
	s1 = (sem_t*)malloc(sizeof(sem_t));
	s2 = (sem_t*)malloc(sizeof(sem_t));
	
	sem_init(s1, 0, 0);
	sem_init(s2, 0, 0);
	
	last = (k % 2 == 1)		?	 (k / 2) + 1 	:	 k / 2;
	
	pthread_create(&threads[0], NULL, numerator, (void*)&last);
	pthread_create(&threads[1], NULL, denominator, (void*)&last);
	
	for(i = 0; i < NT; i++){
		pthread_join(threads[i], NULL);
	}
	
	printf("The final result is %f\n", result);
	
	return 0;
}

void *numerator(void *l){

	int last = *(int *) l;
	int i;
	
	for (i = 0; i < last; i++){
		
		if(k % 2 == 1 && i == last - 1){
			numer = n;
		}else{
			numer = n * (n - 1);
		}
		n -= 2;
		
		sem_wait(s1);
		printf("%d: num is %d and denom %d.\n", i, numer, denom);
		result *= (float)numer / (float)denom;
		
		sem_post(s2);
	}
}

void *denominator(void *l){
	
	int last = *(int *) l;
	int i, d = 1;
	
	for (i = 0; i < last; i++){
		
		if(k % 2 == 1 && i == last - 1){
			denom = d;
		}else{
			denom = d * (d + 1);
		}
		d += 2;
		sem_post(s1);
		sem_wait(s2);
	}
}
