#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "table.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
	Chunk* chunk;
	uint8_t* ip; // instruction pointer
	Value stack[STACK_MAX];
	Value* stackTop;
	// TODO: replace w a flat array indexed by compile-time slot number to avoid
	// hashing the variable name on every access
	Table globals;  // runtime variable store; maps variable name strings to their current values
	Table strings;  // interning table; deduplicates heap-allocated strings for fast equality
	Obj* objects;
} VM;

typedef enum {
	INTERPRET_OK, 
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

// We declare a single global VM for simplicity. In real-language implementations where
// VMs are embedded in other host applications, it is much easier to take a VM pointer.
void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
