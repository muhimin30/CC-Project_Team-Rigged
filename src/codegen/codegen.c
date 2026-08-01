 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static int temp_count = 0;
static int label_count = 0;

static char *new_temp(void) {
    char *buf = (char *)malloc(16);
    snprintf(buf, 16, "t%d", ++temp_count);
    return buf;
}

static char *new_label(void) {
    char *buf = (char *)malloc(16);
    snprintf(buf, 16, "L%d", ++label_count);
    return buf;
}

static char *int_literal_str(int v) {
    char *buf = (char *)malloc(32);
    snprintf(buf, 32, "%d", v);
    return buf;
}

static char *float_literal_str(double v) {
    char *buf = (char *)malloc(32);
    snprintf(buf, 32, "%g", v);
    return buf;
}

static void gen_stmt(Node *n);