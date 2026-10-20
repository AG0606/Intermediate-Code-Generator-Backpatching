#ifndef CFG_H
#define CFG_H

#include "quad.h"

#define MAX_BLOCKS 500

typedef enum {
    EDGE_FALLTHROUGH,
    EDGE_TRUE,
    EDGE_FALSE,
    EDGE_JUMP,
    EDGE_EXIT
} EdgeType;

typedef struct CFGEdge {
    int target_block;
    EdgeType type;
    char label[32];
    char color[16];
    struct CFGEdge *next;
} CFGEdge;

typedef struct BasicBlock {
    int id;
    int start_quad;
    int end_quad;
    bool is_leader;
    CFGEdge *successors;
    struct list *predecessors;
    bool is_reachable;
} BasicBlock;

typedef struct ControlFlowGraph {
    int num_blocks;
    BasicBlock blocks[MAX_BLOCKS];
    int entry_block;
    int exit_block;
} ControlFlowGraph;

extern ControlFlowGraph current_cfg;

/* CFG Generation & Analysis */
void cfg_build(void);
void cfg_print_blocks(FILE *fp);
void cfg_export_dot(const char *filename);
void cfg_export_html(const char *filename, const char *source_title);

#endif /* CFG_H */
