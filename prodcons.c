/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"
#include <assert.h>

// Define Locks, Condition variables, and so on here

int buffer;
int count = 0;
pthread_cond_t empty, full;
pthread_mutex_t mutex;

// Bounded buffer put() get()
int put(Matrix * value)
{
	assert(count == 0);
	count = 1;
	buffer = value;
	return 0;
}

Matrix * get()
{
	assert(count == 1);
	count = 0;
  return buffer;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
	//pthread_mutex_init(&mutex, NULL);
	int i;
	for (i = 0; i < LOOPS; i++) {
		pthread_mutex_lock(&mutex);
		while (count == 1)
			pthread_cond_wait(&empty, &mutex);
		put(i);
		pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);
	}
	//pthread_mutex_destroy(&mutex);
  return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
	int i;
	for (i = 0; i < LOOPS; i++) {
		pthread_mutex_lock(&mutex);
		while (count == 0)
			pthread_cond_wait(&full, &mutex);
		int tmp = get();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		printf("%d\n", tmp);
	}
  return NULL;
}
