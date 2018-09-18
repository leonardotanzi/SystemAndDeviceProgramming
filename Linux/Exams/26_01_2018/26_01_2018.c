#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>

int fId = 0, nTh, sum = 0, balancing, lastBalancing, nSignal, end = 0, globalId = 0;
sem_t *s0, *s1, *s2, *s3, *s4;

void *thAction();

int main(int argc, char *argv[]){
	
	nTh = atoi(argv[1]);
	int 				n = atoi(argv[2]);
	pthread_t			*threads = (pthread_t*)malloc(nTh * sizeof(pthread_t));
	int 				lenVec, fd1, fd2, i, j, tmp1, tmp2, sumInner, *thIds = (int*)malloc(nTh * sizeof(int));
	char 				name1[32], name2[32];
	
	
	s0 = (sem_t*)malloc(sizeof(sem_t));
	s1 = (sem_t*)malloc(sizeof(sem_t));
	s2 = (sem_t*)malloc(sizeof(sem_t));
	s3 = (sem_t*)malloc(sizeof(sem_t));
	s4 = (sem_t*)malloc(sizeof(sem_t));
	
	sem_init(s0, 0, 0);
	sem_init(s1, 0, 0);
	sem_init(s2, 0, 1);
	sem_init(s4, 0, 1);
	
	
	srand(time(NULL));
	
	for(i = 0; i < nTh; i++){
		thIds[i] = i + 1;
		pthread_create(&threads[i], NULL, thAction, (void*)&thIds[i]);
	}
	
	
	for(i = 0; i < 10; i++){
		
		lenVec = rand() % n + 1;
		sprintf(name1, "F1_%d.bin", fId);
		sprintf(name2, "F2_%d.bin", fId); 
		if ((fd1 = open(name1, O_CREAT | O_WRONLY | O_TRUNC), 0644) == -1){
			printf("File1 error.\n");
		}
		if((fd2 = open(name2, O_CREAT | O_WRONLY | O_TRUNC), 0644) == -1){
			printf("File2 error.\n");
		}
		
		printf("name1 %s name2 %s.\n", name1, name2);
		
		for(j = 0; j < lenVec; j++){
			
			tmp1 = rand() % 101;
			printf("tmp1 is %d\n", tmp1);
			write(fd1, &tmp1, sizeof(int));
			tmp2 = rand() % 101;
			printf("tmp2 is %d\n", tmp2);
			write(fd2, &tmp2, sizeof(int));
		}
		
		
		if (lenVec <= nTh){
			balancing = 1;
			lastBalancing = 1;
			nSignal = lenVec;
		}else{
			balancing = lenVec / nTh;
			lastBalancing = balancing + (lenVec % nTh);
			nSignal = nTh;
		}
		for(j = 0; j < nSignal; j++){
			sem_post(s0);
		}
		for(j = 0; j < nSignal; j++){
			sem_wait(s1);
		}
		fId++;
		for(j = 0; j < lenVec; j++){
			lseek(fd1, 0, SEEK_SET);
			read(fd1, &tmp1, sizeof(tmp1));
			lseek(fd2, 0, SEEK_SET);
			read(fd2, &tmp2, sizeof(tmp2));
			sumInner += (tmp1 * tmp2);
			printf("main tmp1 %d tmp2 %d\n", tmp1, tmp2);
		}
		printf("SumInner = %d and sumOuter = %d.\n", sumInner, sum);	
	}
	end = 1;
	
	for(j = 0; j < nSignal; j++){
		sem_post(s0);
	}
	
	for(j = 0; j < nTh; j++){
		pthread_join(threads[j], NULL);
	}
	
	return 0;
}


void *thAction(){
	int fd1, fd2, endCycle, id, tmp1, tmp2, i;
	char name1[32], name2[32];
	
	while(1){
		sem_wait(s0);

		if(end == 1){
			break;
		}

		sem_wait(s4);
		globalId++;
		id = globalId;
		sem_post(s4);
		
		sprintf(name1, "F1_%d.bin", fId);
		sprintf(name2, "F2_%d.bin", fId); 
		fd1 = open(name1, O_CREAT | O_RDWR | O_TRUNC, 0644);
		fd2 = open(name2, O_CREAT | O_RDWR | O_TRUNC, 0644);
		
		lseek(fd1, (id * balancing - 1) * sizeof(int), SEEK_SET);
		lseek(fd2, (id * balancing - 1) * sizeof(int), SEEK_SET);
		
		if(nSignal == id){
			endCycle = lastBalancing;
		}else{
			endCycle = balancing;
		}
		for(i = 0; i < endCycle; i++){
			read(fd1, &tmp1, sizeof(tmp1));
			read(fd2, &tmp2, sizeof(tmp2));
			
			sem_wait(s2);
			sum += (tmp1 * tmp2);
			printf("th %d sum is %d, tmp1 %d tmp2 %d\n", id, sum, tmp1, tmp2);
			sem_post(s2);
		}
		sem_post(s1);
	}
	close(fd1);
	close(fd2);
	return NULL;
}
		
		 
