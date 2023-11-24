All zeros of a type (e.g. `0`, `false`) are falsy.  Empty dictionaries and lists are falsy.  Everything else is truthy.

A "block" is delimited by curly braces.  Each block has its own context.

When invoking a block, push the current context onto the context stack, create a context for that block, set its parent to the
context on the top of the stack, and set it as the current context.  When a block returns a function, set the parent context of the
returned block to the current context.  For immediately invoked blocks, such as those that are part of flow statements, set the
parent context of such a block to the current context and invoke it.

I will need a different mechanism for asynchronous and generator functions.

Functions are implemented as blocks with parameters.  Or, blocks are implemented as parameterless functions.

Final:  define all blocks as parameterless functions.  Define a function as having a context and a sequence of parameters.  Define
top-level code as a parameterless function.
