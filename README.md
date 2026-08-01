# Mini Language Compiler — CSE 416 Compiler Construction Lab Project

Metropolitan University, Bangladesh — Department of CSE

A compiler front-end for the fixed mini language defined in Section 5 of the
Project Manual. It implements the full required pipeline:

```
Source (.mc) --> Lexer (Flex) --> Parser (Bison) --> AST --> Semantic
Analyzer (Symbol Table + Type Checking) --> TAC Code Generator --> TAC output
```

See `docs/architecture.md` for a walkthrough of every module and
`docs/grammar.md` for the formal grammar (CFG) implemented by `parser.y`.

## 1. Requirements

- Linux (Ubuntu/Debian) or WSL on Windows
- `gcc`, `flex`, `bison`, `make`

Install on Ubuntu/WSL:
```bash
sudo apt update
sudo apt install -y gcc flex bison make
```

## 2. Build

From the project root:
```bash
make
```
This runs Bison on `src/parser/parser.y`, Flex on `src/lexer/lexer.l`,
compiles everything, and links `bin/mycompiler`. Generated files
(`parser.tab.c/h`, `lex.yy.c`, `*.o`) are written to `build/` and are not
checked into git (see `.gitignore`).

## 3. Run

```bash
./bin/mycompiler path/to/program.mc
```
or
```bash
make run FILE=examples/sample1.mc
```

The compiler prints, in order:
1. The Abstract Syntax Tree (indented text form)
2. Semantic analysis result (errors, if any, with line numbers)
3. The generated Three-Address Code (TAC)

If lexical or syntax errors are found, compilation stops before semantic
analysis. If semantic errors are found, compilation stops before code
generation. Exit code is `0` on full success, `1` otherwise.

## 4. Run the test suite

```bash
make
./tests/run_tests.sh
```
This compiles every program in `tests/valid/` and `tests/invalid/` and
diffs the output against the checked-in `*.expected_output.txt` files.

## 5. Project layout

```
project-root/
├── docs/                 grammar spec, architecture notes, report template
├── src/
│   ├── lexer/lexer.l      Flex scanner
│   ├── parser/parser.y    Bison grammar + AST construction
│   ├── ast/               AST node definitions + printer
│   ├── symbol_table/      scoped symbol table
│   ├── semantic/          type checking / scope checking
│   ├── codegen/           Three-Address-Code generator
│   └── main.c             driver: wires all phases together
├── tests/
│   ├── valid/             programs that must compile cleanly
│   └── invalid/           programs that must be rejected, with reasons
├── examples/              small demo programs
├── Makefile
└── README.md
```

## 6. Language quick reference

| Category    | Details |
|-------------|---------|
| Types       | `int`, `float`, `bool` |
| Declarations| `int x;`  or  `int x = 5;` |
| Assignment  | `x = expr;` |
| Arithmetic  | `+ - * / %` |
| Relational  | `< > <= >= == !=` |
| Logical     | `&& \|\| !` |
| Control     | `if (cond) { ... } else { ... }`, `while (cond) { ... }` |
| I/O         | `print expr;` |
| Blocks      | `{ ... }` introduce a new nested scope |
| Comments    | `// line comment`, `/* block comment */` |
| Booleans    | `true`, `false` |

See `examples/` for complete sample programs and `docs/grammar.md` for the
full grammar.

## 7. Known / accepted limitations

- Bison reports **1 shift/reduce conflict**: the classic "dangling else"
  ambiguity (`if (a) if (b) s1 else s2`). Bison resolves it by default
  (shift), which matches every C-family language: the `else` binds to the
  nearest unmatched `if`. This is expected and documented, not a bug.
- Logical `&&` / `||` are **not short-circuited** in the generated TAC
  (both operands are always evaluated); this keeps the intermediate code
  generator simple, which is appropriate for this course's scope (see
  Section 6 of the Project Manual — code optimization is out of scope).
- Variables declared inside nested blocks are given a unique internal name
  in the TAC (e.g. an inner shadowing `x` becomes `x__1`) purely so the
  flat TAC namespace doesn't collide with the outer `x`; this does not
  change source-level semantics or scoping rules.
