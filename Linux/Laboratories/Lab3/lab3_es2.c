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

typedef struct Cond{
	pthread_mutex_t lock;  
	pthread_cond_t  cond;
	int             nr;
} Cond;

Cond cond;
pthread_mutex_t meR, meW;

void *writer (void *a){

	int *i = (int *) a;

	sleep((rand() % 501) / 1000);
	printf("Writer %d trying to write\n", *i);
	pthread_mutex_lock(&meW);
	pthread_mutex_lock(&cond.lock);
	printf ("Thread n. %d writing\n", *i);
	sleep(1);
	pthread_mutex_unlock(&cond.lock);
	pthread_mutex_unlock(&meW);
	return NULL;
}


void *reader (void *a){
  
	int *i = (int *) a;

	sleep((rand() % 501) / 1000);
	printf("Reader %d trying to read\n", *i);
	pthread_mutex_lock(&meR);
	cond.nr++;
	if (cond.nr == 1){
		pthread_mutex_lock(&cond.lock);
	}
	pthread_mutex_unlock(&meR);

	printf ("Thread n. %d reading\n", *i);
	sleep (1);

	pthread_mutex_lock(&meR);
	cond.nr--;
	if (cond.nr == 0){
		pthread_mutex_unlock(&cond.lock);
	}
	pthread_mutex_unlock(&meR);

	return NULL;
}

int main (int argc, char *argv[]){
	
	srand(time(NULL));
	
	pthread_t 	*th_a, *th_b;
	int 		i, *v, n;
	
	if (argc < 2) perror("Not enough arguments.\n");
	else n = atoi(argv[1]);

	th_a = (pthread_t *) malloc(n * sizeof(pthread_t));
	th_b = (pthread_t *) malloc(n * sizeof(pthread_t));
	
	pthread_mutex_init(&cond.lock, NULL);
    	pthread_cond_init (&cond.cond, NULL);
	cond.nr = 0;
	pthread_mutex_init(&meW, NULL);
	pthread_mutex_init(&meR, NULL);
	
	for (i = 0; i < n; i++) {
		v = (int *) malloc (sizeof (int));
		*v = i;
		pthread_create (&th_a[i], NULL, reader, v);
	}

	for (i = 0; i < n; i++) {
		v = (int *) malloc (sizeof (int));
		*v = i;
		pthread_create (&th_b[i], NULL, writer, v);
	}
	
	for (i = 0; i < n; i++){
		pthread_join(th_a[i], NULL);
		pthread_join(th_b[i], NULL);
	}
	
	pthread_exit (0);

}
