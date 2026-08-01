// Sum of even numbers from 1 to n, demonstrating nested control flow.
int n;
int i;
int sum;

n = 10;
i = 1;
sum = 0;

while (i <= n) {
    if (i % 2 == 0) {
        sum = sum + i;
    }
    i = i + 1;
}

print sum;
