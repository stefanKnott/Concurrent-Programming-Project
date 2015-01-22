



#include <sched.h>
#include <pthread.h>
#include <assert.h>

#define MUX 0
#define SEM 1
#define SPIN 2

pthread mutex_t mutex;
pthread cond_t start_cv, follow_cv;
pthread_spinlock_t spin;
int pshared;
int count = 0;
int start_turn = 1;

void *start_mutex(){
	pthread_mutex_lock(&mutex, NULL);

	while(!start_turn){
		pthread_cond_wait(&start_cv, NULL);
	}

	count = count + 1;
	start_turn = 0;

	pthrad_cond_signal(&follow_cv, NULL);

	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *follow_mutex(){
	pthread_mutex_lock(&mutex, NULL);

	while(start_turn){
		pthread_cond_wait(&follow_cv, NULL);
	}

	count = count + 1;
	start_turn = 1;

	pthrad_cond_signal(&start_cv, NULL);

	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *start_spin(){
	int ret;

	ret = pthread_spin_lock(&spin);

	while(!start_turn){
		ret = pthread_spin_unlock(&spin);
	}

	count = count + 1;
	start_turn = 0;

	pthread_spin_unlock(&spin);

	return NULL;
}

void *follow_spin(){
	int ret;

	ret = pthread_spin_lock(&spin);

	while(!start_turn){
		ret = pthread_spin_unlock(&spin);
	}

	count = count + 1;
	start_turn = 0;

	pthread_spin_unlock(&spin);

	return NULL;
}

int main(int argc, char* argv[]){

	int num_threads, num_cores, lock_type;

	if(argc < 4){
		fprintf(stderr, "please use the following format: ./no_contention NUM_THREADS NUM_CORES LOCK_TYPE");
		exit(EXIT_FAILURE);
	}

	num_threads = argv[1]; //test 5, 50, 100 threads
	num_cores = argv[2]; //test 2, 4, 8 cores
	lock_type = argv[3]; //0 for mutex and cv, 1 for binary sempahore, 2 for spin lock

	pthread_t start_pool[num_threads];
	pthread_t follow_pool[num_threads];

	//set number of cores to run on
	cpu_set_t set;
	CPU_ZERO(&set);
	for(i = 0; i < num_cores; i++){
		CPU_SET(i, &set);
	}
	sched_setaffinity(0, sizeof(cpu_set_t), &set);


	if(lock_type = MUX){
		if(pthread_mutex_init(&mutex, NULL){
			fprintf(stderr, "Error initializing mutex!");
		}
		if(pthread_cond_init(&cv, NULL)){
			fprintf(stderr, "Error initializing condition variable!")
		}

		int i;
		int rc;
		//initialize thread pools
		for(i = 0; i < num_threads; i++){
			rc = pthread_create(&start_pool[i], NULL, start_mux, NULL);
			if(rc){
				fprintf(stderr, "Error: return code from pthread_create() is %d\n", rc);
				exit(EXIT_FAILURE);
			}

			rc = pthread_create(&follow_pool[i], NULL, follow_mux, NULL);
			if(rc){
				fprintf(stderr, "Error: return code from pthread_create() is %d\n", rc);
				exit(EXIT_FAILURE);
			}
		}

	if(lock_type = SEM){
		//call semaphroe implementations
	}

	if(lock_type = SPIN){
		//call spin lock implementations
	}


		printf("%d", count);
		assert(count == 2 * num_threads);
	}

}