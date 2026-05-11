#include "memory.h"
#include "nursery.h"

/**
 * Return the nursery pointer and bump it by its size.
 */
void* nurseryAlloc(Nursery* n, size_t size) {
    size = ALIGN_UP(size, 8);
    if (n->ptr + size > n->limit) return NULL;
    void* result = n->ptr;
    n->ptr += size;
    return result;
}