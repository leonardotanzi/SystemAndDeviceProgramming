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
	int 	connectionID;
	sem_t	*sem;	
}CONNECTION;


typedef struct{	
	int 		trackId;
	int 		stationId;
	sem_t 		*sem;
	CONNECTION	*nextConnection;
}TRACK;


typedef struct{
	int 	stationId;
	TRACK 	*track;
	int 	trainId;
}TRAIN;

char 			text[2][11];
int				nStation;
TRACK			*tracks;
	

void *startTrain(void * t);

int main (int argc, char ** argv){
	
	int				nTrain, stationId, trackId, i;
	pthread_t 		*threads;
	TRAIN			*trains;
	CONNECTION 		*connections;
	
	strcpy(text[0], "ORARIO");
	strcpy(text[1], "ANTIORARIO");
	
	if (argc < 3){
		perror("Not enough args.\n");
		return 1;
	}
	else {
		nStation = atoi(argv[1]);
		nTrain = atoi(argv[2]);
	}
	
	threads = (pthread_t*) malloc(nTrain * sizeof(pthread_t));
	trains = (TRAIN*) malloc(nTrain * sizeof(TRAIN));
	connections = (CONNECTION*) malloc(nStation * sizeof(CONNECTION));
	tracks = (TRACK*) malloc(nStation * 2 * sizeof(TRACK));
	
	for(i = 0; i < nStation; i++){
		connections[i].connectionID = i;
		connections[i].sem = (sem_t*)malloc(sizeof(sem_t));
		sem_init(connections[i].sem, 0, 1);
	}
	
	//se 0 la prossima connessione ha lo stesso numero della stazione, se no id - 1
	for(i = 0; i < nStation * 2; i += 2){
		//trackid è per le due track di una stessa stazione 0 e 1
		tracks[i].trackId = 0;
		tracks[i + 1].trackId = 1;
		//station id di 0 e 1 track è 0, di 2 e 3 è 1...
		tracks[i].stationId = i % 2;
		tracks[i + 1].stationId = i % 2;
		tracks[i].sem = (sem_t*)malloc(sizeof(sem_t));
		sem_init(tracks[i].sem, 0, 1);
		tracks[i + 1].sem = (sem_t*)malloc(sizeof(sem_t));
		sem_init(tracks[i + 1].sem, 0, 1);
		//questo è per far puntare la prima track in senso antiorario con ultima con e viceversa
		if (i == 0){			
			tracks[i].nextConnection = &connections[i % 2];
			tracks[i + 1].nextConnection = &connections[nStation - 1];
		}else if(i == (nStation * 2) - 2){				
			tracks[i].nextConnection = &connections[0];
			tracks[i + 1].nextConnection = &connections[(i % 2) - 1];
		}else{	
			tracks[i].nextConnection = &connections[i % 2];
			tracks[i + 1].nextConnection = &connections[(i % 2) - 1];
		}
	}
		
	for(i = 0; i < nTrain; i++){
		
		stationId = i; 
		trackId = i % 2;
		trains[i].stationId = stationId; 
		trains[i].trainId = i;
		if(trackId == 0){
			trains[i].track = &tracks[stationId * 2];
		}else{
			trains[i].track = &tracks[stationId * 2 + 1];
		}
		
		pthread_create(&threads[i], NULL, startTrain, (void*)&trains[i]);
	}
	
	for(i = 0; i < nTrain; i++){
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}


void* startTrain(void *t){
	
	TRAIN *train = (TRAIN*) t;
	int rnd;
	
	srand(time(NULL));
	
	while(1){
		sem_wait(train->track->sem);
		printf("Train number %d: in station %d going %s.\n", train->trainId, train->stationId, text[train->track->trackId]);
		rnd = rand() % 6;
		sleep(rnd);
		sem_post(train->track->sem);
		sem_wait(train->track->nextConnection->sem);
		if (train->track->trackId == 0){
			printf("Train number %d: travelling toward station %d.\n", train->trainId, train->stationId + 1);
		}else{
			printf("Train number %d: travelling toward station %d.\n", train->trainId, train->stationId - 1);
		}
		
		sleep(10);
		
		sem_post(train->track->nextConnection->sem);
		if (train->track->trackId == 0){ 
			if (train->stationId == nStation - 1){
				train->stationId = 0;
			}else{
				train->stationId++;
			} 
			train->track = &tracks[train->stationId * 2];
		}else{
			if(train->stationId == 0){
				train->stationId = nStation - 1;
			}else{
				train->stationId--;
			}
			train->track = &tracks[train->stationId * 2 + 1];
		}
	}
	return 0;
}
