
#include "threading.h"
//#include <sys/atomic_op.h>

pthread_mutex_t mutex;
sem_t semaphore;
pthread_spinlock_t spinlock;
int pshared, lock_type, contention_level;
int total_count = 0;

int backoff = 1;
int TTASLock = 0;


//taken from http://atechie.net/tag/compare-and-swap/
//CAS in assembly..edited by: Stefan Knott
int cas(int* dest, int oldvalue, int newvalue){  
    int result = 0;/* 1 shows that cas succeeded and 0 shows that it failed  */  
    __asm__ __volatile__(  
            "movl %2, %%eax\n\t"  
            "movl %3, %%ebx\n\t"  
            "movl %0, %%ecx\n\t"  
            "LOCK\n\t"  
            "CMPXCHG %%ebx, (%%ecx)\n\t"  /* should LOCK be on the same line  */  
            "jz DONE\n\t"  
            "movl $0, %1\n\t "  
            "DONE: \n\t"  
            :"=m"(dest),"=g"(result)  
            :"g" (oldvalue),"g" (newvalue),"m"(dest)  
            :"%eax","%ebx","ecx","cc"  
            );  
    if(*dest == newvalue){//cas succeeded
    	result = 1;
    }
    return result;   
}  

//TTAS with exponential backoff
void lock(){
	while(1){
		while(__sync_add_and_fetch(&TTASLock, 0)){//lock is busy -- handle backoff
			__sync_add_and_fetch(&backoff, backoff * 2);//increase backoff time
			if(backoff >= MAX_SLEEP){
				cas(&backoff, backoff, 1);//reset backoff back to 1
			}
			usleep(backoff);
		}//reading to see if lock is busy
			if(cas(&TTASLock, 0, 1)){ //lock acheived
				return;
			}
	}
}

void unlock(){
	cas(&TTASLock, 1, 0); //set lock state to logical false
}


void *runnable(){
	FILE *hp2;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int local_cnt;

	hp2 = fopen("HP2.txt", "r");

	while((read = getline(&line, &len, hp2)) != -1){
		local_cnt = local_cnt + 1; //one line was read
		if(local_cnt % contention_level == 0){
			switch(lock_type){
				case MUX:
					pthread_mutex_lock(&mutex);
						int i;
						for(i = 0; i < 250; i++){
						total_count = total_count + 1;
					}
					pthread_mutex_unlock(&mutex);
					break;

				case SEM:
					sem_wait(&semaphore);
					for(i = 0; i < 250; i++){
						total_count = total_count + 1;
					}
					sem_post(&semaphore);
					break;

				case SPIN:
					pthread_spin_lock(&spinlock);
					for(i = 0; i < 250; i++){
						total_count = total_count + 1;
					}
					pthread_spin_unlock(&spinlock);
					break;

				case TTASB:
					lock();
					for(i = 0; i < 250; i++){
						total_count = total_count + 1;
					}
					unlock();
					break;

				default:
					break;
			}
		}
	}
	return NULL;

}

int main(int argc, char* argv[]){
	int num_threads, num_cores;

	if(argc < 5){
		fprintf(stderr, "PLEASE ENTER 4 ARGUMENTS: NUM_THREADS NUM_CORES LOCK_TYPE CONTENTION_LEVEL");
	}
	num_threads = atoi(argv[1]);
	num_cores = atoi(argv[2]);
	lock_type = atoi(argv[3]);
	contention_level = atoi(argv[4]);

	pthread_t threadPool[num_threads];

	//SET PROCESSORS TO RUN ON
	cpu_set_t mask;
	CPU_ZERO(&mask);
	int i;
	for(i = 0; i < num_cores; i++){
		CPU_SET(i, &mask);
	}

	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		fprintf(stderr, "Error setting CPU affinity");
	}

	//INIT LOCKING TYPE
	switch(lock_type){
		case MUX:
			pthread_mutex_init(&mutex, NULL);
			break;
		case SEM:
			pshared = 0;
			sem_init(&semaphore, pshared, 1);
			break;

		case SPIN:
			pthread_spin_init(&spinlock, 0);
			break;

		case TTASB:
			break;

		default:
			fprintf(stderr, "Please choose 1 for MUX, 2 for SEMAPHORE");
			break;
	}

	int rc;
	for(i = 0; i < num_threads; i++){
		rc = pthread_create(&(threadPool[i]), NULL, runnable, NULL);
		if(rc){
			printf("ERROR: return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}

	for(i = 0; i < num_threads; i++){
		pthread_join(threadPool[i], NULL);
	}

	return 0;

}
