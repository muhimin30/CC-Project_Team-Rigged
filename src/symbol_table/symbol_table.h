#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../ast/ast.h"

/* One declared identifier. Entries in the same scope are chained via `next`. */
typedef struct SymEntry {
    char name[128];
    char mangled[160];   /* unique, scope-safe name used by the code generator */
    DataType type;
    int scope_level;
    int line_declared;
    struct SymEntry *next;
} SymEntry;

/* One block scope. Scopes are chained to their parent so lookup can walk
 * outward until it either finds the identifier or falls off the global
 * scope (parent == NULL). */
typedef struct Scope {
    SymEntry *entries;
    int level;
    struct Scope *parent;
} Scope;

/* Reset the symbol table module and create the global scope (level 0). */
void symtab_init(void);

/* Enter/exit a nested block scope. Call exit after leaving the block. */
void symtab_enter_scope(void);
void symtab_exit_scope(void);

/* Declare `name` in the *current* scope.
 * Returns 1 on success, 0 if `name` is already declared in the current
 * scope (redeclaration). */
int symtab_declare(const char *name, DataType type, int line);

/* Look up `name` starting at the current scope and walking outward.
 * Returns the entry if found, NULL otherwise. */
SymEntry *symtab_lookup(const char *name);

/* Print the entries currently visible (for debugging / documentation). */
void symtab_print_current_chain(void);

#endif /* SYMBOL_TABLE_H */
