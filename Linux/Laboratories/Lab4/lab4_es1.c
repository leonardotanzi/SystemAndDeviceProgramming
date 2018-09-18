#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>

pthread_t *threads;
int *values;
int counter; 
pthread_mutex_t lock;

typedef struct {
	int left, right, n_threads;
}DataVec;

int counting(){
	int count;
	pthread_mutex_lock(&lock); 
	counter += 2;
	count = counter; //We do this just in case that the value of counter changes after the unlock.
	pthread_mutex_unlock(&lock);
	return count;
}

void printvector(int v[], int l){
	int i;
	for(i = 0; i < l; i++){
		printf("%d ", v[i]);
	}
}

void swap(int i, int j){
	int tmp;
	tmp = values[i];
	values[i] = values[j];
	values[j] = tmp;
}


void *quicksort (void * v) {
	DataVec *vec = (DataVec*) v;
	DataVec prov1, prov2;
	int i, j, x, count;
	if (vec->left >= vec->right) return;
	x = values[vec->left];
	i = vec->left - 1;
	j = vec->right + 1;
	
	while (i < j) {
		while (values[--j] > x);
		while (values[++i] < x);
		if (i < j)
			swap (i,j);
	}
	
	count = counting();
	prov1.left = vec->left;
	prov1.right = j;
	prov1.n_threads = vec->n_threads;	
		
	prov2.left = j + 1;
	prov2.right = vec->right;
	prov2.n_threads = vec->n_threads;
		
	if (count <= vec->n_threads && count - 1 != vec->n_threads){ 
		//Using two threads.
		pthread_create(&threads[count-1], NULL, quicksort, (void*)&prov1);
		pthread_create(&threads[count], NULL, quicksort, (void*)&prov2);
		pthread_join(threads[count-1], NULL);
		pthread_join(threads[count], NULL);
			
	}else if(count > vec->n_threads && count - 1 == vec->n_threads){
		//Using one thread.
		pthread_create(&threads[count-1], NULL, quicksort, (void*)&prov1);
		pthread_join(threads[count-1], NULL);
		quicksort((void*)&prov2);
			
	}else{
			//Reached the maximum number of threads.
		quicksort((void*)&prov1);
		quicksort((void*)&prov2);
	}
}
	
int main(int argc, char * argv[]){
	
	int size, rc;
	DataVec vec;
	
	if (argc < 2 ){
		perror("Not enough arguments.\n");
		exit(-1);
	}
	
	int fd, len, pg;
	struct stat stat_buf;

	if ((fd = open (argv[1], O_RDWR)) == -1){
		perror ("open");
	}

	if (fstat (fd, &stat_buf) == -1){
		perror ("fstat");
	}
	len = stat_buf.st_size;

	pg = getpagesize ();
	printf ("Page size %d\n", pg);
	printf ("File size in byte %d\n", len);
	//len = len + pg - (len % pg);   se non tolgo questo il file viene mappato su 4096 che è la grandezza della pagina
	//								 e i posti vuoti vengono riempiti con zero
	printf ("File size rounded to page size %d\n", len);

	values = mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (values == MAP_FAILED){
		perror ("mmap");
	}
	
	pthread_mutex_init(&lock, NULL);
	
	size = atoi(argv[1]);
	counter = 0;
	threads = malloc(size * sizeof(pthread_t));
	
	printf("List before sorting: \n");
	printvector(values, len / 4);
	printf("\n");
	
	counter = 0;
	vec.left = 0;
	vec.right = (len / 4) - 1;
	vec.n_threads = size;
	
	if ((rc = pthread_create(&threads[0], NULL, quicksort, (void*)&vec)) < 0){
		perror("Thread error.\n");
		exit(-1);
	}
	pthread_join(threads[0], NULL);	
	
	printf("List after sorting:\n");
	printvector(values, len / 4);
	printf("\n");
	
	pthread_mutex_destroy(&lock);
	close (fd);
	
	return 0;

}
