#ifndef LOOP_STACK_H
#define LOOP_STACK_H

#include "quad.h"

typedef enum {
    CTX_WHILE,
    CTX_DO_WHILE,
    CTX_FOR,
    CTX_SWITCH
} ContextType;

typedef struct LoopContext {
    ContextType type;
    int continue_target;
    struct list *breaklist;
    struct list *continuelist;
    struct LoopContext *next;
} LoopContext;

void loop_stack_init(void);
void loop_stack_push(ContextType type);
LoopContext* loop_stack_top(void);
LoopContext* loop_stack_top_loop_only(void);
LoopContext* loop_stack_pop(void);

void handle_break(int line, struct list **out_nextlist);
void handle_continue(int line, struct list **out_nextlist);

#endif /* LOOP_STACK_H */
