# Binterpreter
A simple bytecode interpreter with a compiler frontend. Largely based off Bob Nystrom's clox in the book Crafting Interpreters, with more advanced garbage collection (Generational GC) to better simulate modern runtimes.

## Prerequisites
- `gcc`
- `make`

## Build
```bash
make
```
The build output can be found in `out/binterpreter`. The executable current runs as a REPL.

### Running a source file
WIP

### Debugging
Compile with the following to:
- Print compiled bytecode chunk: `make CFLAGS="-Wall -Wextra -Isrc -g -fsanitize=address -DDEBUG_PRINT_CODE"`
- Trace VM execution: `make CFLAGS="-Wall -Wextra -Isrc -g -fsanitize=address -DDEBUG_TRACE_EXECUTION`
