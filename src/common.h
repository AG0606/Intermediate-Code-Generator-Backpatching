#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Compiler execution options */
extern bool opt_optimize;
extern bool opt_cfg;
extern bool opt_symtab;
extern bool opt_quads;
extern bool opt_all;
extern bool opt_trace;
extern bool opt_strict;
extern bool opt_json;
extern const char *output_cfg_dot;
extern const char *output_cfg_html;

/* Error tracking */
extern int semantic_errors;
extern int semantic_warnings;
extern int syntax_errors;

/* Error reporting helpers */
void report_semantic_error(int line, const char *fmt, ...);
void report_semantic_warning(int line, const char *fmt, ...);
void report_syntax_error(int line, int col, const char *msg, const char *near_token);

#endif /* COMMON_H */
