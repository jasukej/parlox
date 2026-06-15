#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    void* res = realloc(ptr, newSize);
    if (res == NULL) exit(1);
    return res;
}

/**
 * Free subsidiary heap allocations owned by this object e.g. char arrays,
 * upvalue arrays, chunks. Does NOT free the object header itself. Headers
 * live in the nursery and are reclaimed in bulk by nurseryFree().
 */
static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*) object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            break;
        }
        case OBJ_NATIVE:
            break;
        case OBJ_UPVALUE:
            break;
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;
            freeChunk(&function->chunk);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}

/**
 * Transition an object from white to gray (no-ops if marked): set isMarked and push 
 * it onto the gray stack so traceReferences() will later inspect its children. 
 */
void markObject(Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return;

    object->isMarked = true;
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = vm.grayCapacity < 8 ? 8 : vm.grayCapacity * 2;
        vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
        if (vm.grayStack == NULL) exit(1);
    }
    
    vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value) {
    if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

static void markTable(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        markObject((Obj*)entry->key);
        markValue(entry->value);
    }
}

/**
 * Mark every GC root: the value stack, call frame closures, open upvalues,
 * the globals table, and any in-progress compiler functions.
 */
static void markRoots() {
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }

    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj*)vm.frames[i].closure);
    }

    for (ObjUpvalue* uv = vm.openUpvalues; uv != NULL; uv = uv->next) {
        markObject((Obj*)uv);
    }

    markTable(&vm.globals);
    markCompilerRoots();
}

/**
 * Transition a gray object to black by marking all objects it references.
 */
static void blackenObject(Obj* object) {
    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            markObject((Obj*)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            markObject((Obj*)function->name);
            for (int i = 0; i < function->chunk.constants.count; i++) {
                markValue(function->chunk.constants.values[i]);
            }
            break;
        }
        case OBJ_UPVALUE:
            markValue(((ObjUpvalue*)object)->closed);
            break;
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

/**
 * Drain the gray stack to a fixpoint. Each popped object is blackened,
 * which may push new gray entries. Terminates when every reachable object
 * is black and every unreachable object remains white.
 */
static void traceReferences() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

/**
 * Run a full GC cycle: mark all reachable objects from roots, then trace
 * transitively. Called when the nursery bump allocator is exhausted.
 */
void collectGarbage() {
    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    // TODO: compactNursery() — copy survivors to toSpace, update pointers
}
