// 'count' only exists inside the while block; using it afterwards is
// a scope violation.
int n;
n = 3;
while (n > 0) {
    int count;
    count = n;
    n = n - 1;
}
print count;
