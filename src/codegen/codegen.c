 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static int temp_count = 0;
static int label_count = 0;

static char *new_temp(void) {
    char *buf = (char *)malloc(16);
    snprintf(buf, 16, "t%d", ++temp_count);
    return buf;
}

static char *new_label(void) {
    char *buf = (char *)malloc(16);
    snprintf(buf, 16, "L%d", ++label_count);
    return buf;
}

static char *int_literal_str(int v) {
    char *buf = (char *)malloc(32);
    snprintf(buf, 32, "%d", v);
    return buf;
}

static char *float_literal_str(double v) {
    char *buf = (char *)malloc(32);
    snprintf(buf, 32, "%g", v);
    return buf;
}

static void gen_stmt(Node *n);

static char *gen_expr(Node *n) {
    if (!n) return strdup("?");

    switch (n->type) {
        case ND_ID:
            return strdup(n->resolved_name ? n->resolved_name : n->name);

        case ND_INT_LIT:
            return int_literal_str(n->int_val);

        case ND_FLOAT_LIT:
            return float_literal_str(n->float_val);

        case ND_BOOL_LIT:
            return strdup(n->bool_val ? "true" : "false");

        case ND_UNOP: {
            char *operand = gen_expr(n->left);
            char *result = new_temp();
            if (strcmp(n->op, "-") == 0)
                printf("%s = uminus %s\n", result, operand);
            else
                printf("%s = %s %s\n", result, n->op, operand);
            return result;
        }

        case ND_BINOP: {
            char *l = gen_expr(n->left);
            char *r = gen_expr(n->right);
            char *result = new_temp();
            printf("%s = %s %s %s\n", result, l, n->op, r);
            return result;
        }

        default:
            fprintf(stderr, "codegen: unexpected node %s used as expression\n",
                    nodetype_str(n->type));
            return strdup("?");
    }
}

static void gen_if(Node *n) {
    char *cond = gen_expr(n->left);
    if (!n->third) {
        char *l1 = new_label();
        printf("ifFalse %s goto %s\n", cond, l1);
        gen_stmt(n->right);
        printf("%s:\n", l1);
    } else {
        char *l1 = new_label();
        char *l2 = new_label();
        printf("ifFalse %s goto %s\n", cond, l1);
        gen_stmt(n->right);
        printf("goto %s\n", l2);
        printf("%s:\n", l1);
        gen_stmt(n->third);
        printf("%s:\n", l2);
    }
}

static void gen_while(Node *n) {
    char *l1 = new_label();
    char *l2 = new_label();
    printf("%s:\n", l1);
    char *cond = gen_expr(n->left);
    printf("ifFalse %s goto %s\n", cond, l2);
    gen_stmt(n->right);
    printf("goto %s\n", l1);
    printf("%s:\n", l2);
}

static void gen_stmt(Node *n) {
    if (!n) return;

    switch (n->type) {
        case ND_DECL:
            if (n->right) {
                char *v = gen_expr(n->right);
                printf("%s = %s\n", n->resolved_name ? n->resolved_name : n->name, v);
            }
            /* A bare declaration with no initializer produces no TAC: it
             * only reserves an entry in the symbol table. */
            break;

        case ND_ASSIGN: {
            char *v = gen_expr(n->right);
            printf("%s = %s\n", n->resolved_name ? n->resolved_name : n->name, v);
            break;
        }

        case ND_IF:
            gen_if(n);
            break;

        case ND_WHILE:
            gen_while(n);
            break;

        case ND_PRINT: {
            char *v = gen_expr(n->right);
            printf("print %s\n", v);
            break;
        }

        case ND_BLOCK:
            for (int i = 0; i < n->stmt_count; i++) gen_stmt(n->stmts[i]);
            break;

        default:
            fprintf(stderr, "codegen: unexpected node %s used as statement\n",
                    nodetype_str(n->type));
    }
}