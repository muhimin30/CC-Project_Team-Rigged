#include <stdio.h>
#include <stdlib.h>
#include "ast/ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

extern FILE *yyin;
extern int yyparse(void);
extern Node *ast_root;
extern int syntax_error_count;
extern int lexical_error_count;

static void print_banner(const char *title) {
    printf("\n============================================================\n");
    printf(" %s\n", title);
    printf("============================================================\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    printf("Compiling '%s' ...\n", argv[1]);

    int parse_status = yyparse();
    fclose(yyin);

    if (lexical_error_count > 0) {
        fprintf(stderr, "\n%d lexical error(s) found.\n", lexical_error_count);
    }

    if (parse_status != 0 || syntax_error_count > 0 || ast_root == NULL) {
        fprintf(stderr, "\nCompilation failed: %d syntax error(s) found. Stopping before semantic analysis.\n",
                syntax_error_count);
        return 1;
    }

    if (lexical_error_count > 0) {
        fprintf(stderr, "Compilation failed due to lexical errors. Stopping before semantic analysis.\n");
        return 1;
    }

    print_banner("ABSTRACT SYNTAX TREE");
    print_ast(ast_root, 0);

    print_banner("SEMANTIC ANALYSIS");
    int sem_errors = semantic_analyze(ast_root);
    if (sem_errors > 0) {
        fprintf(stderr, "\nCompilation failed: %d semantic error(s) found.\n", sem_errors);
        return 1;
    }
    printf("No semantic errors found.\n");

    print_banner("THREE ADDRESS CODE (TAC)");
    generate_tac(ast_root);

    printf("\nCompilation finished successfully.\n");
    return 0;
}
