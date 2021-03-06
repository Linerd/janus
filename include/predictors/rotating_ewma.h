#ifndef _PREDICTORS_ROTATING_EWMA_H_
#define _PREDICTORS_ROTATING_EWMA_H_

#include "predictor.h"

/* TODO: Fix the EWMA implementation.  Right now EWMA calculation is messed up
 * (it is correct under the framework that it's defined) however it does not
 * match the previous implementation the framework.
 *
 * - Omid 03/27/2019
 * */
struct predictor_rotating_ewma_iterator_t {
  struct predictor_iterator_t; /* ewma_iterator is a predictor iterator */

  // Book keeping
  trace_time_t pos; /* Internal variable keeping track of number of samples returned so far. */
  struct traffic_matrix_t *tm_now; /* Keeps a copy of the last TM observed */
  struct traffic_matrix_trace_iter_t *iter; /* A "sample" */

  unsigned num_samples; /* Number of samples requested */
};

struct predictor_rotating_ewma_t {
  struct predictor_t;

  // Prediction trace
  struct traffic_matrix_trace_t **pred_traces;

  // Error trace is the trace after prediction
  struct traffic_matrix_trace_t **error_traces;

  // Real trace upon which the model is built
  struct traffic_matrix_trace_t *real_trace;

  const char *prefix;

  // Exponential coeff of backing off
  bw_t exp_coeff;

  // How many steps does the exp back off run
  uint16_t steps;

  // Num samples to return
  uint32_t sample;
};

void predictor_rotating_ewma_build
  (struct predictor_t *, struct traffic_matrix_trace_t *);

/*
 * @param exp_coeff: Exponential smoothing factor
 * @param steps: # of steps into the future to predict
 * @param name: path of the file where we save the error matrices
 * */
struct predictor_rotating_ewma_t *predictor_rotating_ewma_create(
    bw_t coeff, uint16_t steps, const char *loc);

void predictor_rotating_ewma_save(struct predictor_t *);
void predictor_rotating_ewma_free(struct predictor_t *);

/*
 * Directory of the ewma cache files
 * Prefix used for the cache files
 * Number of steps predicted using the EWMA predictor
 * Number of TM caches to keep for the EWMA predictor
 * And the real traffic matrix trace
 */
struct predictor_rotating_ewma_t *
  predictor_rotating_ewma_load(
      char const *, char const *, unsigned, 
      unsigned, struct traffic_matrix_trace_t *);

#endif // _PREDICTORS_EWMA_H_
