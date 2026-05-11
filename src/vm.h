#ifndef parlox_vm_h
#define parlox_vm_h

#include "chunk.h"
#include "table.h"
#include "object.h"
#include "nursery.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure* closure;
	uint8_t* ip; // in contrast to x86, the callee's return address is implicit
	Value* slots;
} CallFrame;

typedef struct {
	CallFrame frames[FRAMES_MAX];
	int frameCount;

	Value stack[STACK_MAX];
	Value* stackTop;
	// TODO: replace w a flat array indexed by compile-time slot number to avoid
	// hashing the variable name on every access
	Table globals;  // runtime variable store; maps variable name strings to their current values
	Table strings;  // interning table; deduplicates heap-allocated strings for fast equality
	ObjUpvalue* openUpvalues;
	Obj* objects;
	Nursery nursery;
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
