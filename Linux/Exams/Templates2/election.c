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
	int			threads_id;
	int			threads_rank;
} newtype;


newtype 	bestrank;
int 		n, count;
sem_t 		*s1, *s2;

void* t_action(void *s);

int main(int argc, char* argv[]){
	
	
	int			i;
	pthread_t   *threads;
	newtype     *threads_att;
	
	srand(time(NULL));
	
	if (argc < 2) perror("Note enough arguments.\n");
	else n = atoi(argv[1]);
	
	threads = (pthread_t*) malloc (n * sizeof(pthread_t));
	threads_att = (newtype*) malloc (n * sizeof(newtype));
	s1 = (sem_t*)malloc(sizeof(sem_t));
	s2 = (sem_t*)malloc(sizeof(sem_t));
	
	count = 0;
	bestrank.threads_rank = 0;
	bestrank.threads_id = 0;
	sem_init(s1, 0, 1);
	sem_init(s2, 0, 0);
	
	for(i = 0; i < n; i++){
		threads_att[i].threads_id = i;
		threads_att[i].threads_rank = rand() % n;
		pthread_create(&threads[i], NULL, t_action, &threads_att[i]);
	}
	
	for (i = 0; i < n; i++){
		pthread_join(threads[i], NULL);
	}
	
	sem_destroy(s1);
	sem_destroy(s2);
	
	return 0;
}



void* t_action(void* s){
	
	newtype *arg = (newtype*) s;
	int i, local_count = 0;
	
	if (arg->threads_rank > bestrank.threads_rank){
			bestrank.threads_rank = arg->threads_rank;
			bestrank.threads_id = arg->threads_id;
	}
	sem_wait(s1);
	local_count = ++count;
	sem_post(s1);
	
	if (local_count == n){
		for(i = 0; i < n; i++){
			sem_post(s2);
		}
	}
	sem_wait(s2);
	
	printf("I am thread %d and my rank is %d.\nBest rank is thread %d, rank %d.\n", 
		arg->threads_id, arg->threads_rank, bestrank.threads_id, bestrank.threads_rank);
		
	pthread_exit(NULL);
}
	
	
