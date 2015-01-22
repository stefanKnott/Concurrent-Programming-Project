

#define _GNU_SOURCE
#include <sched.h>

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



#define MUX 0
#define SEM 1
#define SPIN 2

pthread_mutex_t mutex;
pthread_cond_t start_cv, follow_cv;
pthread_spinlock_t spin;
sem_t semlock;

int count = 0;

void *mux_handler(){
	usleep(10);

	pthread_mutex_lock(&mutex);

	count = count + 1;

	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *spin_handler(){
	usleep(10);

	int ret;

	ret = pthread_spin_lock(&spin);
	if(ret != 0){
		fprintf(stderr, "Errot: return code from spin_lock %d\n", ret);
	}

	count = count + 1;

	pthread_spin_unlock(&spin);

	return NULL;
}

void *sem_handler(){
	usleep(10);

	//enter
	sem_wait(&semlock);

	count = count + 1;

	//exit
	if(sem_post(&semlock) == -1){
		fprintf(stderr, "Thread failed to unlock semaphore\n");
	}

	return NULL;
}

int main(int argc, char* argv[]){

	int num_threads, num_cores, lock_type;

	if(argc < 4){
		fprintf(stderr, "please use the following format: ./no_contention NUM_THREADS NUM_CORES LOCK_TYPE");
		exit(EXIT_FAILURE);
	}

	num_threads = atoi(argv[1]); //test 5, 50, 100 threads
	num_cores = atoi(argv[2]); //test 2, 4, 8 cores
	lock_type = atoi(argv[3]); //0 for mutex and cv, 1 for binary sempahore, 2 for spin lock

	pthread_t pool[num_threads];

	//set number of cores to run on
	cpu_set_t set;
	CPU_ZERO(&set);

	int i;
	for(i = 0; i < num_cores; i++){
		CPU_SET(i, &set);
	}
	sched_setaffinity(0, sizeof(cpu_set_t), &set);


	if(lock_type == MUX){

		//initialize mutex
		if(pthread_mutex_init(&mutex, NULL)){
			fprintf(stderr, "Error initializing mutex!");
		}

		int rc;

		//initialize thread pools
		for(i = 0; i < num_threads; i++){
			rc = pthread_create(&pool[i], NULL, mux_handler, NULL);
			if(rc){
				fprintf(stderr, "Error: return code from pthread_create() is %d\n", rc);
				exit(EXIT_FAILURE);
			}
		}
	}

	if(lock_type == SEM){
		int rc;

		//init semaphore
		rc = sem_init(&semlock, 0, 1);

		//initialize thread pools
		for(i = 0; i < num_threads; i++){
			rc = pthread_create(&pool[i], NULL, sem_handler, NULL);
			if(rc){
				fprintf(stderr, "Error: return code from pthread_create() is %d\n", rc);
				exit(EXIT_FAILURE);
			}
		}

	}

	if(lock_type == SPIN){
		int rc;

		//initialize thread pools
		for(i = 0; i < num_threads; i++){
			rc = pthread_create(&pool[i], NULL, spin_handler, NULL);
			if(rc){
				fprintf(stderr, "Error: return code from pthread_create() is %d\n", rc);
				exit(EXIT_FAILURE);
			}
		}
	}
	
	for(i = 0; i < num_threads; i++){
		pthread_join(pool[i], NULL);
	}
		
	assert(count == num_threads);
	
	return 0;
}