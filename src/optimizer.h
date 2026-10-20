#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "quad.h"

typedef struct {
    int initial_quad_count;
    int final_quad_count;
    int jumps_chained;
    int redundant_jumps_removed;
    int dead_quads_removed;
    int constants_folded;
} OptimizationStats;

extern OptimizationStats opt_stats;

/* Optimization passes */
void optimize_quads(void);
void print_optimization_report(FILE *fp);

#endif /* OPTIMIZER_H */
