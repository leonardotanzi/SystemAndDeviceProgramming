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
#include <signal.h>

typedef struct{
	int id;
	int rank;
}TH_ARG;

TH_ARG 	best_rank;
int		counter = 0, n;
sem_t	*start, *end, *br;

void *thAction(void *arg);
void handler();

int main(int argc, char * argv[]){
	
	pthread_t			*threads;
	TH_ARG				*th_args;
	int					i, j, *ids, rnd, continueloop;
	
	srand(time(NULL));
	
	if((n = atoi(argv[1])) <= 8){
		return 1;
	}
	
	threads = (pthread_t*)malloc(n * sizeof(pthread_t));
	th_args = (TH_ARG*)malloc(n * sizeof(TH_ARG));
	ids = (int*)malloc((n - 3) * sizeof(int));
	start = (sem_t*)malloc(sizeof(sem_t));
	end = (sem_t*)malloc(sizeof(sem_t));
	br = (sem_t*)malloc(sizeof(sem_t));
	
	sem_init(start, 0, 0);
	sem_init(end, 0, 0);
	sem_init(br, 0, 1);
	
	best_rank.rank = 0;
	
	for(i = 0; i < n; i++){
		
		th_args[i].id = i;
		th_args[i].rank = (rand() % n) + 1;
		pthread_create(&threads[i], NULL, thAction, (void*)&th_args[i]);
	}
	
	sleep(rand() % 4 + 2);
	
	
	for( i = 0; i < n - 3; i++){
		
		if (i == 0){
			ids[0] = rand() % n;
		}
		else{
			do{
				continueloop = 1;
				rnd = rand() % n;
				for(j = 0; j < i + 1; j++){
					 if(ids[j] == rnd){
						 continueloop = 0;
					 }
				 }if(continueloop == 1){
					 ids[i] = rnd;
				 }
			 }while(continueloop == 0);
		 }
	 }
	 for(i = 0; i < n - 3; i++){
		 printf("send to %d.\n", ids[i]);
		 pthread_kill(threads[ids[i]], SIGUSR1);
	 }
			
	
	for(i = 0; i < n; i++){
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}


void *thAction(void *arg){
	
	TH_ARG 		*data = (TH_ARG*)arg;
	int			localCounter, i;
	
	signal(SIGUSR1, handler);
	pause();
	
	sem_wait(start);
	printf("Thread %d started.\n", data->id);
	
	sem_wait(br);
	
	if (data->rank > best_rank.rank){
		best_rank.rank = data->rank;
		best_rank.id = data->id;
	}
	localCounter = ++counter;
	
	sem_post(br);
	
	if(localCounter == n - 3){
		for(i = 0; i < n - 3; i++){
			sem_post(end);
		}
	}
	sem_wait(end);
	
	printf("Thread %d: rank %d.\nLeader id %d with rank %d.\n", data->id, data->rank, best_rank.id, best_rank.rank);
	
	return;
	
}


void handler(){
	
	sem_post(start);

	return;
}
