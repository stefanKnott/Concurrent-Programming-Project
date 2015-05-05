#ifndef THREADING_H
#define THREADING_H
#define _GNU_SOURCE
#include <sched.h>

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#define MUX 0
#define SEM 1
#define SPIN 2
#define TTASB 3
#define MAX_SLEEP 250


#endif
