#include "loop_stack.h"

static LoopContext *loop_stack_head = NULL;

void loop_stack_init(void) {
    loop_stack_head = NULL;
}

void loop_stack_push(ContextType type) {
    LoopContext *ctx = (LoopContext*)malloc(sizeof(LoopContext));
    if (!ctx) {
        fprintf(stderr, "Out of memory in loop_stack_push\n");
        exit(1);
    }
    ctx->type = type;
    ctx->continue_target = -1;
    ctx->breaklist = NULL;
    ctx->continuelist = NULL;
    ctx->next = loop_stack_head;
    loop_stack_head = ctx;

    if (opt_trace) {
        printf("[TRACE LOOP] Pushed context type %d\n", type);
    }
}

LoopContext* loop_stack_top(void) {
    return loop_stack_head;
}

LoopContext* loop_stack_top_loop_only(void) {
    LoopContext *curr = loop_stack_head;
    while (curr) {
        if (curr->type == CTX_WHILE || curr->type == CTX_DO_WHILE || curr->type == CTX_FOR) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

LoopContext* loop_stack_pop(void) {
    if (!loop_stack_head) return NULL;
    LoopContext *ctx = loop_stack_head;
    loop_stack_head = loop_stack_head->next;
    if (opt_trace) {
        printf("[TRACE LOOP] Popped context type %d\n", ctx->type);
    }
    return ctx;
}

void handle_break(int line, struct list **out_nextlist) {
    LoopContext *ctx = loop_stack_top();
    if (!ctx) {
        report_semantic_error(line, "'break' statement not within loop or switch");
        *out_nextlist = NULL;
        return;
    }
    int q = emit("goto", "-", "-", "-");
    ctx->breaklist = merge(ctx->breaklist, makelist(q));
    *out_nextlist = NULL;
}

void handle_continue(int line, struct list **out_nextlist) {
    LoopContext *ctx = loop_stack_top_loop_only();
    if (!ctx) {
        report_semantic_error(line, "'continue' statement not within a loop");
        *out_nextlist = NULL;
        return;
    }
    int q = emit("goto", "-", "-", "-");
    ctx->continuelist = merge(ctx->continuelist, makelist(q));
    *out_nextlist = NULL;
}
