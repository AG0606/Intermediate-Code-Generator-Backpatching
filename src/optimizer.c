#include "optimizer.h"
#include "cfg.h"

OptimizationStats opt_stats;

static bool is_integer_constant(const char *str, int *val) {
    if (!str || *str == '\0' || strcmp(str, "-") == 0) return false;
    char *endptr;
    long v = strtol(str, &endptr, 10);
    if (*endptr == '\0') {
        if (val) *val = (int)v;
        return true;
    }
    return false;
}

/* Pass 1: Constant Folding */
static void pass_constant_folding(void) {
    for (int i = 0; i < nextquad; i++) {
        const char *op = quads[i].op;
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
            strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) {
            int v1, v2;
            if (is_integer_constant(quads[i].arg1, &v1) && is_integer_constant(quads[i].arg2, &v2)) {
                int res = 0;
                bool valid = true;
                if (strcmp(op, "+") == 0) res = v1 + v2;
                else if (strcmp(op, "-") == 0) res = v1 - v2;
                else if (strcmp(op, "*") == 0) res = v1 * v2;
                else if (strcmp(op, "/") == 0) {
                    if (v2 != 0) res = v1 / v2;
                    else valid = false;
                }
                else if (strcmp(op, "%") == 0) {
                    if (v2 != 0) res = v1 % v2;
                    else valid = false;
                }

                if (valid) {
                    char res_str[32];
                    sprintf(res_str, "%d", res);
                    strcpy(quads[i].op, "=");
                    strcpy(quads[i].arg1, res_str);
                    strcpy(quads[i].arg2, "-");
                    opt_stats.constants_folded++;
                    if (opt_trace) {
                        printf("[OPTIMIZER] Folded constant at quad %d: %s %s %s -> %s\n",
                               i, quads[i].arg1, op, quads[i].arg2, res_str);
                    }
                }
            }
        }
    }
}

/* Pass 2: Jump-to-Jump Chaining */
static void pass_jump_chaining(void) {
    bool changed = true;
    int pass = 0;
    while (changed && pass < 50) {
        changed = false;
        pass++;
        for (int i = 0; i < nextquad; i++) {
            if (strcmp(quads[i].op, "goto") == 0 || strcmp(quads[i].op, "if") == 0) {
                int target = atoi(quads[i].result);
                if (target >= 0 && target < nextquad && target != i) {
                    /* If target is an unconditional goto to another target */
                    if (strcmp(quads[target].op, "goto") == 0) {
                        int next_target = atoi(quads[target].result);
                        if (next_target >= 0 && next_target < nextquad && next_target != target) {
                            char new_res[32];
                            sprintf(new_res, "%d", next_target);
                            strcpy(quads[i].result, new_res);
                            opt_stats.jumps_chained++;
                            changed = true;
                            if (opt_trace) {
                                printf("[OPTIMIZER] Jump chained: Quad %d target %d -> %d\n",
                                       i, target, next_target);
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Pass 3: Redundant Jump Elimination and Dead Code Elimination */
static void pass_dead_code_elimination(void) {
    /* Identify reachability */
    cfg_build();

    bool keep[MAX_QUADS] = { false };

    /* Mark all quads belonging to reachable blocks as kept */
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        if (current_cfg.blocks[b].is_reachable) {
            for (int q = current_cfg.blocks[b].start_quad; q <= current_cfg.blocks[b].end_quad; q++) {
                keep[q] = true;
            }
        }
    }

    /* Redundant jump elimination: if an unconditional goto jumps to the next quad */
    for (int i = 0; i < nextquad; i++) {
        if (keep[i] && strcmp(quads[i].op, "goto") == 0) {
            int target = atoi(quads[i].result);
            if (target == i + 1) {
                keep[i] = false; /* Redundant jump! */
                opt_stats.redundant_jumps_removed++;
                if (opt_trace) {
                    printf("[OPTIMIZER] Removed redundant goto next at quad %d\n", i);
                }
            }
        }
    }

    /* Count dead quads */
    for (int i = 0; i < nextquad; i++) {
        if (!keep[i]) {
            opt_stats.dead_quads_removed++;
        }
    }

    /* Remap jump targets */
    int new_index[MAX_QUADS];
    int compact_count = 0;
    for (int i = 0; i < nextquad; i++) {
        if (keep[i]) {
            new_index[i] = compact_count++;
        } else {
            new_index[i] = -1;
        }
    }

    /* For quads that are removed, their target maps to the next valid quad */
    for (int i = nextquad - 1; i >= 0; i--) {
        if (new_index[i] == -1) {
            if (i + 1 < nextquad) {
                new_index[i] = new_index[i + 1];
            } else {
                new_index[i] = compact_count;
            }
        }
    }

    /* Create compacted quad array */
    struct quad_entry compacted[MAX_QUADS];
    for (int i = 0; i < nextquad; i++) {
        if (keep[i]) {
            int ni = new_index[i];
            compacted[ni] = quads[i];
            if (strcmp(compacted[ni].op, "goto") == 0 || strcmp(compacted[ni].op, "if") == 0) {
                int old_t = atoi(compacted[ni].result);
                if (old_t >= 0 && old_t < nextquad) {
                    int new_t = new_index[old_t];
                    sprintf(compacted[ni].result, "%d", new_t);
                }
            }
        }
    }

    /* Copy back */
    for (int i = 0; i < compact_count; i++) {
        quads[i] = compacted[i];
    }
    nextquad = compact_count;
}

void optimize_quads(void) {
    opt_stats.initial_quad_count = nextquad;
    opt_stats.final_quad_count = 0;
    opt_stats.jumps_chained = 0;
    opt_stats.redundant_jumps_removed = 0;
    opt_stats.dead_quads_removed = 0;
    opt_stats.constants_folded = 0;

    pass_constant_folding();
    pass_jump_chaining();
    pass_dead_code_elimination();

    opt_stats.final_quad_count = nextquad;
}

void print_optimization_report(FILE *fp) {
    fprintf(fp, "\n=========================================================================\n");
    fprintf(fp, "                     INTERMEDIATE CODE OPTIMIZATION REPORT               \n");
    fprintf(fp, "=========================================================================\n");
    fprintf(fp, "  Initial Quad Count       : %d\n", opt_stats.initial_quad_count);
    fprintf(fp, "  Final Quad Count         : %d\n", opt_stats.final_quad_count);
    int reduction = opt_stats.initial_quad_count - opt_stats.final_quad_count;
    double pct = (opt_stats.initial_quad_count > 0)
        ? ((double)reduction / opt_stats.initial_quad_count * 100.0) : 0.0;
    fprintf(fp, "  Code Size Reduction      : %d quads (%.1f%%)\n", reduction, pct);
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    fprintf(fp, "  Optimization Details:\n");
    fprintf(fp, "    - Constants Folded     : %d\n", opt_stats.constants_folded);
    fprintf(fp, "    - Jumps Chained        : %d\n", opt_stats.jumps_chained);
    fprintf(fp, "    - Redundant Jumps Cut  : %d\n", opt_stats.redundant_jumps_removed);
    fprintf(fp, "    - Dead Quads Eliminated: %d\n", opt_stats.dead_quads_removed);
    fprintf(fp, "=========================================================================\n\n");
}
