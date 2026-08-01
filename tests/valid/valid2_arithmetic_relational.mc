int a;
int b;
float c;
bool ok;

a = 7;
b = 3;
c = a + b * 2;      // int * int -> int, then int + int -> int, widened into float
ok = (a > b) && (b != 0);

print c;
print ok;
print a % b;
