#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "common.h"

typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID
} DataType;

typedef struct SymbolEntry {
    char name[64];
    char ir_name[64];      /* Mangled/unique IR name for shadowed variables */
    DataType type;
    int scope_level;
    int line_declared;
    bool is_initialized;
    bool is_active;        /* True if currently in active scope */
    bool is_function;      /* True if function symbol */
    int param_count;       /* Number of parameters if function */
    struct SymbolEntry *next;
} SymbolEntry;

#define HASH_TABLE_SIZE 211

typedef struct SymbolTable {
    SymbolEntry *buckets[HASH_TABLE_SIZE];
    SymbolEntry *all_symbols; /* Linked list of all symbols ever entered for final table display */
    int current_scope;
    int scope_counter;        /* Monotonically increasing counter for unique scope IDs */
} SymbolTable;

extern SymbolTable symtab;

/* Scope & Table Management */
void symtab_init(void);
void symtab_enter_scope(void);
void symtab_exit_scope(void);

/* Symbol Operations */
SymbolEntry* symtab_insert(const char *name, DataType type, int line, bool initialized);
SymbolEntry* symtab_insert_func(const char *name, DataType return_type, int line);
SymbolEntry* symtab_lookup(const char *name);
SymbolEntry* symtab_lookup_current_scope(const char *name);
const char* symtab_get_ir_name(const char *name);
bool symtab_mark_initialized(const char *name);

/* Type Checking and Conversions */
const char* type_to_str(DataType type);
DataType str_to_type(const char *str);
DataType check_binary_op_type(DataType left, DataType right, const char *op, int line);
bool check_assignment_type(DataType target, DataType value, int line);

/* Display */
void symtab_print_to_stream(FILE *fp);
void symtab_print(void);

#endif /* SYMBOL_TABLE_H */
