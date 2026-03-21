#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
} OpCode;

typedef struct {
    int count;
    int line;
} LineInfo;

/**
* A dynamic array to allocate in memory. 
* For now, we maintain line numbering in our program through another allocation
*/
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int linesCount;
    int linesCapacity;
    LineInfo* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
int getLine(Chunk* chunk, int idx);

#endif
