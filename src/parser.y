%code requires {
    #include "common.h"
    #include "quad.h"
    #include "symbol_table.h"
    #include "loop_stack.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "quad.h"
#include "symbol_table.h"
#include "loop_stack.h"
#include "cfg.h"
#include "optimizer.h"

extern int yylex(void);
extern int yylineno;
extern int yycolumn;
extern char *yytext;
void yyerror(const char *s);

static DataType current_decl_type = TYPE_INT;

%}

%union {
    int quad;
    int arg_count;
    struct list *list_ptr;
    char str[64];
    struct {
        struct list *truelist;
        struct list *falselist;
    } b_expr;
    struct {
        struct list *nextlist;
    } s_stmt;
    struct {
        char str[64];
        DataType type;
    } expr_val;
    DataType data_type;
    struct CaseLabel *case_lbls;
}

%token <str> ID NUM FLOAT_NUM RELOP
%token <str> TYPE_INT_KW TYPE_FLOAT_KW TYPE_CHAR_KW TYPE_BOOL_KW TYPE_VOID_KW
%token IF ELSE WHILE FOR DO SWITCH CASE DEFAULT BREAK CONTINUE RETURN
%token INC_OP DEC_OP ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token SHL_OP SHR_OP
%token AND OR NOT TRUE_KW FALSE_KW

%type <b_expr> B
%type <s_stmt> S L for_init for_step declaration compound_stmt function_definition top_level_item translation_unit
%type <expr_val> E
%type <quad> M
%type <list_ptr> N N_skip N_back N_switch
%type <data_type> type_spec
%type <case_lbls> case_list case_clause
%type <arg_count> arg_list_opt arg_list

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left RELOP
%left SHL_OP SHR_OP
%left '+' '-'
%left '*' '/' '%'
%right NOT '~' UMINUS
%left INC_OP DEC_OP

%%

Program:
    translation_unit {
        backpatch($1.nextlist, nextquad);
        emit("end", "-", "-", "-");
    }
    ;

translation_unit:
    translation_unit M top_level_item {
        backpatch($1.nextlist, $2);
        $$.nextlist = $3.nextlist;
    }
    | top_level_item {
        $$.nextlist = $1.nextlist;
    }
    ;

top_level_item:
    function_definition {
        $$.nextlist = $1.nextlist;
    }
    | S {
        $$.nextlist = $1.nextlist;
    }
    ;

function_definition:
    type_spec ID '(' param_list_opt ')' '{' {
        symtab_insert_func($2, $1, yylineno);
        symtab_enter_scope();
        emit("func_begin", $2, "-", "-");
    }
    L '}' {
        symtab_exit_scope();
        emit("func_end", $2, "-", "-");
        $$.nextlist = $8.nextlist;
    }
    ;

param_list_opt:
    /* empty */
    | param_list
    ;

param_list:
    param_list ',' param
    | param
    ;

param:
    type_spec ID {
        symtab_insert($2, $1, yylineno, true);
    }
    ;

/* Marker to record nextquad for jump target resolution */
M:
    /* empty */ {
        $$ = nextquad;
    }
    ;

/* Marker to emit unconditional jump with unresolved target */
N:
    /* empty */ {
        $$ = makelist(nextquad);
        emit("goto", "-", "-", "-");
    }
    ;

N_skip:
    /* empty */ {
        $$ = makelist(nextquad);
        emit("goto", "-", "-", "-");
    }
    ;

N_back:
    /* empty */ {
        $$ = makelist(nextquad);
        emit("goto", "-", "-", "-");
    }
    ;

N_switch:
    /* empty */ {
        $$ = makelist(nextquad);
        emit("goto", "-", "-", "-");
    }
    ;

push_while_ctx:
    /* empty */ {
        loop_stack_push(CTX_WHILE);
    }
    ;

push_do_ctx:
    /* empty */ {
        loop_stack_push(CTX_DO_WHILE);
    }
    ;

push_for_ctx:
    /* empty */ {
        loop_stack_push(CTX_FOR);
    }
    ;

push_switch_ctx:
    /* empty */ {
        loop_stack_push(CTX_SWITCH);
    }
    ;

L:
    L M S {
        backpatch($1.nextlist, $2);
        $$.nextlist = $3.nextlist;
    }
    | S {
        $$.nextlist = $1.nextlist;
    }
    | L error ';' {
        /* Error recovery: synchronize on semicolon */
        $$.nextlist = $1.nextlist;
    }
    ;

