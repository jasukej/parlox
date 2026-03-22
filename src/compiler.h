#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"

// Compiles program from source to bytecode, and stores it in given chunk.
// Returns whether compilation succeeded.
bool compile(const char* source, Chunk* chunk);

#endif
