#include <stdlib.h>
#include <stdio.h>

void* xmalloc(size_t size) {
    void* p = malloc(size);
    if (!p) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    return p;
}

void* xcalloc(size_t n, size_t size) {
    void* p = calloc(n, size);
    if (!p) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    return p;
}
