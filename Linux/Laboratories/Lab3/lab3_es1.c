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

#define BUF_SIZE 	16
#define OVER		(-1)
#define N_THREADS 	2
#define NORMAL 		0
#define URGENT		1


typedef struct Buffer{
  sem_t *empty, *full;
  int in, out;
  long long *buffer;
  int size;
}Buffer;


void *producer();
void *consumer();
long long current_timestamp();
Buffer *buffer_init_normal(int size);
Buffer *buffer_init_urgent(int size);
void put (long long data, Buffer *b);
long long getUrgent();
long long getNormal();


Buffer 		*b_urgent, *b_normal;
int			p;
sem_t 		*s;

int main(int argc, char *argv[]){
	
	int 		i;
	pthread_t	threads[N_THREADS];
	
	srand(time(NULL));
	
	if (argc < 2 ) perror ("Not enough arguments.\n");
	else p = atoi(argv[1]);
	
	buffer_init_normal(BUF_SIZE);
	buffer_init_urgent(BUF_SIZE);
	
	s = (sem_t *) malloc(sizeof(sem_t));
	sem_init (s, 0, 0);
	
	pthread_create(&threads[0], NULL, producer, NULL);
	pthread_create(&threads[1], NULL, consumer, NULL);
	
	for (i = 0; i < 2; i++){
		pthread_join(threads[i], NULL);
	}
	
	sem_destroy(s);
	
	return 0;
}



void *producer(){
	
	int 		rnd, i, type_of_buffer;
	long long 	ms;
	
	for (i = 0; i < 100; i++){
		rnd = rand() % 9001 + 1000;
		printf("Thread Producer %ld: sleep %d ms.\n", pthread_self(), rnd);
		usleep(rnd);
		ms = current_timestamp();
		rnd = rand() % 101;
		printf("Thread Producer %ld: ms is %lld and rnd is %d.\n", pthread_self(), ms, rnd);
		if (rnd < p){
			type_of_buffer = NORMAL;
		}else{
			type_of_buffer = URGENT;
		}
		printf("Thread Producer %ld: ms is %lld and buffer is %d.\n", pthread_self(), ms, type_of_buffer);
			
		if (type_of_buffer == NORMAL){
			put (ms, b_normal);
		}else{
			put (ms, b_urgent);
		}
		sem_post(s);
	}
	pthread_exit(NULL);
	
}

void *consumer(){
	
	int		  	i, buff_type = URGENT;
	long long 	d;
	
	
	for (i = 0; i < 100; i++){
		sem_wait(s);
		usleep(10000);
		if ((d = getUrgent()) == -1){
			d = getNormal();
			buff_type = NORMAL;
		}
		printf ("Thread Consumer %ld: ms read %lld and buffer is %d.\n", pthread_self(), d, buff_type);
    }
    pthread_exit(NULL);
	
}



Buffer * buffer_init_normal(int size){
  
	b_normal = (Buffer *) malloc(sizeof(Buffer));
	b_normal->size = size;
	b_normal->buffer = (long long *) malloc(b_normal->size * sizeof(long long));
	b_normal->in = 0;
	b_normal->out = 0;
	b_normal->empty = (sem_t *) malloc(sizeof(sem_t)); 
	sem_init (b_normal->empty, 0, b_normal->size); //tutto vuoto quindi sem inizializzato a 16
	b_normal->full = (sem_t *) malloc(sizeof(sem_t));  
	sem_init (b_normal->full, 0, 0); //tutto vuoto quindi sem full inizializzato a 0 (nulla da leggere)

	return b_normal;
}

Buffer * buffer_init_urgent(int size){
  
	b_urgent = (Buffer *) malloc(sizeof(Buffer));
	b_urgent->size = size;
	b_urgent->buffer = (long long *) malloc(b_urgent->size * sizeof(long long));
	b_urgent->in = 0;
	b_urgent->out = 0;
	b_urgent->empty = (sem_t *) malloc(sizeof(sem_t)); 
	sem_init (b_urgent->empty, 0, b_urgent->size); //tutto vuoto quindi sem inizializzato a 16
	b_urgent->full = (sem_t *) malloc(sizeof(sem_t));  
	sem_init (b_urgent->full, 0, 0); //tutto vuoto quindi sem full inizializzato a 0 (nulla da leggere)

	return b_urgent;
}


void put (long long data, Buffer *b){
		
	sem_wait(b->empty);
	
	b->buffer[b->in] = data;
	b->in = (b->in + 1) % b->size;  //se è 1 metto all'uno, se è 17 metto all'1 e cosi via
	sem_post(b->full);
}

long long getUrgent (){
	
	long long data;
	if (sem_trywait(b_urgent->full) != 0){
		return -1;
	}
	data = b_urgent->buffer[b_urgent->out];
	b_urgent->out = (b_urgent->out + 1) % b_urgent->size;
	sem_post(b_urgent->empty);

	return data;
}

long long getNormal (){
	
	long long data;
	sem_wait(b_normal->full);
	data = b_normal->buffer[b_normal->out];
	b_normal->out = (b_normal->out + 1) % b_normal->size;
	sem_post(b_normal->empty);

	return data;
}


long long current_timestamp() {
	
	struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; //moltiplica secondi per mille e nanosecondi divide per mille
    
    return milliseconds;
}
