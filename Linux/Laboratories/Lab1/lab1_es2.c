#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]){
	
	
	if (argc < 3){
		perror("Not enough arguments");
		return 1;
	}
	
	int 		num_process = atoi(argv[1]);
	int 		i;
	size_t 		len;
	char		*dir = argv[2];
	char 		systemArg[256];
	char		*line = NULL;
	FILE		*fp;
	pid_t		pid, wpid;
	int 		status;
	
	snprintf(systemArg, sizeof(systemArg), "find %s -type f -printf \"%%f\n\" > list.txt", dir); 
	system(systemArg);
	
	i = 0;
		
	chdir(dir);
	
	fp = fopen("../list.txt", "r+");
	
	while ((getline(&line, &len, fp)) != 1){
		line[strcspn(line, "\n")] = 0;	
		printf("line is %s\n", line);
		if ((pid = fork()) == 0){
			printf("pid = %d i = %d\n", getpid(), i);
			execlp("sort", "sort", "-n", "-o", line, line, (char*)0);
			perror("execlp failed.\n");
		}else if (pid > 0) {
			i++;
			if (i == num_process - 1){
				printf("Now i is %d\n", i);
				while ((wpid = wait(&status)) > 0);
				i = 0;
			}
			
		}
		
	}
	
		
	
	
	return 0;
	
}
