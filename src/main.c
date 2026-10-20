#include "common.h"
#include "quad.h"
#include "symbol_table.h"
#include "loop_stack.h"
#include "cfg.h"
#include "optimizer.h"

extern int yyparse(void);
extern FILE *yyin;

static void print_banner(void) {
    printf("=========================================================================\n");
    printf("     INTERMEDIATE CODE GENERATOR FOR CONTROL-FLOW STATEMENTS (ICG)      \n");
    printf("           Compiler Design Project - Review 2 (100%% Implementation)      \n");
    printf("=========================================================================\n");
}

static void print_usage(const char *progname) {
    printf("Usage: %s [options] [input_file]\n", progname);
    printf("Options:\n");
    printf("  --all            Execute full pipeline (SymTab, Quads, CFG, Optimization)\n");
    printf("  -O, --optimize   Enable intermediate code optimization passes\n");
    printf("  --cfg            Partition into basic blocks and generate CFG (DOT & HTML)\n");
    printf("  --symtab         Display the symbol table\n");
    printf("  --quads          Display the quadruple table\n");
    printf("  --trace          Enable detailed trace of emit and backpatching steps\n");
    printf("  --strict         Enable strict semantic checking (reject undeclared vars)\n");
    printf("  --json           Export quadruples to quads.json\n");
    printf("  -h, --help       Display this help message\n\n");
    printf("If no input file is specified, input is read from standard input.\n");
    printf("Example:\n");
    printf("  %s < input.txt\n", progname);
    printf("  %s input.txt --all\n", progname);
}

int main(int argc, char **argv) {
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            opt_all = true;
            opt_cfg = true;
            opt_symtab = true;
            opt_quads = true;
            opt_optimize = true;
        }
        else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--optimize") == 0) {
            opt_optimize = true;
        }
        else if (strcmp(argv[i], "--cfg") == 0) {
            opt_cfg = true;
        }
        else if (strcmp(argv[i], "--symtab") == 0) {
            opt_symtab = true;
        }
        else if (strcmp(argv[i], "--quads") == 0) {
            opt_quads = true;
        }
        else if (strcmp(argv[i], "--trace") == 0) {
            opt_trace = true;
        }
        else if (strcmp(argv[i], "--strict") == 0) {
            opt_strict = true;
        }
        else if (strcmp(argv[i], "--json") == 0) {
            opt_json = true;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
        else {
            input_file = argv[i];
        }
    }

    if (input_file) {
        yyin = fopen(input_file, "r");
        if (!yyin) {
            fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
            return 1;
        }
    } else {
        yyin = stdin;
    }

    /* Initialize compiler subsystems */
    symtab_init();
    loop_stack_init();

    if (opt_all) {
        print_banner();
        if (input_file) printf("Compiling source: %s\n\n", input_file);
    }

    /* Parse and generate 3-address code */
    int parse_result = yyparse();

    if (input_file) {
        fclose(yyin);
    }

    if (parse_result != 0 || syntax_errors > 0) {
        fprintf(stderr, "\nCompilation terminated with %d syntax error(s).\n", syntax_errors);
        return 1;
    }

    if (semantic_errors > 0 && opt_strict) {
        fprintf(stderr, "\nCompilation failed with %d semantic error(s).\n", semantic_errors);
        return 1;
    }

    /* Symbol Table Display */
    if (opt_symtab) {
        symtab_print();
    }

    /* Quadruple Table Display */
    if (opt_quads) {
        print_quad_table();
    }

    /* Default output if not in all mode and no specific quads requested: print 3AC */
    if (!opt_all && !opt_quads) {
        print_quads();
    } else if (opt_all) {
        printf("\n=========================================================================\n");
        printf("               GENERATED THREE-ADDRESS CODE (UNOPTIMIZED)                \n");
        printf("=========================================================================\n");
        print_quads();
        printf("=========================================================================\n");
    }

    /* CFG Generation (before optimization or standalone) */
    if (opt_cfg) {
        cfg_build();
        cfg_print_blocks(stdout);
        cfg_export_dot(output_cfg_dot);
        cfg_export_html(output_cfg_html, input_file ? input_file : "ICG Output");
        printf("[CFG] Exported Graphviz DOT to '%s'\n", output_cfg_dot);
        printf("[CFG] Exported Interactive HTML Visualizer to '%s'\n", output_cfg_html);
    }

    /* Optimization Pass */
    if (opt_optimize) {
        optimize_quads();
        print_optimization_report(stdout);

        printf("=========================================================================\n");
        printf("                OPTIMIZED THREE-ADDRESS CODE (AFTER PASSES)              \n");
        printf("=========================================================================\n");
        print_quads();
        printf("=========================================================================\n");

        if (opt_cfg) {
            cfg_build();
            cfg_export_dot("cfg_optimized.dot");
            cfg_export_html("cfg_optimized.html", "Optimized CFG");
            printf("[CFG] Exported Optimized CFG to 'cfg_optimized.dot' and 'cfg_optimized.html'\n");
        }
    }

    /* JSON export */
    if (opt_json) {
        export_quads_json("quads.json");
        printf("[JSON] Exported quadruples to 'quads.json'\n");
    }

    if (opt_all) {
        printf("\n[SUCCESS] Compilation, analysis, CFG generation, and optimization complete!\n");
        if (semantic_warnings > 0) {
            printf("[NOTE] Finished with %d semantic warning(s).\n", semantic_warnings);
        }
    }

    return 0;
}
