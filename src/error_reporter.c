#include "common.h"
#include <stdarg.h>

bool opt_optimize = false;
bool opt_cfg = false;
bool opt_symtab = false;
bool opt_quads = false;
bool opt_all = false;
bool opt_trace = false;
bool opt_strict = false;
bool opt_json = false;
const char *output_cfg_dot = "cfg.dot";
const char *output_cfg_html = "cfg.html";

int semantic_errors = 0;
int semantic_warnings = 0;
int syntax_errors = 0;

void report_semantic_error(int line, const char *fmt, ...) {
    semantic_errors++;
    fprintf(stderr, "\033[1;31m[SEMANTIC ERROR]\033[0m Line %d: ", line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void report_semantic_warning(int line, const char *fmt, ...) {
    semantic_warnings++;
    fprintf(stderr, "\033[1;33m[SEMANTIC WARNING]\033[0m Line %d: ", line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void report_syntax_error(int line, int col, const char *msg, const char *near_token) {
    syntax_errors++;
    if (near_token && *near_token) {
        fprintf(stderr, "\033[1;31m[SYNTAX ERROR]\033[0m Line %d, Column %d: %s near '%s'\n",
                line, col, msg, near_token);
    } else {
        fprintf(stderr, "\033[1;31m[SYNTAX ERROR]\033[0m Line %d, Column %d: %s\n",
                line, col, msg);
    }
}
