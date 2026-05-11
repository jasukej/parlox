## Problem
High-level language interpreters face a dichotomy between single-threaded allocation throughput and scaling to multi-core architectures. CPython exemplifies this architectural constraint, relying on reference counting for memory management, which requires constant mutation of object headers upon creation, assignment, and destruction. Because these mutations are inherently thread-unsafe, CPython relies on the Global Interpreter Lock (GIL), allowing only a single thread to execute bytecode at a time. **While the GIL ensures the atomicity of individual instructions, it severely bottlenecks multi-core performance.**

As concurrent execution becomes an essential feature, the Python ecosystem is actively attempting to fit multi-threading capabilities. With the introduction of "free-threaded" builds in CPython 3.13+, the GIL is made optional. However, this shift breaks many legacy C extensions (ie. `numpy` and custom DB drivers) that assume single-threaded memory access. Current solutions highlight Rust-based tools like `Py03`, which take advantage of Rust's `Send` and `Sync` traits to guarantee thread safety in CPython extensions without relying on a GIL.

We aim to design a bytecode interpreter that avoids CPython's concurrency pitfalls by swapping C-based ref-counting in favor of a thread-safe, tracing Garbage Collector (GC) managed in Rust. Coupling a C execution engine with a Rust GC introduces the Foreign Function Interface (FFI) boundary as a throughput bottleneck which negates the advantages of writing the execution loop in C. **The core problem this design addresses is how to couple a cross-language memory model that provides the memory safety and concurrent garbage collection of Rust, without incurring FFI latency on the critical path of C-side object allocation.**

## Goal and Non-goals
Goals:
* Isolate memory state management, where the C runtime executes bytecode and object tracing, copying, and tenuring falls on the Rust GC.
* Maintain performance: Implement a bump allocator in C that allows zero-overhead allocation on the hot path (object creation) without crossing the FFI boundary.
* Prove thread-safe extensibility: Provide a mechanism where native language extensions can be written in Rust. 

Non-goals: 
* Just-In-Time (JIT) compilation: The design focuses strictly on optimizing a bytecode interpreter and cross-language memory management.
* A comprehensive standard library: The focus will lie on the core VM architecture, implementing only the minimal built-in types (e.g. arrays, hashmaps) to prove the outlined concepts.

## Pipeline
A brief overview of the bytecode interpreter:
![VM Memory Architecture](assets/architecture.drawio.svg)
1. Frontend: Source code is ingested by a lexer and top-down operator precedence (Pratt) parser, and outputted as flat bytecode arrays (chunks).
2. Execution Loop: The compiled bytecode is fed into the C Virtual Machine (VM). The VM operates a highly optimized, single-threaded dispatch loop. It tracks live object references in a contiguous stack to identify root nodes. 
3. Fast-Path Allocation: When an object is created, the VM uses a C-side bump allocator to increment a pointer within a pre-allocated memory space (the Nursery). This avoids expensive cross-language calls during normal execution.
4. GC FFI Bridge: Once the C-side Nursery pointer reaches its limit, the fast path fails. The C VM pauses execution and invokes the Rust GC via FFI, passing the roots from the VM and the filled Nursery arena.
5. Memory Management: Rust-based GC traces the passed roots, identifies which objects are still alive within the Nursery and promotes (copies) them into a "Tenured" memory space. Rust clears the Nursery arena and hands the fresh memory pointers back to C, upon which the VM resumes the execution loop.

## Proposed Solution
The interpreter replaces its memory management with a split C/Rust model. The C VM performs zero-overhead object allocation via a bump allocator within a pre-allocated Nursery arena, avoiding any FFI calls on the hot path. When the Nursery fills, the VM pauses and crosses the FFI boundary once in bulk, handing a shadow-stack root set and the full arena to a Rust-based tracing GC that copies live objects into a tenured heap and returns fresh pointers. This design amortizes the cost of FFI by batching allocations on the C side while keeping all mutable memory state management in Rust. Without a GIL, the `Send`/`Sync` traits are implemented to enforce thread safety.

### What is a GC?
A Garbage Collector (GC) automates memory lifecycle management, preventing the dangling pointers and memory leaks common in manual C-based memory management. While early collectors relied on full-heap mark-and-sweep phases (which induce severe latency), this architecture implements a Generational Garbage Collector.

This design leverages the Generational Hypothesis (Ungar, 1984), which observes that "most objects die young." By partitioning the heap into a Nursery (for new allocations) and a Tenured Space (for long-lived objects), the VM avoids scanning the entire heap on every collection cycle. The trade-off: Generational collectors require complex state management and write barriers to track references between old and new objects. We mitigate this complexity by safely isolating the tenuring and compaction logic entirely within Rust.

### Deep-Copy Message Passing
By abandoning thread-unsafe reference counting, this architecture unlocks true multi-core execution via the Actor Model (Hewitt et al., 1973; popularized by Erlang). Instead of sharing memory and relying on a Global Interpreter Lock, individual C VMs operate as independent actors. Concurrency is achieved through deep-copy message passing, where objects sent between threads are recursively copied into the receiving VM's heap.

Deep copying incurs a CPU and memory overhead proportional to the message size. However, literature (Armstrong, 2007) demonstrates that lock-free, shared-nothing architectures vastly outperform shared-memory models in highly concurrent, distributed-style workloads by eliminating lock contention and cache invalidation bottlenecks.

### Root tracking
A **root** in this context is a reference to a memory address that the system knows is in use. Currently, roots live implicitly in the VM; on the value stack, the globals hash table, and the linked list of open upvalues, as well as compiler temporaries. Anything not referenced by a root is immediately freed -- this is the GC's function.

To facilitate this memory and concurrency model without incurring limiting FFI overhead, it is proposed that the execution loop should be strictly decoupled from the collection logic. 

To fulfill these requirements, the design choices made cover the implementation of:

1. **Bump allocator**: Allocating an object in the Nursery requires only a single pointer addition. This maintains maximum throughput.
2. **Root tracking with a shadow stack**: To provide the Rust GC with an immutable map of live memory, the C VM implements a Shadow Stack (Henderson, 2002). As the bytecode VM pushes object references onto its virtual execution stack, it simultaneously records these pointers in a contiguous C-array (the root set).
3. **A FFI collection boundary**: The C VM only crosses the FFI boundary when the Nursery is exhausted. The VM yields execution, passing the Shadow Stack and Nursery pointers to Rust. Rust safely executes the complex tracing, copying, and tenuring logic, compacts the survivors, and returns a zeroed Nursery back to the C execution context.

Alternative considered: We rejected conservative stack scanning (e.g., the Boehm GC approach), which scans the raw C call stack for pointer-like values. While a shadow stack introduces a slight overhead to PUSH/POP instructions, conservative scanning is platform-dependent, and prone to memory leaks via false positives.

## Core Data Model
TODO

## Invariants
TODO
