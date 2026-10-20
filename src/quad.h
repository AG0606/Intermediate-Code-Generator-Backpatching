#ifndef QUAD_H
#define QUAD_H

#include "common.h"

#define MAX_QUADS 10000

/* Backpatching linked list for unresolved jump targets */
struct list {
    int quad;
    struct list *next;
};

/* Case label linked list for switch statements */
struct CaseLabel {
    int is_default;
    int val;
    int target_quad;
    struct CaseLabel *next;
};

/* Quadruple entry */
struct quad_entry {
    char op[16];
    char arg1[64];
    char arg2[64];
    char result[64];
};

extern struct quad_entry quads[MAX_QUADS];
extern int nextquad;
extern int temp_count;

/* Core backpatching functions */
int emit(const char *op, const char *arg1, const char *arg2, const char *result);
struct list* makelist(int q);
struct list* merge(struct list *p1, struct list *p2);
void backpatch(struct list *p, int target);
char* newtemp(void);

/* Output and inspection routines */
void print_quads(void);
void print_quads_to_stream(FILE *fp);
void print_quad_table(void);
void print_quad_table_to_stream(FILE *fp);
void export_quads_json(const char *filename);

/* Memory cleanup for backpatching lists */
void free_list(struct list *p);

#endif /* QUAD_H */
