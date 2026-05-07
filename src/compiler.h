#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"
#include "vm.h"

// Compiles program from source to bytecode, and stores it in given chunk.
// Returns the associated function object with bytecode and metadata.
ObjFunction* compile(const char* source, Chunk* chunk);

#endif