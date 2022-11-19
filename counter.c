/*
 *  Synchronized counter routines
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Include libraries required for this module only
#include <stdio.h>
#include <pthread.h>
#include "counter.h"

// SYNCHRONIZED COUNTER METHOD IMPLEMENTATION
// Based on Three Easy Pieces

void init_cnt(counter_t *c)  {
  c->value = 0;
  pthread_mutex_init(&c->lock, NULL);
}

void increment_cnt(counter_t *c)  {
  pthread_mutex_lock(&c->lock);
  c->value++;
  pthread_mutex_unlock(&c->lock);
}

int get_cnt(counter_t *c)  {
  pthread_mutex_lock(&c->lock);
  int rc = c->value;
  pthread_mutex_unlock(&c->lock);
  return rc;
}
