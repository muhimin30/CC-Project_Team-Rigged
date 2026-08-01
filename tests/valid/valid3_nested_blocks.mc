// Nested blocks and shadowing: the inner 'x' is a different variable
// from the outer 'x' and only exists inside the block.
int x;
x = 1;

if (x == 1) {
    int x;
    x = 99;
    print x;   // prints 99 (inner x)
}

print x;       // prints 1 (outer x, unaffected)
