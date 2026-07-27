#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"

/* Walks the AST, builds/consults the symbol table, annotates every
 * expression node with its resolved DataType, and reports errors
 * (undeclared use, redeclaration, scope violation, type mismatch,
 * invalid assignment / expression) to stderr with line numbers.
 *
 * Returns the number of semantic errors found. 0 means the AST is safe
 * to hand to the intermediate-code generator. */
int semantic_analyze(Node *program);

#endif /* SEMANTIC_H */
