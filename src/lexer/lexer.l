/* Flex scanner source. */

%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "parser.tab.h"

int lexical_error_count = 0;
%}

DIGIT   [0-9]
LETTER  [a-zA-Z_]
ID      {LETTER}({LETTER}|{DIGIT})*
INT     {DIGIT}+
FLOAT   {DIGIT}+"."{DIGIT}+

%%

"//".*                 { /* single-line comment: discard */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* block comment: discard */ }

"int"                   { return KW_INT; }
"float"                 { return KW_FLOAT; }
"bool"                  { return KW_BOOL; }
"if"                    { return KW_IF; }
"else"                  { return KW_ELSE; }
"while"                 { return KW_WHILE; }
"print"                 { return KW_PRINT; }
"true"                  { return KW_TRUE; }
"false"                 { return KW_FALSE; }

{FLOAT}                 { yylval.fval = atof(yytext); return FLOAT_LIT; }
{INT}                   { yylval.ival = atoi(yytext); return INT_LIT; }
{ID}                    { yylval.sval = strdup(yytext); return ID; }

"&&"                    { return AND; }
"||"                    { return OR; }
"!"                     { return NOT; }
"=="                    { return EQ; }
"!="                    { return NE; }
"<="                    { return LE; }
">="                    { return GE; }
"<"                     { return LT; }
">"                     { return GT; }

"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"%"                     { return '%'; }
"="                     { return '='; }
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
";"                     { return ';'; }

[ \t\r]+                { /* skip whitespace */ }
\n                      { /* yylineno auto-increments; nothing else to do */ }

.                       {
                            fprintf(stderr,
                                "Lexical Error (line %d): unexpected character '%s'\n",
                                yylineno, yytext);
                            lexical_error_count++;
                        }

%%
