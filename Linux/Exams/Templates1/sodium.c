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

int 	k, iS = 0, iC = 0, iPrint = 0;
sem_t   *sem, *scs1, *scs2, *scs3, *x, *y;
int 	*producedSodium, *producedChlorine;

void *First();
void *Second();
void *Sodium(void * arg);
void *Chlorine(void * arg);

int main(int argc, char *argv[]){
	
	if (argc < 2) perror("Not enough args.\n");
	else k = atoi(argv[1]);
	
	pthread_t 	threads[2];
	int 		i;
	
	producedSodium = (int*)malloc(k * sizeof(int));
	producedChlorine = (int*)malloc(k * sizeof(int));
	
	sem = (sem_t*)malloc(sizeof(sem_t));
	scs1 = (sem_t*)malloc(sizeof(sem_t));
	scs2 = (sem_t*)malloc(sizeof(sem_t));
	scs3 = (sem_t*)malloc(sizeof(sem_t));
	x = (sem_t*)malloc(sizeof(sem_t));
	y = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem, 0, 0),
	sem_init(scs1, 0, 1);
	sem_init(scs2, 0, 1);
	sem_init(scs3, 0, 1);
	sem_init(x, 0, 0);
	sem_init(y, 0, 0);
	
	pthread_create(&threads[0], NULL, First, NULL);
	pthread_create(&threads[1], NULL, Second, NULL);
	
	for(i = 0; i < 2; i++){
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}

void *First(){
	
	pthread_t 	threads[k];
	int 		i, threadsId[k];
	
	for(i = 0; i < k; i++){
		threadsId[i] = i;
		pthread_create(&threads[i], NULL, Sodium, &threadsId[i]);
	}
	for(i = 0; i < k; i++){
		pthread_join(threads[i], NULL);
	}
	
	return;
}
	
void *Second(){
	
	pthread_t 	threads[k];
	int 		i, j, threadsId[k];
	
	for(i = k, j = 0; i < k * 2; i++, j++){
		threadsId[j] = i;
		pthread_create(&threads[j], NULL, Chlorine, &threadsId[j]);
	}
	for(i = 0; i < k; i++){
		pthread_join(threads[i], NULL);
	}
	
	return;
}
	
void *Sodium(void * arg){
	
	int	thId = *(int *) arg;
	
	sleep(rand() % 5);
	
	sem_wait(scs1);
	producedSodium[iS] = thId;
	iS++;
	sem_post(scs1);
	
	sem_wait(sem);
	
	sem_wait(scs3);
	printf("id: %d  - Na %d Cl %d.\t", iPrint, producedSodium[iPrint], producedChlorine[iPrint]);
	sem_post(x);
	sem_wait(y);
	//iPrint++;
	sem_post(scs3);
	
	return;
}
	

void *Chlorine(void * arg){
	
	int	thId = *(int *) arg;
	
	sleep(rand() % 5);
	
	sem_wait(scs2);
	producedChlorine[iC] = thId;
	iC++;
	sem_post(scs2);
	
	sem_post(sem);
	
	sem_wait(x);
	printf("id: %d  - Na %d Cl %d.\n", iPrint + k, producedSodium[iPrint], producedChlorine[iPrint]);
	iPrint++;
	sem_post(y);
	
	
	return;
}
	
	
	
