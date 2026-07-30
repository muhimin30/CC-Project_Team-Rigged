#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Node *new_node(NodeType type, int line) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) {
        fprintf(stderr, "Fatal: out of memory while building AST\n");
        exit(1);
    }
    n->type = type;
    n->line = line;
    n->data_type = TYPE_UNKNOWN;
    return n;
}

void add_stmt(Node *block, Node *stmt) {
    if (!block || !stmt) return;
    if (block->stmt_count >= block->stmt_cap) {
        block->stmt_cap = block->stmt_cap == 0 ? 8 : block->stmt_cap * 2;
        block->stmts = (Node **)realloc(block->stmts, sizeof(Node *) * block->stmt_cap);
    }
    block->stmts[block->stmt_count++] = stmt;
}

const char *datatype_str(DataType t) {
    switch (t) {
        case TYPE_INT:   return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL:  return "bool";
        case TYPE_VOID:  return "void";
        default:         return "unknown";
    }
}

const char *nodetype_str(NodeType t) {
    switch (t) {
        case ND_PROGRAM:   return "Program";
        case ND_BLOCK:     return "Block";
        case ND_DECL:      return "Decl";
        case ND_ASSIGN:    return "Assign";
        case ND_IF:        return "If";
        case ND_WHILE:     return "While";
        case ND_PRINT:     return "Print";
        case ND_BINOP:     return "BinOp";
        case ND_UNOP:      return "UnOp";
        case ND_ID:        return "Id";
        case ND_INT_LIT:   return "IntLit";
        case ND_FLOAT_LIT: return "FloatLit";
        case ND_BOOL_LIT:  return "BoolLit";
        default:           return "?";
    }
}

static void indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
}

void print_ast(Node *n, int depth) {
    if (!n) return;
    indent(depth);

    switch (n->type) {
        case ND_PROGRAM:
        case ND_BLOCK:
            printf("[%s]\n", nodetype_str(n->type));
            for (int i = 0; i < n->stmt_count; i++)
                print_ast(n->stmts[i], depth + 1);
            break;

        case ND_DECL:
            printf("[Decl] %s %s (line %d)\n", n->decl_type_name, n->name, n->line);
            if (n->right) print_ast(n->right, depth + 1);
            break;

        case ND_ASSIGN:
            printf("[Assign] %s = (line %d)\n", n->name, n->line);
            print_ast(n->right, depth + 1);
            break;

        case ND_IF:
            printf("[If] (line %d)\n", n->line);
            indent(depth + 1); printf("cond:\n");
            print_ast(n->left, depth + 2);
            indent(depth + 1); printf("then:\n");
            print_ast(n->right, depth + 2);
            if (n->third) {
                indent(depth + 1); printf("else:\n");
                print_ast(n->third, depth + 2);
            }
            break;

        case ND_WHILE:
            printf("[While] (line %d)\n", n->line);
            indent(depth + 1); printf("cond:\n");
            print_ast(n->left, depth + 2);
            indent(depth + 1); printf("body:\n");
            print_ast(n->right, depth + 2);
            break;

        case ND_PRINT:
            printf("[Print] (line %d)\n", n->line);
            print_ast(n->right, depth + 1);
            break;

        case ND_BINOP:
            printf("[BinOp %s] (line %d)\n", n->op, n->line);
            print_ast(n->left, depth + 1);
            print_ast(n->right, depth + 1);
            break;

        case ND_UNOP:
            printf("[UnOp %s] (line %d)\n", n->op, n->line);
            print_ast(n->left, depth + 1);
            break;

        case ND_ID:
            printf("[Id] %s (line %d)\n", n->name, n->line);
            break;

        case ND_INT_LIT:
            printf("[IntLit] %d (line %d)\n", n->int_val, n->line);
            break;

        case ND_FLOAT_LIT:
            printf("[FloatLit] %g (line %d)\n", n->float_val, n->line);
            break;

        case ND_BOOL_LIT:
            printf("[BoolLit] %s (line %d)\n", n->bool_val ? "true" : "false", n->line);
            break;
    }
}
