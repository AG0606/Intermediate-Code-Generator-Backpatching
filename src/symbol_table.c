#include "symbol_table.h"
#include <stdarg.h>

SymbolTable symtab;

static unsigned int hash(const char *key) {
    unsigned int h = 0;
    while (*key) {
        h = (h << 4) + (unsigned char)(*key++);
        unsigned int g = h & 0xF0000000L;
        if (g) h ^= g >> 24;
        h &= ~g;
    }
    return h % HASH_TABLE_SIZE;
}

void symtab_init(void) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        symtab.buckets[i] = NULL;
    }
    symtab.all_symbols = NULL;
    symtab.current_scope = 0;
    symtab.scope_counter = 0;
}

void symtab_enter_scope(void) {
    symtab.current_scope++;
    symtab.scope_counter++;
    if (opt_trace) {
        printf("[TRACE SCOPE] Entering scope level %d (scope ID s%d)\n",
               symtab.current_scope, symtab.scope_counter);
    }
}

void symtab_exit_scope(void) {
    if (opt_trace) {
        printf("[TRACE SCOPE] Exiting scope level %d\n", symtab.current_scope);
    }
    /* Deactivate symbols declared in the current scope */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        SymbolEntry *entry = symtab.buckets[i];
        while (entry) {
            if (entry->scope_level == symtab.current_scope && entry->is_active) {
                entry->is_active = false;
            }
            entry = entry->next;
        }
    }
    symtab.current_scope--;
}

SymbolEntry* symtab_lookup_current_scope(const char *name) {
    unsigned int idx = hash(name);
    SymbolEntry *entry = symtab.buckets[idx];
    while (entry) {
        if (strcmp(entry->name, name) == 0 &&
            entry->scope_level == symtab.current_scope &&
            entry->is_active) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

SymbolEntry* symtab_lookup(const char *name) {
    unsigned int idx = hash(name);
    SymbolEntry *entry = symtab.buckets[idx];
    SymbolEntry *best_match = NULL;
    int max_scope = -1;

    while (entry) {
        if (strcmp(entry->name, name) == 0 && entry->is_active) {
            if (entry->scope_level <= symtab.current_scope && entry->scope_level > max_scope) {
                best_match = entry;
                max_scope = entry->scope_level;
            }
        }
        entry = entry->next;
    }
    return best_match;
}

const char* symtab_get_ir_name(const char *name) {
    SymbolEntry *entry = symtab_lookup(name);
    if (entry && entry->ir_name[0] != '\0') {
        return entry->ir_name;
    }
    return name;
}

SymbolEntry* symtab_insert(const char *name, DataType type, int line, bool initialized) {
    SymbolEntry *existing_same_scope = symtab_lookup_current_scope(name);
    if (existing_same_scope) {
        report_semantic_error(line, "Redeclaration of variable '%s' in the same scope (previously declared at line %d)",
                              name, existing_same_scope->line_declared);
        return existing_same_scope;
    }

    SymbolEntry *entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!entry) {
        fprintf(stderr, "Out of memory in symtab_insert\n");
        exit(1);
    }
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';

    /* Check if shadowing an outer variable */
    SymbolEntry *outer = symtab_lookup(name);
    if (outer && symtab.current_scope > 0) {
        /* Assign distinct IR name for the shadowed variable */
        snprintf(entry->ir_name, sizeof(entry->ir_name), "%s_s%d", name, symtab.scope_counter);
        if (opt_trace) {
            printf("[TRACE SCOPE] Variable '%s' shadows outer '%s', assigned IR name '%s'\n",
                   name, outer->name, entry->ir_name);
        }
    } else {
        strncpy(entry->ir_name, name, sizeof(entry->ir_name) - 1);
        entry->ir_name[sizeof(entry->ir_name) - 1] = '\0';
    }

    entry->type = type;
    entry->scope_level = symtab.current_scope;
    entry->line_declared = line;
    entry->is_initialized = initialized;
    entry->is_active = true;
    entry->is_function = false;
    entry->param_count = 0;

    unsigned int idx = hash(name);
    entry->next = symtab.buckets[idx];
    symtab.buckets[idx] = entry;

    /* Add to all_symbols list for summary printing */
    SymbolEntry *all_node = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    *all_node = *entry;
    all_node->next = symtab.all_symbols;
    symtab.all_symbols = all_node;

    if (opt_trace) {
        printf("[TRACE SYMTAB] Declared '%s' [IR: '%s'] (%s) at line %d (scope %d)\n",
               entry->name, entry->ir_name, type_to_str(type), line, symtab.current_scope);
    }
    return entry;
}

SymbolEntry* symtab_insert_func(const char *name, DataType return_type, int line) {
    SymbolEntry *existing = symtab_lookup(name);
    if (existing && existing->is_function) {
        report_semantic_error(line, "Redeclaration of function '%s' (previously declared at line %d)",
                              name, existing->line_declared);
        return existing;
    }

    SymbolEntry *entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!entry) {
        fprintf(stderr, "Out of memory in symtab_insert_func\n");
        exit(1);
    }
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    strncpy(entry->ir_name, name, sizeof(entry->ir_name) - 1);
    entry->ir_name[sizeof(entry->ir_name) - 1] = '\0';
    entry->type = return_type;
    entry->scope_level = 0;
    entry->line_declared = line;
    entry->is_initialized = true;
    entry->is_active = true;
    entry->is_function = true;
    entry->param_count = 0;

    unsigned int idx = hash(name);
    entry->next = symtab.buckets[idx];
    symtab.buckets[idx] = entry;

    SymbolEntry *all_node = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    *all_node = *entry;
    all_node->next = symtab.all_symbols;
    symtab.all_symbols = all_node;

    return entry;
}

