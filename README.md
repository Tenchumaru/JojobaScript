All zeros of a type (e.g. `0`, `false`, `null`) are falsy.  Empty dictionaries and lists are falsy.  Everything else is truthy.
Outline of creating contexts.

A "block" is delimited by curly braces.  Each block has its own context.

During parse, create a context template for each block, giving it the current context template as its parent and setting it as the
current context template.  Context templates have a context which is initially set to the global context.  The context stack starts
with the global context as its only element.

Modification:  context templates need not have a parent.

Either:

	When invoking a block, push the current context onto the context stack, create a context from that block's context template,
	set its parent to its context template's context, set the context of each child of its context template to it, and set it as
	the current context.

Or:

	When invoking a block, create a context from its context template, set the parent of the context to its parent context, and
	push the context onto the context stack.  When a block returns a function, set the parent context of the returned block to the
	current context.  For immediately invoked blocks, such as those that are part of flow statements, set the parent context of
	such a block to the current context and invoke it.

I will choose the second option.  This means context templates need not have a context.  Each block has a parent context that is
its lexical parent.

When leaving a block, pop the current context off of the context stack, deleting it if no block references it.  I will need a
different mechanism for asynchronous and generator functions.

Functions are implemented as blocks with parameters.  Or, blocks are implemented as parameterless functions.

Final:  define all blocks as parameterless functions.  Define a function as having a context template and a sequence of parameters.
Define top-level code as a parameterless function.
