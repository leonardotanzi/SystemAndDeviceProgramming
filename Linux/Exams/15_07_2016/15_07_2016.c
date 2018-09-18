//NON FINITO, NON SO COME FARGLI LEGGERE IL '\N'


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#define SECONDS 20

typedef struct{
	int timeSleep;
	int id;
}P_ARG;


int main(int argc, char *argv[]){
	
	int 		k = atoi(argv[1]);
	int 		pipedSingle[2], **pipedMultiple, pipedMonitor[2], i, tmp, start = 1, stop = 0;
	pid_t		pid;
	P_ARG		process;
	time_t 		startTime = time(NULL);
	
	
	
	pipe(pipedSingle); //tutti i figli scrivono sulla stessa al padre
	pipe(pipedMonitor); //tutti i figli scrivono sulla stessa al monitor
	
	pipedMultiple = (int**)malloc(k * sizeof(int*)); //il padre scrive su pipe diverse ai figli
	for(i = 0; i < k; i++){
		pipedMultiple[i] = (int*)malloc(2 * sizeof(int));
		pipe(pipedMultiple[i]);
	}
	
	for(i = 0; i < k; i++){
		
		pid = fork();	//cosi si creano k figli
		
		//figli
		if(pid == 0){	
			
			while(1){
				srand(time(NULL) ^ getpid()); //per generare valori casuali 
	
				read(pipedMultiple[i][0], &tmp, sizeof(tmp));
				
				if(tmp == 1){
					
					printf("CHILD: Process %d starts.\n", i);
					process.timeSleep = rand() % 4 + 1;
					sleep(process.timeSleep);
					printf("CHILD:Process %d sleep %d secs.\n", i, process.timeSleep);
					process.id = i;
					write(pipedSingle[1], &process, sizeof(process));
					//write(pipedMonitor[1], &process.timeSleep, sizeof(process.timeSleep));
					
				}else if(tmp == 0){
					
					printf("CHILD: Process %d ends.\n", i);
					return 1;
					
				}
			}
		}
	}
	
	
	if((pid = fork()) == 0){
		
		//monitor
		int counter = 0;
		float total, mean;
		
		while(1){
				
			if(read(pipedMonitor[0], &tmp, sizeof(tmp)) == -1){
				
				//grazie a errno si puo controllare cosa ha generato l'errore per cui la read è fallita, i.e. l'interrupt
				if(errno == EINTR){
					mean = total / (float)counter;
					printf("The mean time is now %f.\n", mean);
					counter = 0;
					total = 0;
				}
			}else{
				counter++;
				total += (float)tmp;
			}
		}
	}
	
	//padre
	for(i = 0; i < k; i++){
			write(pipedMultiple[i][1], &start, sizeof(int)); //fa partire
	}
	
	while (time(NULL) - startTime < SECONDS){
		
		//si mette a leggere un po alla volta i processi e poi spedisce indietro los tart
		read(pipedSingle[0], &process, sizeof(process));
		printf("FATHER: Received %d from %d.\n", process.timeSleep, process.id);
		write(pipedMultiple[process.id][1], &start, sizeof(start));
		
	}
	//stoppa tutti dopo i 20 secondi
	for(i = 0; i < k; i++){
		write(pipedMultiple[process.id][1], &stop, sizeof(stop));
	}	
	wait(NULL);

	return 0;
}
