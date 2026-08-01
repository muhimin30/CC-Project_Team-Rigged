#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

static Scope *current = NULL;
static int uid_counter = 0;

void symtab_init(void) {
    uid_counter = 0;
    current = (Scope *)calloc(1, sizeof(Scope));
    current->level = 0;
    current->parent = NULL;
}

void symtab_enter_scope(void) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    s->level = current->level + 1;
    s->parent = current;
    current = s;
}

void symtab_exit_scope(void) {
    if (!current) return;
    Scope *old = current;
    current = current->parent;

    /* Free the entries and the scope we are leaving. */
    SymEntry *e = old->entries;
    while (e) {
        SymEntry *next = e->next;
        free(e);
        e = next;
    }
    free(old);
}

int symtab_declare(const char *name, DataType type, int line) {
    /* Only the *current* scope is checked for redeclaration: shadowing an
     * outer-scope variable in an inner block is allowed. */
    for (SymEntry *e = current->entries; e; e = e->next) {
        if (strcmp(e->name, name) == 0) {
            return 0; /* redeclared */
        }
    }
    SymEntry *e = (SymEntry *)calloc(1, sizeof(SymEntry));
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->type = type;
    e->scope_level = current->level;
    e->line_declared = line;

    /* Global-scope names are left untouched (readable TAC); anything
     * declared inside a nested block gets a unique suffix so that
     * shadowing a variable never overwrites the outer one's TAC slot. */
    if (current->level == 0) {
        strncpy(e->mangled, name, sizeof(e->mangled) - 1);
    } else {
        snprintf(e->mangled, sizeof(e->mangled), "%s__%d", name, ++uid_counter);
    }

    e->next = current->entries;
    current->entries = e;
    return 1;
}

SymEntry *symtab_lookup(const char *name) {
    for (Scope *s = current; s; s = s->parent) {
        for (SymEntry *e = s->entries; e; e = e->next) {
            if (strcmp(e->name, name) == 0) return e;
        }
    }
    return NULL;
}

void symtab_print_current_chain(void) {
    printf("=== Symbol Table (innermost scope first) ===\n");
    for (Scope *s = current; s; s = s->parent) {
        printf("-- scope level %d --\n", s->level);
        for (SymEntry *e = s->entries; e; e = e->next) {
            printf("  %-15s %-8s declared at line %d\n",
                   e->name, datatype_str(e->type), e->line_declared);
        }
    }
}
