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

volatile int ptrIn = 0;
volatile int ptrOut = 0;
volatile int counter = 0;

// Define Locks, Condition variables, and so on here
pthread_cond_t empty_Condition = PTHREAD_COND_INITIALIZER;	// Buffer condition
pthread_cond_t full_Condition = PTHREAD_COND_INITIALIZER;	// Buffer condition
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;	// Buffer lock

// Producer consumer data structures
counter_t prod;
counter_t cons;



// Bounded buffer put() get()
int put(Matrix * value)
{
	pthread_mutex_lock(&buffer_lock);
	if (value != NULL && counter < BOUNDED_BUFFER_SIZE) {
	    bigmatrix[ptrIn] = value;
	    ptrOut = ptrIn;
	    ptrIn = (ptrIn + 1) % BOUNDED_BUFFER_SIZE;
	    counter++;
	}
	pthread_mutex_unlock(&buffer_lock);
	return 0;


}

Matrix * get()
{
  Matrix * tmp;
  pthread_mutex_lock(&buffer_lock);
  if (counter > 0) {
    tmp = bigmatrix[ptrOut];
    ptrIn = ptrOut;
    ptrOut = (ptrOut - 1) % BOUNDED_BUFFER_SIZE;
    counter--;
  }
  pthread_mutex_unlock(&buffer_lock);

  return (tmp != NULL) ? tmp : NULL;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats * stats = (ProdConsStats*) arg;
  while (get_cnt(&prod) < NUMBER_OF_MATRICES) {
  	pthread_mutex_lock(&mutex);
	  // check buffer to get put data in wait if full_condition
	  while (counter == BOUNDED_BUFFER_SIZE) {
	   // Other producer finished the tstatset
	     if (get_cnt(&prod) >= NUMBER_OF_MATRICES) {
	     	pthread_cond_signal(&full_Condition);
	     	pthread_mutex_unlock(&mutex);
	     	return 0;
	     }
	     pthread_cond_wait(&empty_Condition, &mutex); // thread sleep

	  }
	  Matrix * mat = GenMatrixRandom();  // generate an NxM matrix
	  if (get_cnt(&prod) < NUMBER_OF_MATRICES) {  // update current counter
	  	put(mat);
	  	stats->sumtotal += SumMatrix(mat);
	  	stats->matrixtotal += 1;
	  	increment_cnt(&prod);
	  	pthread_cond_signal(&full_Condition);

	  }

  	pthread_mutex_unlock(&mutex);
  } pthread_cond_broadcast(&empty_Condition);  // wake up all sleeping threads


  return 0;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  ProdConsStats * stats = (ProdConsStats*) arg;
  while (get_cnt(&cons) < NUMBER_OF_MATRICES) {
  	pthread_mutex_lock(&mutex);
  	// check buffer to get data from m1, wait if empty_Condition
  	while (counter == 0) {
  		// Other consumer got data then job done
  		if (get_cnt(&cons) >= NUMBER_OF_MATRICES) {
  			pthread_cond_signal(&full_Condition);
  			pthread_mutex_unlock(&mutex);
  			return 0;

  		}
  		pthread_cond_broadcast(&empty_Condition);
  		pthread_cond_wait(&full_Condition, &mutex);

  	}

  	Matrix * m1 = get();
  	stats->sumtotal += SumMatrix(m1);
  	increment_cnt(&cons); // increment the consumer counter
  	stats->matrixtotal += 1;

  	// check buffer to get data from m2, wait if empty_Condition
  	while (counter == 0) {
  	// Other consumer got data then job done
  		if (get_cnt(&cons) >= NUMBER_OF_MATRICES || m1 == NULL) {
  			pthread_cond_signal(&full_Condition);
  			pthread_mutex_unlock(&mutex);
  			return 0;

  		}
  		pthread_cond_broadcast(&empty_Condition);
  		pthread_cond_wait(&full_Condition, &mutex);

  	}
  	Matrix * m2 = NULL;
  	Matrix * m3;

  	// keep fetch data for m2 matrix until it finds one compatible with m1
  	// m1 cols must match m2 rows
  	do {
  		// check buffer to get data for m2, wait if empty_Condition
  		while (counter == 0) {
  			// Other consumer got data then job done
  			if (get_cnt(&cons) >= NUMBER_OF_MATRICES || m2 == NULL) {
	  			pthread_cond_signal(&full_Condition);
  				pthread_mutex_unlock(&mutex);
  				return 0;
  			}
  			pthread_cond_broadcast(&empty_Condition);
  			pthread_cond_wait(&full_Condition, &mutex);

  		}

  		// proceed with the calculation with the newly acquired m2
  		if (m2 != NULL) FreeMatrix(m2);
  		m2 = get();
  		increment_cnt(&cons);
  		stats->sumtotal += SumMatrix(m2);
  		stats->matrixtotal += 1;
  		m3 = MatrixMultiply(m1, m2);

  	} while (m3 == NULL);

      DisplayMatrix(m1,stdout);
      printf("    X\n");
      DisplayMatrix(m2,stdout);
      printf("    =\n");
      DisplayMatrix(m3,stdout);
      printf("\n");

      FreeMatrix(m3);
      FreeMatrix(m2);
      FreeMatrix(m1);

      stats->multtotal += 1;
      pthread_cond_signal(&empty_Condition);
      pthread_mutex_unlock(&mutex);

  } pthread_cond_broadcast(&full_Condition);  // again, wake up sleeping process, job done



}
