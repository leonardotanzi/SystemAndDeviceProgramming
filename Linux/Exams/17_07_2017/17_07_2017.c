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

#define L 					10
#define T_TR 				12
#define TRAIN_CAPACITY 		10
#define T_MAX				3
#define T_NEW_PASS			10
#define N_TRAIN				2
#define N_STATION			4


typedef struct{
	
	int 	startStation;
	int 	destStation;
	int 	passengerId;
	sem_t	*end;
	
}PASSENGER;

typedef struct{

	int 		stationId;
	int			in, out;
	PASSENGER 	buffer[L];
	sem_t		*full;
	sem_t		*empty;
	sem_t		*trainSem;
	int			piped[2];
	
}STATION;


typedef struct{

	int 		startStation;
	int 		trainId;
	int 		stationId;
	PASSENGER	ids[TRAIN_CAPACITY]; //inializzo a -1
	int			countPass;
	
}TRAIN;


typedef struct{
	
	int id;
	int nPass;
	int nDown;
	
}PIPE_DATA;


STATION		stations[N_STATION];
PASSENGER	passengers[L];
TRAIN		trains[N_TRAIN];
sem_t		*sTmp;

void *timer();
void *trainsAction(void *arg);
void *stationsAction(void *arg);
void *passAction(void *arg);


int main(int argc, char *argv[]){
	
	pthread_t		*threads = (pthread_t*)malloc((L + N_TRAIN + N_STATION) * sizeof(pthread_t));
	pthread_t		*tmpT = (pthread_t*)malloc(L * sizeof(pthread_t));
	int				i, j, tmp, tmp2;
	
	sTmp = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sTmp, 0, 0);
	
	for(i = 0; i < N_TRAIN; i++){
		trains[i].trainId = i;
		if(i == 0){
			tmp = rand() % 4;
			trains[i].stationId = tmp;		//questo in modo da non venire assegnati in partenza alla stesa stazione
		}else{
			do{
				tmp2 = rand() % 4;
			}while(tmp2 == tmp);
			
			trains[i].stationId = tmp2;
		}
		trains[i].countPass = 0;
		trains[i].startStation = trains[i].stationId;
		for(j = 0; j < TRAIN_CAPACITY; j++){
			trains[i].ids[j].passengerId = -1 ;
		}
		pthread_create(&threads[i], NULL, trainsAction, (void*)&trains[i]);
	}
	
	for(i = 0; i < N_STATION; i++){
		stations[i].stationId = i;
		stations[i].full = (sem_t*)malloc(sizeof(sem_t));
		sem_init(stations[i].full, 0, 0);
		stations[i].empty = (sem_t*)malloc(sizeof(sem_t));
		sem_init(stations[i].empty, L, 0);
		stations[i].trainSem = (sem_t*)malloc(sizeof(sem_t));
		for(j = 0; j < N_TRAIN; j++){
			if(trains[i].stationId == i){
				sem_init(stations[i].trainSem, 1, 0);
			}else{
				sem_init(stations[i].trainSem, 0, 0);
			}
		}
		stations[i].in = 0;
		stations[i].out = 0;
		pthread_create(&threads[i + N_TRAIN], NULL, stationsAction, (void*)&stations[i]);
	}
	
	for(i = 0; i < L; i++){
		passengers[i].passengerId = i;
		
		passengers[i].startStation = rand() % N_STATION;
		do{
			passengers[i].destStation = rand() % N_STATION;
		}while(passengers[i].startStation == passengers[i].destStation);
		
		passengers[i].end = (sem_t*)malloc(sizeof(sem_t));
		sem_init(passengers[i].end, 0, 0);
		
		pthread_create(&threads[i + N_TRAIN + N_STATION], NULL, passAction, (void*)&passengers[i]);
		pthread_create(&tmpT[i], NULL, timer, NULL );
		sem_wait(sTmp);
		
	}
	
	for(i = 0; i < N_TRAIN + N_STATION + L; i++){
		pthread_join(threads[i], NULL);
	}
	return 0;
}

