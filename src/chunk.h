#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE, // define explicitly instead of loading through OP_CONSTANT
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
} OpCode;

typedef struct {
    int count;
    int line;
} LineInfo;

/**
 * A compiled unit of bytecode. `code` is a flat array of bytes where each byte
 * is either an opcode or an operand. Instructions are variable-width:
 *
 *   1-byte instructions (e.g. OP_ADD): just the opcode.
 *   2-byte instructions (e.g. OP_CONSTANT): opcode + operand (1-byte index into `constants`).
 *
 * Example: the expression `1 + 2` compiles to:
 *
 *   code:      [OP_CONSTANT][0][OP_CONSTANT][1][OP_ADD]
 *   constants: [1.0, 2.0]
 *
 * The VM reads bytes sequentially via an instruction pointer (vm.ip).
 */
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int linesCount;
    int linesCapacity;
    LineInfo* lines;
    ValueArray constants; // compile-time pool of literal values; accessed by index from bytecode operands
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
int getLine(Chunk* chunk, int idx);

#endif