compound_stmt:
    '{' { symtab_enter_scope(); } L '}' {
        symtab_exit_scope();
        $$.nextlist = $3.nextlist;
    }
    | '{' { symtab_enter_scope(); } '}' {
        symtab_exit_scope();
        $$.nextlist = NULL;
    }
    ;

S:
    ID '=' E ';' {
        $$.nextlist = NULL;
        SymbolEntry *entry = symtab_lookup($1);
        if (!entry) {
            if (opt_strict) {
                report_semantic_error(yylineno, "Undeclared variable '%s'", $1);
            } else {
                entry = symtab_insert($1, $3.type, yylineno, true);
            }
        } else {
            check_assignment_type(entry->type, $3.type, yylineno);
            entry->is_initialized = true;
        }
        const char *ir_name = entry ? entry->ir_name : $1;
        emit("=", $3.str, "-", ir_name);
    }
    | ID ADD_ASSIGN E ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("+", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID SUB_ASSIGN E ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("-", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID MUL_ASSIGN E ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("*", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID DIV_ASSIGN E ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("/", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID MOD_ASSIGN E ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("%", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID INC_OP ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("+", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | INC_OP ID ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($2);
        char *t = newtemp();
        emit("+", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | ID DEC_OP ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("-", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | DEC_OP ID ';' {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($2);
        char *t = newtemp();
        emit("-", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | ID '(' arg_list_opt ')' ';' {
        char count_str[16];
        sprintf(count_str, "%d", $3);
        emit("call", $1, count_str, "-");
        $$.nextlist = NULL;
    }
    | RETURN ';' {
        emit("return", "-", "-", "-");
        $$.nextlist = NULL;
    }
    | RETURN E ';' {
        emit("return", $2.str, "-", "-");
        $$.nextlist = NULL;
    }
    | declaration {
        $$.nextlist = $1.nextlist;
    }
    | compound_stmt {
        $$.nextlist = $1.nextlist;
    }
    | IF '(' B ')' M S %prec LOWER_THAN_ELSE {
        backpatch($3.truelist, $5);
        $$.nextlist = merge($3.falselist, $6.nextlist);
    }
    | IF '(' B ')' M S N ELSE M S {
        backpatch($3.truelist, $5);
        backpatch($3.falselist, $9);
        struct list *temp = merge($6.nextlist, $7);
        $$.nextlist = merge(temp, $10.nextlist);
    }
    | WHILE M '(' B ')' M push_while_ctx S {
        LoopContext *ctx = loop_stack_pop();
        if (ctx) {
            backpatch(ctx->continuelist, $2);
        }
        backpatch($8.nextlist, $2);
        backpatch($4.truelist, $6);
        char target[16];
        sprintf(target, "%d", $2);
        emit("goto", "-", "-", target);
        $$.nextlist = merge($4.falselist, ctx ? ctx->breaklist : NULL);
    }
    | DO M push_do_ctx S WHILE M '(' B ')' ';' {
        LoopContext *ctx = loop_stack_pop();
        backpatch($4.nextlist, $6);
        if (ctx) {
            backpatch(ctx->continuelist, $6);
        }
        backpatch($8.truelist, $2);
        $$.nextlist = merge($8.falselist, ctx ? ctx->breaklist : NULL);
    }
    | FOR '(' for_init ';' M B ';' N_skip M for_step N_back ')' M push_for_ctx S {
        LoopContext *ctx = loop_stack_pop();
        backpatch($6.truelist, $13);           /* cond true -> body */
        backpatch($8, $13);                    /* skip step -> body */
        backpatch($11, $5);                    /* step back -> cond */
        backpatch($15.nextlist, $9);           /* body end -> step */
        if (ctx) {
            backpatch(ctx->continuelist, $9);  /* continue -> step */
        }
        char step_str[16];
        sprintf(step_str, "%d", $9);
        emit("goto", "-", "-", step_str);      /* loop back to step */
        $$.nextlist = merge($6.falselist, ctx ? ctx->breaklist : NULL);
    }
    | SWITCH '(' E ')' N_switch push_switch_ctx '{' case_list '}' {
        LoopContext *ctx = loop_stack_pop();
        int test_start = nextquad;
        backpatch($5, test_start);

        struct CaseLabel *curr = $8;
        int default_target = -1;
        while (curr) {
            if (curr->is_default) {
                default_target = curr->target_quad;
            } else {
                char cond[128];
                char target[16];
                sprintf(cond, "%s == %d", $3.str, curr->val);
                sprintf(target, "%d", curr->target_quad);
                emit("if", cond, "goto", target);
            }
            curr = curr->next;
        }

        struct list *exit_jump = NULL;
        if (default_target != -1) {
            char def_target[16];
            sprintf(def_target, "%d", default_target);
            emit("goto", "-", "-", def_target);
        } else {
            int q = emit("goto", "-", "-", "-");
            exit_jump = makelist(q);
        }

        $$.nextlist = merge(ctx ? ctx->breaklist : NULL, exit_jump);
    }
    | BREAK ';' {
        handle_break(yylineno, &$$.nextlist);
    }
    | CONTINUE ';' {
        handle_continue(yylineno, &$$.nextlist);
    }
    | ';' {
        $$.nextlist = NULL;
    }
    ;

declaration:
    type_spec init_declarator_list ';' {
        $$.nextlist = NULL;
    }
    ;

init_declarator_list:
    init_declarator_list ',' init_declarator
    | init_declarator
    ;

init_declarator:
    ID {
        symtab_insert($1, current_decl_type, yylineno, false);
    }
    | ID '=' E {
        SymbolEntry *entry = symtab_insert($1, current_decl_type, yylineno, true);
        check_assignment_type(current_decl_type, $3.type, yylineno);
        const char *ir_name = entry ? entry->ir_name : $1;
        emit("=", $3.str, "-", ir_name);
    }
    ;

type_spec:
    TYPE_INT_KW     { $$ = TYPE_INT; current_decl_type = TYPE_INT; }
    | TYPE_FLOAT_KW { $$ = TYPE_FLOAT; current_decl_type = TYPE_FLOAT; }
    | TYPE_CHAR_KW  { $$ = TYPE_CHAR; current_decl_type = TYPE_CHAR; }
    | TYPE_BOOL_KW  { $$ = TYPE_BOOL; current_decl_type = TYPE_BOOL; }
    | TYPE_VOID_KW  { $$ = TYPE_VOID; current_decl_type = TYPE_VOID; }
    ;

for_init:
    /* empty */ {
        $$.nextlist = NULL;
    }
    | type_spec init_declarator_list {
        $$.nextlist = NULL;
    }
    | ID '=' E {
        $$.nextlist = NULL;
        SymbolEntry *entry = symtab_lookup($1);
        if (!entry) {
            if (opt_strict) report_semantic_error(yylineno, "Undeclared variable '%s'", $1);
            else entry = symtab_insert($1, $3.type, yylineno, true);
        } else {
            check_assignment_type(entry->type, $3.type, yylineno);
            entry->is_initialized = true;
        }
        const char *ir_name = entry ? entry->ir_name : $1;
        emit("=", $3.str, "-", ir_name);
    }
    ;

for_step:
    /* empty */ {
        $$.nextlist = NULL;
    }
    | ID '=' E {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        emit("=", $3.str, "-", ir_name);
    }
    | ID INC_OP {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("+", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | INC_OP ID {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($2);
        char *t = newtemp();
        emit("+", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | ID DEC_OP {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("-", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | DEC_OP ID {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($2);
        char *t = newtemp();
        emit("-", ir_name, "1", t);
        emit("=", t, "-", ir_name);
    }
    | ID ADD_ASSIGN E {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("+", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    | ID SUB_ASSIGN E {
        $$.nextlist = NULL;
        const char *ir_name = symtab_get_ir_name($1);
        char *t = newtemp();
        emit("-", ir_name, $3.str, t);
        emit("=", t, "-", ir_name);
    }
    ;

case_list:
    case_list case_clause {
        if (!$1) {
            $$ = $2;
        } else {
            struct CaseLabel *curr = $1;
            while (curr->next) curr = curr->next;
            curr->next = $2;
            $$ = $1;
        }
    }
    | case_clause {
        $$ = $1;
    }
    ;

case_clause:
    CASE NUM ':' M L {
        struct CaseLabel *lbl = (struct CaseLabel*)malloc(sizeof(struct CaseLabel));
        lbl->is_default = 0;
        lbl->val = atoi($2);
        lbl->target_quad = $4;
        lbl->next = NULL;
        $$ = lbl;
    }
    | DEFAULT ':' M L {
        struct CaseLabel *lbl = (struct CaseLabel*)malloc(sizeof(struct CaseLabel));
        lbl->is_default = 1;
        lbl->val = 0;
        lbl->target_quad = $3;
        lbl->next = NULL;
        $$ = lbl;
    }
    ;

B:
    B OR M B {
        backpatch($1.falselist, $3);
        $$.truelist = merge($1.truelist, $4.truelist);
        $$.falselist = $4.falselist;
    }
    | B AND M B {
        backpatch($1.truelist, $3);
        $$.truelist = $4.truelist;
        $$.falselist = merge($1.falselist, $4.falselist);
    }
    | NOT B {
        $$.truelist = $2.falselist;
        $$.falselist = $2.truelist;
    }
    | '(' B ')' {
        $$ = $2;
    }
    | E RELOP E {
        $$.truelist = makelist(nextquad);
        char cond[256];
        sprintf(cond, "%s %s %s", $1.str, $2, $3.str);
        emit("if", cond, "goto", "-");
        $$.falselist = makelist(nextquad);
        emit("goto", "-", "-", "-");
        check_binary_op_type($1.type, $3.type, $2, yylineno);
    }
    | TRUE_KW {
        $$.truelist = makelist(nextquad);
        emit("goto", "-", "-", "-");
        $$.falselist = NULL;
    }
    | FALSE_KW {
        $$.falselist = makelist(nextquad);
        emit("goto", "-", "-", "-");
        $$.truelist = NULL;
    }
    | NUM {
        if (atoi($1) != 0) {
            $$.truelist = makelist(nextquad);
            emit("goto", "-", "-", "-");
            $$.falselist = NULL;
        } else {
            $$.falselist = makelist(nextquad);
            emit("goto", "-", "-", "-");
            $$.truelist = NULL;
        }
    }
    | ID {
        const char *ir_name = symtab_get_ir_name($1);
        $$.truelist = makelist(nextquad);
        char cond[64];
        sprintf(cond, "%s != 0", ir_name);
        emit("if", cond, "goto", "-");
        $$.falselist = makelist(nextquad);
        emit("goto", "-", "-", "-");
    }
    ;

arg_list_opt:
    /* empty */ { $$ = 0; }
    | arg_list   { $$ = $1; }
    ;

arg_list:
    arg_list ',' E {
        emit("param", $3.str, "-", "-");
        $$ = $1 + 1;
    }
    | E {
        emit("param", $1.str, "-", "-");
        $$ = 1;
    }
    ;

E:
    E '+' E {
        char *t = newtemp();
        emit("+", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = check_binary_op_type($1.type, $3.type, "+", yylineno);
    }
    | E '-' E {
        char *t = newtemp();
        emit("-", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = check_binary_op_type($1.type, $3.type, "-", yylineno);
    }
    | E '*' E {
        char *t = newtemp();
        emit("*", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = check_binary_op_type($1.type, $3.type, "*", yylineno);
    }
    | E '/' E {
        char *t = newtemp();
        emit("/", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = check_binary_op_type($1.type, $3.type, "/", yylineno);
    }
    | E '%' E {
        char *t = newtemp();
        emit("%", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = check_binary_op_type($1.type, $3.type, "%", yylineno);
    }
    | E '&' E {
        char *t = newtemp();
        emit("&", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | E '|' E {
        char *t = newtemp();
        emit("|", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | E '^' E {
        char *t = newtemp();
        emit("^", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | E SHL_OP E {
        char *t = newtemp();
        emit("<<", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | E SHR_OP E {
        char *t = newtemp();
        emit(">>", $1.str, $3.str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | '-' E %prec UMINUS {
        char *t = newtemp();
        emit("uminus", $2.str, "-", t);
        strcpy($$.str, t);
        $$.type = $2.type;
    }
    | '~' E {
        char *t = newtemp();
        emit("~", $2.str, "-", t);
        strcpy($$.str, t);
        $$.type = $2.type;
    }
    | ID '(' arg_list_opt ')' {
        char *t = newtemp();
        char count_str[16];
        sprintf(count_str, "%d", $3);
        emit("call", $1, count_str, t);
        strcpy($$.str, t);
        $$.type = TYPE_INT;
    }
    | '(' E ')' {
        $$ = $2;
    }
    | ID {
        SymbolEntry *entry = symtab_lookup($1);
        if (!entry) {
            if (opt_strict) {
                report_semantic_error(yylineno, "Undeclared variable '%s'", $1);
                $$.type = TYPE_UNKNOWN;
                strcpy($$.str, $1);
            } else {
                entry = symtab_insert($1, TYPE_INT, yylineno, false);
                $$.type = TYPE_INT;
                strcpy($$.str, entry->ir_name);
            }
        } else {
            $$.type = entry->type;
            strcpy($$.str, entry->ir_name);
        }
    }
    | NUM {
        strcpy($$.str, $1);
        $$.type = TYPE_INT;
    }
    | FLOAT_NUM {
        strcpy($$.str, $1);
        $$.type = TYPE_FLOAT;
    }
    ;

%%

void yyerror(const char *s) {
    report_syntax_error(yylineno, yycolumn, s, yytext);
}
