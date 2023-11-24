All zeros of a type (e.g. `0`, `false`) and empty dictionaries, lists, and strings are falsy.  Everything else is truthy.

A "block" is delimited by curly braces.  Each block has its own context.  Note that "case" statements are considered blocks and,
thus, have their own contexts.  Flow statements with initializers create them inside of their contexts.  Although those
initializers appear before the opening of the block, consider them part of the scope of the block.

When invoking a block, create a context and set its outer context to the current context.  When a block returns a function, return
it with its enclosing runtime context.

I will need a different mechanism for asynchronous and generator functions.

Define top-level code as a parameterless function.
