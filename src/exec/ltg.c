#include <math.h>

#include "algo/array.h"
#include "config.h"
#include "network.h"
#include "util/log.h"

#include "exec.h"
#include "exec/ltg.h"

#define TO_LTG(e) struct exec_ltg_t *ltg = (struct exec_ltg_t *)(e);

struct mop_t *_ltg_next_mop(
    struct exec_t *exec, struct expr_t *expr) {
  return 0;
}

static int _cp_cmp(void const *v1, void const *v2) {
  struct exec_critical_path_t *t1 = (struct exec_critical_path_t *)v1;
  struct exec_critical_path_t *t2 = (struct exec_critical_path_t *)v2;

  bw_t b1 = t1->bandwidth / t1->num_switches;
  bw_t b2 = t2->bandwidth / t2->num_switches;

  if       (b1 > b2) return -1;
  else if  (b1 < b2) return  1;

  return 0;
}

static void
_exec_ltg_validate(struct exec_t *exec, struct expr_t const *expr) {
  TO_LTG(exec);

  unsigned subplan_count = 0;
  ltg->trace = traffic_matrix_trace_load(400, expr->traffic_test);
  if (ltg->trace == 0)
    panic("Couldn't load the traffic matrix file: %s", expr->traffic_test);

  ltg->steady_packet_loss = exec_rvar_cache_load(expr, &subplan_count);
  if (ltg->steady_packet_loss == 0)
    panic_txt("Couldn't load the long-term RVAR cache.");

  if (expr->criteria_time == 0)
    panic_txt("Time criteria not set.");

  struct traffic_matrix_trace_iter_t *iter =
    ltg->trace->iter(ltg->trace);

  ltg->plan = exec_critical_path_analysis(exec, expr, iter, iter->length(iter));
}

// Creates fixed plans by distributing the upgrade as best as it can over the
// upgrade interval.
static risk_cost_t _exec_ltg_best_plan_at(
    struct exec_t *exec,
    struct expr_t const *expr,
    trace_time_t at) {
  // LTG doesn't really care about the best_plan_cost
  // Pack as many subplans as you can 
  TO_LTG(exec);

  struct exec_critical_path_stats_t *plan = ltg->plan;
  qsort(plan->paths, plan->num_paths, sizeof(struct exec_critical_path_t), _cp_cmp);


  struct jupiter_located_switch_t **sws = malloc(
      sizeof(struct jupiter_located_switch_t *) * expr->nlocated_switches);

  unsigned nsteps = expr->criteria_time->steps;
  struct mop_t **mops = malloc(
      sizeof(struct mop_t *) * nsteps);

  // TODO: This should somehow use expr->criteria_time->acceptable
  for (uint32_t i = 0; i < nsteps; ++i) {
    unsigned idx = 0;
    for (uint32_t j = 0; j < plan->num_paths; ++j) {
      int nsws = ceil(((double)plan->paths[j].num_switches)/(double)nsteps);
      for (uint32_t k = 0; k < nsws; ++k) {
        sws[idx++] = plan->paths[j].sws[k];
      }
    } 

    mops[i] = jupiter_mop_for(sws, idx);
#if DEBUG_MODE
    char *exp = mops[i]->explain(mops[i], expr->network);
    info("MOp explanation: %s", exp);
    free(exp);
#endif
  }

  risk_cost_t cost = exec_plan_cost(exec, expr, mops, nsteps, at);
  free(sws);

  return cost;
}

static struct exec_output_t *
_exec_ltg_runner(struct exec_t *exec, struct expr_t const *expr) {
  struct exec_output_t *res = malloc(sizeof(struct exec_output_t));
  struct exec_result_t result = {0};
  res->result = array_create(sizeof(struct exec_result_t), 10);

  for (uint32_t i = expr->scenario.time_begin; i < expr->scenario.time_end; i += expr->scenario.time_step) {
    trace_time_t at = i;
    risk_cost_t actual_cost = _exec_ltg_best_plan_at(exec, expr, at);
    info("[%4d] Actual cost of the best plan (%02d) is: %4.3f : %4.3f",
        at, expr->criteria_time->steps, actual_cost, actual_cost);

    result.cost = actual_cost;
    result.description = 0;
    result.num_steps = expr->criteria_time->steps;
    result.at = i;

    array_append(res->result, &result);
  }

  return res;
}

static void
_exec_ltg_explain(struct exec_t const *exec) {
  text_block_txt(
       "LTG divides an upgrade plan over the available planning intervals.\n"
       "The length of the planning interval is set through the criteria-time\n"
       " in the .ini file.\n");
}

struct exec_t *exec_ltg_create(void) {
  struct exec_t *exec = malloc(sizeof(struct exec_ltg_t));
  exec->net_dp = 0;

  exec->validate = _exec_ltg_validate;
  exec->run = _exec_ltg_runner;
  exec->explain = _exec_ltg_explain;

  return exec;
}