bool symtab_mark_initialized(const char *name) {
    SymbolEntry *entry = symtab_lookup(name);
    if (entry) {
        entry->is_initialized = true;
        return true;
    }
    return false;
}

const char* type_to_str(DataType type) {
    switch (type) {
        case TYPE_INT:   return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR:  return "char";
        case TYPE_BOOL:  return "bool";
        case TYPE_VOID:  return "void";
        default:         return "unknown";
    }
}

DataType str_to_type(const char *str) {
    if (!str) return TYPE_UNKNOWN;
    if (strcmp(str, "int") == 0 || strcmp(str, "short") == 0 || strcmp(str, "long") == 0) return TYPE_INT;
    if (strcmp(str, "float") == 0 || strcmp(str, "double") == 0) return TYPE_FLOAT;
    if (strcmp(str, "char") == 0) return TYPE_CHAR;
    if (strcmp(str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(str, "void") == 0) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

DataType check_binary_op_type(DataType left, DataType right, const char *op, int line) {
    if (left == TYPE_UNKNOWN || right == TYPE_UNKNOWN) {
        return TYPE_INT;
    }

    /* Relational operators return bool/int */
    if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
        strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
        if (left != right) {
            report_semantic_warning(line, "Comparison between different types (%s vs %s)",
                                    type_to_str(left), type_to_str(right));
        }
        return TYPE_BOOL;
    }

    /* Modulo requires integer types */
    if (strcmp(op, "%") == 0) {
        if (left != TYPE_INT || right != TYPE_INT) {
            report_semantic_error(line, "Operator '%%' requires integer operands, got %s and %s",
                                  type_to_str(left), type_to_str(right));
            return TYPE_INT;
        }
        return TYPE_INT;
    }

    /* Arithmetic operations */
    if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    if (left == TYPE_INT && right == TYPE_INT) {
        return TYPE_INT;
    }

    report_semantic_warning(line, "Operation '%s' between types %s and %s",
                            op, type_to_str(left), type_to_str(right));
    return left;
}

bool check_assignment_type(DataType target, DataType value, int line) {
    if (target == TYPE_UNKNOWN || value == TYPE_UNKNOWN) return true;
    if (target == value) return true;

    if (target == TYPE_FLOAT && value == TYPE_INT) {
        return true;
    }

    if (target == TYPE_INT && value == TYPE_FLOAT) {
        report_semantic_warning(line, "Possible loss of precision: assigning float to int");
        return true;
    }

    report_semantic_error(line, "Type mismatch in assignment: cannot convert %s to %s",
                          type_to_str(value), type_to_str(target));
    return false;
}

void symtab_print_to_stream(FILE *fp) {
    fprintf(fp, "\n=========================================================================================\n");
    fprintf(fp, "                    SYMBOL TABLE ENTRIES & SCOPE HIERARCHY               \n");
    fprintf(fp, "=========================================================================================\n");
    fprintf(fp, "| %-16s | %-16s | %-8s | %-6s | %-9s | %-11s | %-8s |\n",
            "Variable Name", "IR Symbol Name", "Type", "Scope", "Declared", "Initialized", "Kind");
    fprintf(fp, "+------------------+------------------+----------+--------+-----------+-------------+----------+\n");

    SymbolEntry *curr = symtab.all_symbols;
    int count = 0;
    while (curr) {
        fprintf(fp, "| %-16s | %-16s | %-8s | Scope %-1d | Line %-4d | %-11s | %-8s |\n",
                curr->name, curr->ir_name, type_to_str(curr->type), curr->scope_level,
                curr->line_declared, curr->is_initialized ? "yes" : "no",
                curr->is_function ? "function" : "variable");
        count++;
        curr = curr->next;
    }
    if (count == 0) {
        fprintf(fp, "| %-85s |\n", "  (No symbols declared)  ");
    }
    fprintf(fp, "=========================================================================================\n\n");
}

void symtab_print(void) {
    symtab_print_to_stream(stdout);
}
