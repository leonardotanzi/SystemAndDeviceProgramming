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

sem_t 		*s1, *s2, *s3, *s4;
int 		g = 0;


void * client(void * f);

int main(int argc, char * argv[]){
		
	int 		n1, n2, i, counter = 0;
	int 		*v1, *v2;
	FILE 		*f1, *f2;
	pthread_t	*threads;
	
	if (argc < 3) perror("Not enough arg.\n");
	else {
		n1 = atoi(argv[1]);
		n2 = atoi(argv[2]);
	}
	
	v1 = (int*) malloc (n1 * sizeof(int));
	v2 = (int*) malloc (n2 * sizeof(int));
	threads = (pthread_t*) malloc (2 * sizeof(pthread_t));
	s1 = (sem_t *) malloc(sizeof(sem_t));
	s2 = (sem_t *) malloc(sizeof(sem_t));
	s3 = (sem_t *) malloc(sizeof(sem_t));
	s4 = (sem_t *) malloc(sizeof(sem_t));
	
	sem_init (s1, 0, 1);
	sem_init (s2, 0, 0);
	sem_init (s3, 0, 0);
	sem_init (s4, 0, -1);
	
	srand(time(NULL));
	
	for (i = 0; i < n1; i++){
		v1[i] = rand() % 91 + 10;
	}
	
	for (i = 0; i < n2; i++){
		v2[i] = rand() % 81 + 21;
	}
	
	if ((f1 = fopen ("fv1.b", "rb+")) == NULL){
		perror("Error open 1.\n");
		return(1); 
	}

	fwrite(v1, sizeof(v1), n1, f1); 
	fseek(f1, 0, SEEK_SET);
	
	if ((f2 = fopen ("fv2.b", "rb+")) == NULL){
		perror("Error open 2.\n");
		return(1); 
	}
	
	fwrite(v2, sizeof(v2), n2, f2); 
	fseek(f2, 0, SEEK_SET);
	/*
	pthread_create(&threads[0], NULL, client, (void*) f1);
	pthread_create(&threads[1], NULL, client, (void*) f2);
	
	while(sem_trywait(s4) != 0){
		sem_wait(s2);
		g *= 3;
		counter++;
		printf("counter is %d,\n", counter);
		sem_post(s3);
	}	
	
	for (i = 0; i < 2; i++){
		pthread_join(threads[i], NULL);
	}
	
	printf("Father: I've made %d request.\n", counter);
	
	sem_destroy(s1);
	sem_destroy(s2);
	sem_destroy(s3);
	sem_destroy(s4);
	*/
	
	return 0;
	
}


void *client(void *f){
	
	FILE 	*fp = (FILE*) f;
	int 	n;
	
	while(fread(&n, sizeof(int), 1, fp) > 0){
		sem_wait(s1);
		g = n;
		printf("Thread %ld, g before multiplication is %d.\n", pthread_self(), g);
		sem_post(s2);
		sem_wait(s3);
		printf("Thread %ld, g after multiplication is %d.\n", pthread_self(), g);
		sem_post(s1);
	}
	sem_post(s4);
	sem_post(s4);
	pthread_exit(NULL);
}
