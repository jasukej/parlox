Though most of these mentioned features are already defined as part of clox (in turn, inspired by Lua), I wanted to highlight the key decisions made by both Nystrom (2021) and additional features I wanted to implement, specifically in comparison with other modern languages.

## Pratt Parsing
Perhaps my favourite language design concept! Pratt breaks down the convoluted art of parsing into a method dispatch model that executes a prefix and/or infix evaluator for each token (produced from lexing the source code).

The core loop lives in `parsePrecedence`, which takes an argument of the minimum precedence level a token can accept (`minPrecedence`).
1. Consume a token and loop up its prefix handler, and return a syntax error if none exists.
2. In a while loop, and consume the next token as long as the its precedence >= `minPrecedence`. Then call its infix handler.
3. Each infix handler recursively calls the root function `parsePrecedence` with a higher minimum (which is where binding strength comes in). 

Every token has a row in the `rules[]` table: `{prefix, infix, precedence}`. The precedence enum is ordered weakest to strongest (`PREC_ASSIGNMENT` < `PREC_OR` < ... < `PREC_CALL` < `PREC_PRIMARY`), so higher values bind tighter.

A visual example:

Parsing `1 + 2 * 3`

[TODO]

Result: `OP_CONSTANT 1`, `OP_CONSTANT 2`, `OP_CONSTANT 3`, `OP_MULTIPLY`, `OP_ADD` — correctly encoding `1 + (2 * 3)`.

## Closures
The need for upvalues:
It is possible that a closure will try to reference a variable in a scope that it has outlived. Here is an example:

```lox
fun outer() {
    var x = "value";
    fun middle() {
        fun inner() {
            print x;
        }

        print "create inner closure";
        return inner;
    }

    print "return from outer";
    return middle;
}

var mid = outer();
var inn = mid();
inn();
```

Here, `inner()` tries to access the value `x` after `outer()` has returned and its stack frame (where `x` lived) has been popped.

Upvalues are heap-allocated wrappers that capture variables from enclosing scopes.
1. While the enclosing function is still running, the upvalue points directly at the variable's stack slot ("open" upvalue).
2. When the enclosing function returns, the upvalue copies the value off the stack into its own storage and becomes "closed". The variable now lives on the heap independently of any stack frame.
3. Resolution is recursive: if `inner` references a variable from `outer`, nested inside `middle`, the compiler threads upvalues through each intermediate function in the chain.