//----------------------------------------------------------------------------------------

void *trainsAction(void *arg){
	
	TRAIN 		train = *(TRAIN*)arg;
	int			nextStation, down, i;
	struct 		timespec ts;
	PASSENGER	*pass;
	PIPE_DATA	datas;
	
	
	while(1){
		sleep(T_TR);
		if (train.stationId != 3){
			nextStation = train.stationId + 1;
		}else{
			nextStation = 0;
		}
		sem_wait(stations[nextStation].trainSem);
		sem_post(stations[train.stationId].trainSem); //libero la stazione che sto lasciando
		train.stationId = nextStation; //unlock the sem, quindi sono nella nuova station
		
		do{
			ts.tv_sec = time(NULL) + T_MAX;
			ts.tv_nsec = 0;
			sem_timedwait(stations[train.stationId].full, &ts); //consumer preòeva i passeggeri
			dequeue(pass);
			for(i = 0; i < TRAIN_CAPACITY; i++){ //serve per metterlo dove c'è un buco cioè -1
				if(train.ids[i].passengerId == -1){
				train.ids[i].startStation = pass.startStation;
				train.ids[i].destStation = pass.destStation;
				train.ids[i].passengerId = pass.passengerId;
			}
			train.countPass++;
		}while(train.countPass <= TRAIN_CAPACITY);
		
		//printf();
	}
	down = 0;
	for(i = 0; i < countPass; i++){
		if(train.ids[i].destStation == train.stationId){
			sem_post(passengers[trains.ids[i].passengerId].end); //tolgo il thread dal treno
			train.ids[i].passengerId = -1; //cosi so che non c'è più nel treno
			train.countPass--;
			down++;
		}
	}
	datas.id = train.trainId;
	datas.nPass = train.countPass;
	datas.nDown = down;
	write(stations[train.stationId].piped[1], &datas, sizeof(datas));
}
	
//----------------------------------------------------------------------------------------
	
void *passAction(void *arg){
	
	PASSENGER pass = *(PASSENGER*) arg;
	
	sem_wait(stations[pass.startStation].empty);
	
	enqueue(pass);
	
	sem_post(stations[pass.startStation].full);
	sem_wait(passengers[pass.passengerId].end);
	
	return 1;
}

//----------------------------------------------------------------------------------------


void *stationsAction(void *arg){
	
	STATION 	stat = *(STATION*)arg;
	PIPE_DATA	tmp;
	
	while(1){
		read(stations[stat.stationId].piped[0], &tmp, sizeof(tmp));
		//printf();
	}
	return 1;
}
	
//----------------------------------------------------------------------------------------
	
	
void enqueue (PASSENGER val) {
	
	stations[val.stationId].queue[stations[val.stationId].in].startStation = val.startStation;
	stations[val.stationId].queue[stations[val.stationId].in].destStation = val.desttStation;
	stations[val.stationId].queue[stations[val.stationId].in].passengerId = val.passengerId;
	
	stations[val.stationId].in = (stations[val.stationId].in + 1) % L;
	
	return;
}

void dequeue (PASSENGER *val) {
	
	*val.startStation = stations[val.stationId].queue[stations[val.stationId].out].startStation;
	*val.destStation = stations[val.stationId].queue[stations[val.stationId].out].destStation;
	*val.passengerId = stations[val.stationId].queue[stations[val.stationId].out].passengerId;
	
	stations[val.stationId].out = (stations[val.stationId].out + 1) % L;
	return;
}
	
//----------------------------------------------------------------------------------------
	
	
//uso un thread per dormire perchè se dormisse il main dormirebbero anche tutti i thread
void *timer(){
	sleep(rand() % T_NEW_PASS + 1);
	sem_post(sTmp);
	return;
}
