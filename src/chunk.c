#include <stdint.h>
#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
#include "value.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->linesCount = 0;
    chunk->linesCapacity = 0;
    initValueArray(&chunk->constants);
    chunk->lines = NULL;
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;

    if (chunk->linesCapacity < chunk->linesCount + 1) {
        int oldCapacity = chunk->linesCapacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->lines = GROW_ARRAY(LineInfo, chunk->lines, oldCapacity, chunk->linesCapacity);
    }
    
    if (chunk->lines[chunk->linesCount-1].line != line) {
        chunk->linesCount++;
    } 
    chunk->lines[chunk->linesCount].count++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(LineInfo, chunk->lines, chunk->linesCapacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value val) {
    writeValueArray(&chunk->constants,  val);
    return chunk->constants.count - 1;
}

// Given index of an instruction, return line where instruction occurs
int getLine(Chunk* chunk, int idx) {
    int i = 0;
    while (i < chunk->linesCount && chunk->lines[i].line != idx) {
        i++;
    }
    return i;
}
