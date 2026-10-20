#include "quad.h"

struct quad_entry quads[MAX_QUADS];
int nextquad = 0;
int temp_count = 0;

char* newtemp(void) {
    static char buf[32];
    sprintf(buf, "t%d", ++temp_count);
    return strdup(buf);
}

int emit(const char *op, const char *arg1, const char *arg2, const char *result) {
    if (nextquad >= MAX_QUADS) {
        fprintf(stderr, "Error: Maximum quadruple limit exceeded (%d)\n", MAX_QUADS);
        exit(1);
    }
    int cur = nextquad;
    strncpy(quads[cur].op, op ? op : "-", sizeof(quads[cur].op) - 1);
    quads[cur].op[sizeof(quads[cur].op) - 1] = '\0';

    strncpy(quads[cur].arg1, arg1 ? arg1 : "-", sizeof(quads[cur].arg1) - 1);
    quads[cur].arg1[sizeof(quads[cur].arg1) - 1] = '\0';

    strncpy(quads[cur].arg2, arg2 ? arg2 : "-", sizeof(quads[cur].arg2) - 1);
    quads[cur].arg2[sizeof(quads[cur].arg2) - 1] = '\0';

    strncpy(quads[cur].result, result ? result : "-", sizeof(quads[cur].result) - 1);
    quads[cur].result[sizeof(quads[cur].result) - 1] = '\0';

    if (opt_trace) {
        printf("[TRACE EMIT] Quad %2d: op='%s', arg1='%s', arg2='%s', result='%s'\n",
               cur, quads[cur].op, quads[cur].arg1, quads[cur].arg2, quads[cur].result);
    }

    nextquad++;
    return cur;
}

struct list* makelist(int q) {
    struct list *node = (struct list*)malloc(sizeof(struct list));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in makelist\n");
        exit(1);
    }
    node->quad = q;
    node->next = NULL;
    return node;
}

struct list* merge(struct list *p1, struct list *p2) {
    if (!p1) return p2;
    if (!p2) return p1;
    struct list *curr = p1;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = p2;
    return p1;
}

void backpatch(struct list *p, int target) {
    char target_str[16];
    sprintf(target_str, "%d", target);
    struct list *curr = p;
    while (curr) {
        if (curr->quad >= 0 && curr->quad < nextquad) {
            if (opt_trace) {
                printf("[TRACE BACKPATCH] Patching Quad %d (was '%s') -> target '%s'\n",
                       curr->quad, quads[curr->quad].result, target_str);
            }
            strncpy(quads[curr->quad].result, target_str, sizeof(quads[curr->quad].result) - 1);
            quads[curr->quad].result[sizeof(quads[curr->quad].result) - 1] = '\0';
        }
        curr = curr->next;
    }
}

void free_list(struct list *p) {
    while (p) {
        struct list *tmp = p->next;
        free(p);
        p = tmp;
    }
}

void print_quads_to_stream(FILE *fp) {
    for (int i = 0; i < nextquad; i++) {
        if (strcmp(quads[i].op, "=") == 0) {
            fprintf(fp, "%2d: %s = %s\n", i, quads[i].result, quads[i].arg1);
        }
        else if (strcmp(quads[i].op, "goto") == 0) {
            fprintf(fp, "%2d: goto %s\n", i, quads[i].result);
        }
        else if (strcmp(quads[i].op, "if") == 0) {
            fprintf(fp, "%2d: if %s goto %s\n", i, quads[i].arg1, quads[i].result);
        }
        else if (strcmp(quads[i].op, "end") == 0) {
            fprintf(fp, "%2d: end\n", i);
        }
        else if (strcmp(quads[i].op, "func_begin") == 0) {
            fprintf(fp, "%2d: func_begin %s\n", i, quads[i].arg1);
        }
        else if (strcmp(quads[i].op, "func_end") == 0) {
            fprintf(fp, "%2d: func_end %s\n", i, quads[i].arg1);
        }
        else if (strcmp(quads[i].op, "return") == 0) {
            if (strcmp(quads[i].arg1, "-") == 0) {
                fprintf(fp, "%2d: return\n", i);
            } else {
                fprintf(fp, "%2d: return %s\n", i, quads[i].arg1);
            }
        }
        else if (strcmp(quads[i].op, "param") == 0) {
            fprintf(fp, "%2d: param %s\n", i, quads[i].arg1);
        }
        else if (strcmp(quads[i].op, "call") == 0) {
            if (strcmp(quads[i].result, "-") == 0) {
                fprintf(fp, "%2d: call %s, %s\n", i, quads[i].arg1, quads[i].arg2);
            } else {
                fprintf(fp, "%2d: %s = call %s, %s\n", i, quads[i].result, quads[i].arg1, quads[i].arg2);
            }
        }
        else if (strcmp(quads[i].op, "uminus") == 0) {
            fprintf(fp, "%2d: %s = -%s\n", i, quads[i].result, quads[i].arg1);
        }
        else if (strcmp(quads[i].op, "-") == 0) {
            if (strcmp(quads[i].arg2, "-") != 0) {
                fprintf(fp, "%2d: %s = %s - %s\n", i, quads[i].result, quads[i].arg1, quads[i].arg2);
            } else {
                fprintf(fp, "%2d: %s\n", i, quads[i].result);
            }
        }
        else {
            fprintf(fp, "%2d: %s = %s %s %s\n", i, quads[i].result, quads[i].arg1, quads[i].op, quads[i].arg2);
        }
    }
}

void print_quads(void) {
    print_quads_to_stream(stdout);
}

void print_quad_table_to_stream(FILE *fp) {
    fprintf(fp, "\n=========================================================================\n");
    fprintf(fp, "                        QUADRUPLE TABLE REPRESENTATION                   \n");
    fprintf(fp, "=========================================================================\n");
    fprintf(fp, "| %-5s | %-12s | %-20s | %-12s | %-12s |\n", "Index", "Operator", "Argument 1", "Argument 2", "Result");
    fprintf(fp, "+-------+--------------+----------------------+--------------+--------------+\n");
    for (int i = 0; i < nextquad; i++) {
        fprintf(fp, "| %-5d | %-12s | %-20s | %-12s | %-12s |\n",
                i, quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].result);
    }
    fprintf(fp, "=========================================================================\n\n");
}

void print_quad_table(void) {
    print_quad_table_to_stream(stdout);
}

void export_quads_json(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Warning: Could not open %s for JSON export\n", filename);
        return;
    }
    fprintf(fp, "{\n  \"quadruples\": [\n");
    for (int i = 0; i < nextquad; i++) {
        fprintf(fp, "    {\"index\": %d, \"op\": \"%s\", \"arg1\": \"%s\", \"arg2\": \"%s\", \"result\": \"%s\"}%s\n",
                i, quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].result, (i == nextquad - 1) ? "" : ",");
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
}
