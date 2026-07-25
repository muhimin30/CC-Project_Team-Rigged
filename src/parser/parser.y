%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"

extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

void yyerror(const char *s);

/* Root of the AST, filled in by the `program` rule and read by main(). */
Node *ast_root = NULL;

/* Number of syntax errors reported by yyerror(); main() checks this
 * before proceeding to semantic analysis / codegen. */
int syntax_error_count = 0;

static Node *mkbin(const char *op, Node *l, Node *r) {
    Node *n = new_node(ND_BINOP, l ? l->line : yylineno);
    n->op = strdup(op);
    n->left = l;
    n->right = r;
    return n;
}

static Node *mkun(const char *op, Node *operand) {
    Node *n = new_node(ND_UNOP, yylineno);
    n->op = strdup(op);
    n->left = operand;
    return n;
}
%}

%union {
    int    ival;
    double fval;
    char  *sval;
    struct Node *node;
}

%token <sval> ID
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token KW_INT KW_FLOAT KW_BOOL KW_IF KW_ELSE KW_WHILE KW_PRINT KW_TRUE KW_FALSE
%token OR AND NOT EQ NE LE GE

%type <node> program stmt_list stmt block
%type <node> decl_stmt assign_stmt if_stmt while_stmt print_stmt expr

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left '+' '-'
%left '*' '/' '%'
%right NOT UMINUS

%%

program:
    stmt_list { $1->type = ND_PROGRAM; ast_root = $1; }
    ;

stmt_list:
      /* empty */            { $$ = new_node(ND_BLOCK, 0); }
    | stmt_list stmt         { add_stmt($1, $2); $$ = $1; }
    | stmt_list error ';'    {
                                  fprintf(stderr,
                                      "Syntax Error (line %d): skipping malformed statement, resuming after ';'\n",
                                      yylineno);
                                  yyerrok;
                                  $$ = $1;
                              }
    ;

block:
    '{' stmt_list '}' { $$ = $2; /* stmt_list already builds an ND_BLOCK */ }
    ;

stmt:
      decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    ;

decl_stmt:
      KW_INT   ID ';'          { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("int");   $$->name = $2; }
    | KW_FLOAT ID ';'          { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("float"); $$->name = $2; }
    | KW_BOOL  ID ';'          { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("bool");  $$->name = $2; }
    | KW_INT   ID '=' expr ';' { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("int");   $$->name = $2; $$->right = $4; }
    | KW_FLOAT ID '=' expr ';' { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("float"); $$->name = $2; $$->right = $4; }
    | KW_BOOL  ID '=' expr ';' { $$ = new_node(ND_DECL, yylineno); $$->decl_type_name = strdup("bool");  $$->name = $2; $$->right = $4; }
    ;

assign_stmt:
    ID '=' expr ';' { $$ = new_node(ND_ASSIGN, yylineno); $$->name = $1; $$->right = $3; }
    ;

if_stmt:
      KW_IF '(' expr ')' stmt                { $$ = new_node(ND_IF, yylineno); $$->left = $3; $$->right = $5; }
    | KW_IF '(' expr ')' stmt KW_ELSE stmt   { $$ = new_node(ND_IF, yylineno); $$->left = $3; $$->right = $5; $$->third = $7; }
    ;

while_stmt:
    KW_WHILE '(' expr ')' stmt { $$ = new_node(ND_WHILE, yylineno); $$->left = $3; $$->right = $5; }
    ;

print_stmt:
    KW_PRINT expr ';' { $$ = new_node(ND_PRINT, yylineno); $$->right = $2; }
    ;

expr:
      expr OR expr        { $$ = mkbin("||", $1, $3); }
    | expr AND expr       { $$ = mkbin("&&", $1, $3); }
    | expr EQ expr        { $$ = mkbin("==", $1, $3); }
    | expr NE expr        { $$ = mkbin("!=", $1, $3); }
    | expr LT expr        { $$ = mkbin("<",  $1, $3); }
    | expr GT expr        { $$ = mkbin(">",  $1, $3); }
    | expr LE expr        { $$ = mkbin("<=", $1, $3); }
    | expr GE expr        { $$ = mkbin(">=", $1, $3); }
    | expr '+' expr       { $$ = mkbin("+",  $1, $3); }
    | expr '-' expr       { $$ = mkbin("-",  $1, $3); }
    | expr '*' expr       { $$ = mkbin("*",  $1, $3); }
    | expr '/' expr       { $$ = mkbin("/",  $1, $3); }
    | expr '%' expr       { $$ = mkbin("%",  $1, $3); }
    | NOT expr             { $$ = mkun("!", $2); }
    | '-' expr %prec UMINUS { $$ = mkun("-", $2); }
    | '(' expr ')'        { $$ = $2; }
    | ID                   { $$ = new_node(ND_ID, yylineno); $$->name = $1; }
    | INT_LIT              { $$ = new_node(ND_INT_LIT, yylineno); $$->int_val = $1; }
    | FLOAT_LIT            { $$ = new_node(ND_FLOAT_LIT, yylineno); $$->float_val = $1; }
    | KW_TRUE              { $$ = new_node(ND_BOOL_LIT, yylineno); $$->bool_val = 1; }
    | KW_FALSE             { $$ = new_node(ND_BOOL_LIT, yylineno); $$->bool_val = 0; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error (line %d): %s near '%s'\n", yylineno, s, yytext);
    syntax_error_count++;
}
