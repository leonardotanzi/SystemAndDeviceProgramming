#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]){
	
	char 		tmpName[9];
	int 		fd;
	char 		folder[] = "/home/leo/Desktop/lab1";
	int 		i, j;
	char 		buffer[256] = {};
	int 		ran;
	
	srand(time(NULL));
	
	if (argc < 3){
		perror("Not enough arguments.\n");
		return 1;
	}
	
	mkdir(folder, 0700);
	chdir(folder);
	
	for (i = 0; i < atoi(argv[1]); i++){
		if (i < 10){
			snprintf(tmpName, sizeof(tmpName),"f0%d.txt", i);
		}else{
			snprintf(tmpName, sizeof(tmpName),"f%d.txt", i);
		}	
		for (j = 0; j < rand() % 20; j++){
			fd = open(tmpName, O_RDWR | O_CREAT | O_APPEND, 0777);
			ran = rand()%(atoi(argv[2]));
			snprintf(buffer, sizeof(buffer), "%d\n", ran);
			write(fd, buffer, strlen(buffer)); 
			close(fd);
		}
	}
	
	return 0;
}
