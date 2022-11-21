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

//int buffer;
Matrix** bigmatrix;
int count = 0;
pthread_cond_t empty, full;
pthread_mutex_t mutex;
// struct prodcons* stats = (struct prodcons*)malloc(sizeof(struct prodcons));
struct prodcons* stats = {0};

// Bounded buffer put() get()
int put(Matrix * value)
{
	assert(count <= BOUNDED_BUFFER_SIZE);
	bigmatrix[count] = value;
	count++;
	return 0;
}

Matrix * get()
{
	assert(count > 0);
	Matrix* tmp = bigmatrix[count];
	count--;
  return tmp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
	//pthread_mutex_init(&mutex, NULL);
	int i;
	Matrix* bm;
	printf("Befor entering forloop pr\n");
	for (i = 0; i < LOOPS; i++) {
		bm = GenMatrixRandom();
		printf("Generated random matrix\n");
		pthread_mutex_lock(&mutex);
		put(bm);
		printf("Put bm into buffer\n");
		// count++;
		pthread_cond_signal(&full);
		while (count == BOUNDED_BUFFER_SIZE) {
			pthread_cond_wait(&empty, &mutex);
		}
		stats->sumtotal += SumMatrix(bm);
		pthread_mutex_unlock(&mutex);
	}
	//pthread_mutex_destroy(&mutex);
  return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
	int i;
	printf("Before entering cons worker forloop\n");
	for (i = 0; i < LOOPS; i++) {
		pthread_mutex_lock(&mutex);
		while (count < 2) {
			printf("In consumer while loop\n");
			pthread_cond_wait(&full, &mutex);
		}
		Matrix *m1, *m2, *m3;
		m1 = get();
		printf("First get cons\n");
		// count--;
	  m2 = get();
		printf("Second get cons\n");
		// count--;
	  m3 = MatrixMultiply(m1, m2);
		printf("Matrix Multiplcation cons\n");
	  if (m3 != NULL)
	  {
	    DisplayMatrix(m1,stdout);
	    printf("    X\n");
	    DisplayMatrix(m2,stdout);
	    printf("    =\n");
	    DisplayMatrix(m3,stdout);
	    printf("\n");

			pthread_cond_signal(&empty);
			pthread_mutex_unlock(&mutex);
			// printf("%d\n", tmp);
			stats->sumtotal += SumMatrix(m1);
			stats->sumtotal += SumMatrix(m2);
			stats->sumtotal += SumMatrix(m3);
			FreeMatrix(m3);
			FreeMatrix(m2);
			FreeMatrix(m1);
			m1=NULL;
			m2=NULL;
			m3=NULL;
		}
	}
	return NULL;
}
