# Compiler Architecture

## Pipeline

```
 program.mc
     │
     ▼
┌───────────────┐   yylex()      ┌────────────────┐
│ Lexer (Flex)  │ ─────────────► │ Parser (Bison)  │
│ lexer.l       │  tokens        │ parser.y        │
└───────────────┘                └────────┬────────┘
                                           │ builds
                                           ▼
                                   ┌───────────────┐
                                   │      AST      │  (src/ast)
                                   └───────┬───────┘
                                           │ walked by
                                           ▼
                          ┌─────────────────────────────────┐
                          │  Semantic Analyzer               │
                          │  (src/semantic + src/symbol_table)│
                          │  - builds scoped symbol table     │
                          │  - type-checks every expression   │
                          │  - annotates AST nodes with types │
                          └───────────────┬──────────────────┘
                                          │ 0 errors?
                                          ▼
                          ┌─────────────────────────────────┐
                          │  TAC Code Generator (src/codegen)│
                          └───────────────┬──────────────────┘
                                          ▼
                                Three-Address Code (stdout)
```

`src/main.c` is the driver: it opens the input file, calls `yyparse()`,
then runs each later phase only if the previous one produced zero errors.

## Module responsibilities

### `src/lexer/lexer.l`
Flex scanner. Recognizes keywords, identifiers, integer/float literals,
operators, delimiters, and both comment styles. Uses `%option yylineno`
so every token knows its source line. Unrecognized characters are
reported as lexical errors and skipped (scanning continues so multiple
errors can be reported in one pass, mirroring how GCC reports errors).

### `src/parser/parser.y`
Bison grammar. Builds the AST directly in the semantic actions (there is
no separate "parse tree" stage — the parser produces the AST straight
away, which is the structure required by Section 4.3 of the manual).
Operator precedence/associativity is declared with `%left` / `%right`
so the grammar itself stays unambiguous for arithmetic/logical
expressions. A generic `stmt_list: stmt_list error ';'` rule gives basic
syntax-error recovery: a malformed statement is skipped up to its
terminating `;` and parsing continues, so later errors are still
reported instead of stopping at the first one.

### `src/ast/`
One generic `Node` struct represents every construct (statements and
expressions alike); the `NodeType` tag says which fields are meaningful.
`print_ast()` renders the tree as indented text (Section 4.3 requires the
AST be printable; Graphviz visualization is listed as an optional bonus
feature in Section 14 and is not implemented here).

### `src/symbol_table/`
A **stack of scopes**, each holding a linked list of `SymEntry` records
(`name`, `type`, `scope_level`, `line_declared`). `symtab_enter_scope()`
/ `symtab_exit_scope()` are called whenever the semantic analyzer enters
or leaves a `{ ... }` block, so declarations inside a block are
automatically invisible once that block ends — this is what implements
nested scoping and lets the analyzer detect scope violations "for free"
(using a variable after its block ends is just an ordinary "undeclared
variable" lookup failure).

Every declared variable also gets a **mangled name**
(`symtab_declare()` in `symbol_table.c`): globals keep their original
name, but anything declared inside a nested block is renamed internally
(e.g. `x` becomes `x__3`) so two different variables that happen to
share a source name in different scopes never collide once they reach
the flat TAC namespace. This mangled name is only used by the code
generator — the AST printer and error messages always show the
programmer's original spelling.

### `src/semantic/semantic.c`
Recursively walks the AST:
- `analyze_stmt()` handles declarations, assignments, `if`, `while`,
  `print`, and blocks (pushing/popping scope for blocks).
- `analyze_expr()` handles literals, identifiers, unary/binary
  operators, resolves each identifier via the symbol table, computes
  and stores each expression's `DataType` on the node, and reports:
  - undeclared-variable use
  - redeclaration in the same scope
  - scope violations (via lookup failure after a block closes)
  - type mismatches on assignment/initialization
  - invalid operand types for arithmetic / relational / logical
    operators
  - non-bool `if` / `while` conditions

`semantic_analyze()` returns the total error count; `main.c` only
proceeds to code generation if it is `0`.

### `src/codegen/codegen.c`
Walks the (now type-annotated) AST a second time and emits
Three-Address Code:
- `x = y op z` for binary operators
- `x = op y` for unary operators (unary minus prints as `uminus`)
- `x = y` for assignments/initializers
- `print y`
- `if`/`if-else` lowered to `ifFalse ... goto L` / `goto L` / labels
- `while` lowered to a labelled loop that re-evaluates its condition
  every iteration (see `docs/grammar.md` and Lab 4's background theory
  for the exact patterns used)

No optimization passes (constant folding, dead-code elimination, etc.)
are applied — Section 6 of the Project Manual explicitly places
optimization out of scope for the mandatory deliverable; it is listed
only as an optional bonus feature (Section 14).
