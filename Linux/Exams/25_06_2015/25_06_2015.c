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

sem_t				*tA, *tB, *AA, *BB, *synch;
pthread_mutex_t 	mutexA, mutexB;
int 				iA = 0, iB = 0, a_id, b_id, counter = 0, *Aids, *Bids;


void * typeA(void *arg);
void * typeB(void *arg);


int main(int argc, char *argv[]){
	
	pthread_t		*a_threads, *b_threads;
	int				i, *id_threadsA, *id_threadsB, n;
	
	srand(time(NULL));
	
	if(argc < 2) perror("Not enough args.\n");
	else n = atoi(argv[1]);
	
	a_threads = (pthread_t*)malloc(n * sizeof(pthread_t));
	b_threads = (pthread_t*)malloc(n * sizeof(pthread_t));
	id_threadsA = (int*)malloc(n * sizeof(int));
	id_threadsB = (int*)malloc(n * sizeof(int));
	Aids = (int*)malloc(2 * sizeof(int));
	Bids = (int*)malloc(2 * sizeof(int));
	
	tA = (sem_t*)malloc(sizeof(sem_t));
	tB = (sem_t*)malloc(sizeof(sem_t));
	AA = (sem_t*)malloc(sizeof(sem_t));
	BB = (sem_t*)malloc(sizeof(sem_t));
	synch = (sem_t*)malloc(sizeof(sem_t));
	
	sem_init(tA, 0, 2);
	sem_init(AA, 0, 0);
	sem_init(tB, 0, 2);
	sem_init(BB, 0, 0);
	sem_init(synch, 0, 1);
	
	pthread_mutex_init(&mutexA, NULL);
	pthread_mutex_init(&mutexB, NULL);
	
	for(i = 0; i < n; i++){
		id_threadsA[i] = i;
		pthread_create(&a_threads[i], NULL, typeA, (void*)&id_threadsA[i]);
	}
	for(i = 0; i < n; i++){
		id_threadsB[i] = i;
		pthread_create(&b_threads[i], NULL, typeB, (void*)&id_threadsB[i]);
	}
	
	for(i = 0; i < n; i++){
		pthread_join(a_threads[i], NULL);
		pthread_join(b_threads[i], NULL);
	}
	return 0;
}


void * typeA(void *arg){
	
	int id = *(int*)arg;
	int x;
	
	sleep(rand() % 4);
	
	sem_wait(tA);
	pthread_mutex_lock(&mutexA);
	iA++;
	x = iA;
	pthread_mutex_unlock(&mutexA);
	
	if (x == 1){
		a_id = id;
		sem_wait(AA);
		sem_post(tA); //mandato due volte
		sem_post(tA); //mandato due volte
	}else if (x == 2){
		printf("A%d cats A%d A%d.\n", id, id, a_id);
		sem_wait(synch);
		counter++;
		if(counter == 1){
			Aids[0] = id;
			Aids[1] = a_id;
			sem_post(synch);
		}else if(counter == 2){
			printf("A%d merges A%d A%d B%d B%d.\n\n", id, id, a_id, Bids[0], Bids[1]);
			counter = 0;
			sem_post(synch); //per i prossimi thread
			sem_post(AA);
			sem_post(BB);
		}
	}
	
	iA = 0;
}


void * typeB(void *arg){
	
	int id = *(int*)arg;
	int x;
	
	sleep(rand() % 4);
	
	sem_wait(tB);
	
	pthread_mutex_lock(&mutexB);
	iB++;
	x = iB;
	pthread_mutex_unlock(&mutexB);
	
	if (x == 1){
		b_id = id;
		sem_wait(BB);
		sem_post(tB);
		sem_post(tB);
	}else if (x == 2){
		printf("B%d cats B%d B%d.\n", id, id, b_id);
		sem_wait(synch);
		counter++;
		if(counter == 1){
			Bids[0] = id;
			Bids[1] = b_id;
			sem_post(synch);
		}else if(counter == 2){
			printf("B%d merges B%d B%d A%d A%d.\n\n", id, id, b_id, Aids[0], Aids[1]);
			counter = 0;
			sem_post(synch); //per i prossimi thread
			sem_post(AA);
			sem_post(BB);
		}
	}
	iB = 0;
}
