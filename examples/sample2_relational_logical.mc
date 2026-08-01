// Demonstrates relational/logical operators and if-else.
int score;
bool passed;
bool honors;

score = 78;
passed = score >= 40;
honors = (score >= 80) && (score <= 100);

if (honors) {
    print score;
} else {
    if (passed) {
        print passed;
    } else {
        print 0;
    }
}
