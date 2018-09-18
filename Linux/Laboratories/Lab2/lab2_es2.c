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

int totalThreads(int);
int power(int, int);
void *recursiveCreation();

int j, n_threads, n_level;
pthread_t *threads;
sem_t *sem;

int main(int argc, char *argv[]){
	
	
	int			rc, i, t;
	
	sem = (sem_t *) malloc(sizeof(sem_t));
	sem_init (sem, 0, 1);
	
	if (argc < 2) perror("Not enough arguments.\n");
	else n_level = atoi(argv[1]);
	
	n_threads = totalThreads(n_level);
	
	threads = (pthread_t*)malloc(n_threads * sizeof(pthread_t));
	
	printf("nt is %d and nl is %d.\n", n_threads, n_level);
	
	if (n_threads > 1) {
		
		j = 0;
		
		for (i = 0; i < 2; i++){		
			rc = pthread_create (&threads[i], NULL, recursiveCreation, (void*)pthread_self());
			if (rc) {
			  perror("ERROR: pthread_create()");
			  exit(-1);
			}
		}
	}
	printf("Thread tree: %li ", pthread_self());
	
	
	for (t = 0; t < n_threads; t++) {
		pthread_join (threads[t], NULL);
	}

    return 0;
}

void *recursiveCreation(void * ps){
	
	int i, rc;
	
	sem_wait(sem);
	j += 1;
	
	if (n_threads > 1){
		n_threads--;
		for (i = j; i < j + 2; i++){		
			rc = pthread_create (&threads[i], NULL, recursiveCreation, NULL);
			if (rc) {
			  fprintf (stderr, "ERROR: pthread_create()\n");
			  exit(-1);
			}
		}
	}
	//else { //if(n_threads > (n_threads - power(2, n_level))){
		printf("%d ", j);
		printf("%li\n", pthread_self());
	//}
	sem_post(sem);
	
	pthread_exit(NULL);
}

int totalThreads(int nl){
	
	int i, total = 0;
	
	for (i = 0; i <= nl; i++){
		total += power(2, i);
	}
	
	return total;
}

int power(int b, int e){
	
	int i, result = 1;
	
	for (i = 0; i < e; i++){
		result = result * b;
	}
	
	return result;
}
