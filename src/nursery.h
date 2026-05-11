#ifndef parlox_nursery_h
#define parlox_nursery_h

#include "common.h"

#define NURSERY_SIZE (1 << 20)
// start on the next [align]-byte boundary
#define ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))

typedef struct {
    uint8_t* fromSpace; // currently-allocated-to half
    uint8_t* toSpace;   // copy destination during GC
    uint8_t* ptr;       // bump pointer within fromSpace
    uint8_t* limit;     // fromSpace + NURSERY_SIZE
} Nursery;

void nurseryInit(Nursery* n);
void* nurseryAlloc(Nursery* n, size_t size);
void nurseryFree(Nursery* n);

#endif