#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"
#include "../symbol_table/symbol_table.h"

static int error_count = 0;

static void sem_error(int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Semantic Error (line %d): ", line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    error_count++;
}

static int is_numeric(DataType t) { return t == TYPE_INT || t == TYPE_FLOAT; }

static DataType decl_type_from_name(const char *name) {
    if (strcmp(name, "int") == 0) return TYPE_INT;
    if (strcmp(name, "float") == 0) return TYPE_FLOAT;
    if (strcmp(name, "bool") == 0) return TYPE_BOOL;
    return TYPE_UNKNOWN;
}

/* Is it legal to assign a value of type `from` into a variable of type `to`? */
static int assignable(DataType to, DataType from) {
    if (to == from) return 1;
    if (to == TYPE_FLOAT && from == TYPE_INT) return 1; /* int -> float widening */
    return 0;
}

static DataType analyze_expr(Node *n) {
    if (!n) return TYPE_UNKNOWN;

    switch (n->type) {
        case ND_INT_LIT:
            n->data_type = TYPE_INT;
            break;

        case ND_FLOAT_LIT:
            n->data_type = TYPE_FLOAT;
            break;

        case ND_BOOL_LIT:
            n->data_type = TYPE_BOOL;
            break;

        case ND_ID: {
            SymEntry *e = symtab_lookup(n->name);
            if (!e) {
                sem_error(n->line, "undeclared variable '%s' used", n->name);
                n->data_type = TYPE_UNKNOWN;
                n->resolved_name = n->name;
            } else {
                n->data_type = e->type;
                n->resolved_name = strdup(e->mangled);
            }
            break;
        }

        case ND_UNOP: {
            DataType t = analyze_expr(n->left);
            if (strcmp(n->op, "!") == 0) {
                if (t != TYPE_BOOL && t != TYPE_UNKNOWN)
                    sem_error(n->line, "logical operator '!' requires a bool operand, got %s",
                              datatype_str(t));
                n->data_type = TYPE_BOOL;
            } else { /* unary minus */
                if (!is_numeric(t) && t != TYPE_UNKNOWN)
                    sem_error(n->line, "unary '-' requires a numeric operand, got %s",
                              datatype_str(t));
                n->data_type = (t == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            }
            break;
        }

        case ND_BINOP: {
            DataType lt = analyze_expr(n->left);
            DataType rt = analyze_expr(n->right);
            const char *op = n->op;

            if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                if ((lt != TYPE_BOOL && lt != TYPE_UNKNOWN) ||
                    (rt != TYPE_BOOL && rt != TYPE_UNKNOWN))
                    sem_error(n->line,
                              "logical operator '%s' requires bool operands, got %s and %s",
                              op, datatype_str(lt), datatype_str(rt));
                n->data_type = TYPE_BOOL;
            } else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
                int ok = (is_numeric(lt) && is_numeric(rt)) || (lt == rt && lt == TYPE_BOOL);
                if (!ok && lt != TYPE_UNKNOWN && rt != TYPE_UNKNOWN)
                    sem_error(n->line, "cannot compare %s with %s using '%s'",
                              datatype_str(lt), datatype_str(rt), op);
                n->data_type = TYPE_BOOL;
            } else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
                       strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
                if ((!is_numeric(lt) || !is_numeric(rt)) &&
                    lt != TYPE_UNKNOWN && rt != TYPE_UNKNOWN)
                    sem_error(n->line, "relational operator '%s' requires numeric operands, got %s and %s",
                              op, datatype_str(lt), datatype_str(rt));
                n->data_type = TYPE_BOOL;
            } else if (strcmp(op, "%") == 0) {
                if ((lt != TYPE_INT || rt != TYPE_INT) &&
                    lt != TYPE_UNKNOWN && rt != TYPE_UNKNOWN)
                    sem_error(n->line, "'%%' requires two int operands, got %s and %s",
                              datatype_str(lt), datatype_str(rt));
                n->data_type = TYPE_INT;
            } else { /* + - * / */
                if ((!is_numeric(lt) || !is_numeric(rt)) &&
                    lt != TYPE_UNKNOWN && rt != TYPE_UNKNOWN)
                    sem_error(n->line, "arithmetic operator '%s' requires numeric operands, got %s and %s",
                              op, datatype_str(lt), datatype_str(rt));
                n->data_type = (lt == TYPE_FLOAT || rt == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            }
            break;
        }

        default:
            sem_error(n->line, "internal error: node of type %s is not an expression",
                      nodetype_str(n->type));
            n->data_type = TYPE_UNKNOWN;
    }

    return n->data_type;
}

static void analyze_stmt(Node *n) {
    if (!n) return;

    switch (n->type) {
        case ND_DECL: {
            DataType t = decl_type_from_name(n->decl_type_name);
            if (!symtab_declare(n->name, t, n->line)) {
                SymEntry *prev = symtab_lookup(n->name);
                sem_error(n->line, "redeclaration of variable '%s' (already declared at line %d)",
                          n->name, prev ? prev->line_declared : -1);
            }
            /* Whether declare succeeded or not, look the entry up so codegen
             * always has a valid (unique) name to emit. */
            SymEntry *e = symtab_lookup(n->name);
            n->resolved_name = e ? strdup(e->mangled) : n->name;

            if (n->right) {
                DataType et = analyze_expr(n->right);
                if (!assignable(t, et) && et != TYPE_UNKNOWN)
                    sem_error(n->line, "cannot initialize %s '%s' with a value of type %s",
                              datatype_str(t), n->name, datatype_str(et));
            }
            n->data_type = t;
            break;
        }

        case ND_ASSIGN: {
            SymEntry *e = symtab_lookup(n->name);
            DataType et = analyze_expr(n->right);
            if (!e) {
                sem_error(n->line, "assignment to undeclared variable '%s'", n->name);
            } else if (!assignable(e->type, et) && et != TYPE_UNKNOWN) {
                sem_error(n->line, "invalid assignment: cannot assign %s to '%s' of type %s",
                          datatype_str(et), n->name, datatype_str(e->type));
            }
            n->data_type = e ? e->type : TYPE_UNKNOWN;
            n->resolved_name = e ? strdup(e->mangled) : n->name;
            break;
        }

        case ND_IF: {
            DataType ct = analyze_expr(n->left);
            if (ct != TYPE_BOOL && ct != TYPE_UNKNOWN)
                sem_error(n->line, "if-condition must be bool, got %s", datatype_str(ct));
            analyze_stmt(n->right);
            if (n->third) analyze_stmt(n->third);
            break;
        }

        case ND_WHILE: {
            DataType ct = analyze_expr(n->left);
            if (ct != TYPE_BOOL && ct != TYPE_UNKNOWN)
                sem_error(n->line, "while-condition must be bool, got %s", datatype_str(ct));
            analyze_stmt(n->right);
            break;
        }

        case ND_PRINT:
            analyze_expr(n->right);
            break;

        case ND_BLOCK:
            symtab_enter_scope();
            for (int i = 0; i < n->stmt_count; i++) analyze_stmt(n->stmts[i]);
            symtab_exit_scope();
            break;

        default:
            sem_error(n->line, "internal error: node of type %s is not a statement",
                      nodetype_str(n->type));
    }
}

int semantic_analyze(Node *program) {
    error_count = 0;
    symtab_init();

    /* The program's top level statements share the global scope (level 0);
     * we do NOT push a new scope here, so declarations make it into the
     * scope that symtab_init() created. */
    for (int i = 0; i < program->stmt_count; i++) {
        analyze_stmt(program->stmts[i]);
    }

    return error_count;
}
