# Formal Grammar (CFG)

This is the context-free grammar implemented in `src/parser/parser.y`,
written in BNF-ish notation for the project report (Section 12.6 "Parser
Design" of the Project Manual). Terminals are `UPPERCASE` or quoted
literals; non-terminals are `lowercase`.

```
program        -> stmt_list

stmt_list      -> stmt_list stmt
                |  /* empty */

block          -> '{' stmt_list '}'

stmt           -> decl_stmt
                |  assign_stmt
                |  if_stmt
                |  while_stmt
                |  print_stmt
                |  block

decl_stmt      -> TYPE ID ';'
                |  TYPE ID '=' expr ';'

TYPE           -> 'int' | 'float' | 'bool'

assign_stmt    -> ID '=' expr ';'

if_stmt        -> 'if' '(' expr ')' stmt
                |  'if' '(' expr ')' stmt 'else' stmt

while_stmt     -> 'while' '(' expr ')' stmt

print_stmt     -> 'print' expr ';'

expr           -> expr '||' expr
                |  expr '&&' expr
                |  expr '==' expr
                |  expr '!=' expr
                |  expr '<'  expr
                |  expr '>'  expr
                |  expr '<=' expr
                |  expr '>=' expr
                |  expr '+'  expr
                |  expr '-'  expr
                |  expr '*'  expr
                |  expr '/'  expr
                |  expr '%'  expr
                |  '!' expr
                |  '-' expr            %prec UMINUS
                |  '(' expr ')'
                |  ID
                |  INT_LIT
                |  FLOAT_LIT
                |  'true'
                |  'false'
```

## Operator precedence (lowest to highest)

Declared in `parser.y` from lowest to highest binding power, matching
common C-family conventions:

| Level (low -> high) | Operators        | Associativity |
|----------------------|-------------------|---------------|
| 1                     | `\|\|`              | left          |
| 2                     | `&&`               | left          |
| 3                     | `==`  `!=`         | left          |
| 4                     | `<` `>` `<=` `>=`  | left          |
| 5                     | `+`  `-`           | left          |
| 6                     | `*` `/` `%`        | left          |
| 7                     | `!`, unary `-`     | right         |

## Ambiguity: dangling else

The grammar as written is ambiguous for
```
if (a) if (b) s1; else s2;
```
because it is not clear whether `else` belongs to the inner or outer `if`.
Bison reports this as **one shift/reduce conflict** and resolves it, by
default, in favour of shift — i.e. the parser always attaches an `else` to
the *nearest* unmatched `if`. This is the same rule every C-family
language uses, so the grammar is left as-is rather than rewritten into the
(more verbose) unambiguous "matched/unmatched statement" form.

## Tokens produced by the lexer

| Token       | Pattern                              |
|-------------|----------------------------------------|
| `ID`        | `[a-zA-Z_][a-zA-Z0-9_]*` (minus keywords) |
| `INT_LIT`   | `[0-9]+`                                |
| `FLOAT_LIT` | `[0-9]+\.[0-9]+`                        |
| keywords    | `int float bool if else while print true false` |
| operators   | `+ - * / % < > <= >= == != && \|\| !`   |
| delimiters  | `{ } ( ) ; =`                           |
| comments    | `// ...` and `/* ... */` (discarded)    |
| whitespace  | spaces / tabs / newlines (discarded)    |

Any character that does not match one of the rules above is reported as a
**lexical error** with its line number, and scanning continues.
