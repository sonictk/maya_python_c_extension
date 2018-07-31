# Getting started #

Now that we understand the goals of the project, let's dive a little bit into
the theory surrounding how a Python C extension works before writing any
boilerplate. It'll help serve as a basis for guiding what we need to do first.

## What is a Python C Extension anyway? ##

If you have some competence with Python, you'll probably have noticed by now
that sometimes, certain Python modules aren't loaded in the traditional sense of
having a ``.py``file along with a ``__init__.py`` file as well; there's usually
a single ``.pyd`` instead. If you've seen
my
[previous tutorial](https://sonictk.github.io/maya_hot_reload_example_public/getting_started/#understanding-how-libraries-work),
know this: a ``.pyd`` file is essentially a DLL. It's the _exact same
format_. However, just like a Maya plugin (``.mll`` on Windows) shares the exact
same format as a DLL does, a ``.pyd`` file follows specific conventions so that
the Python interpreter knows how to load it:

* The entry point for module named ``foo_bar.pyd`` will be automatically determined
  to be ``initfoo_bar``. This is the convention that the Python interpreter uses
  to search for a symbol so that it can dynamically load the library at run-time
  when the statement ``import foo_bar`` is parsed in the interpreter. We'll talk
  about this more when we implement our own entry point in a bit.

* As mentioned above, the ``.pyd`` file does not need to be linked with the main
  program executable (or Maya plugin, in this case), since the Python
  interpreter will be loading it dynamically at run-time.

* You cannot rely on the standard method of declaring exports from a DLL, either
  through symbol visiblity or specifying external linkage (i.e. ``extern
  "C"``). Python-specific exports must be declared through the use of special
  functions, ``Py_InitModule`` and its variants. Again, we'll be talking about
  this in a bit.

!!! tip "Crossing the platforms"
    On Linux, ``.pyd`` files will instead be ``.so`` files, and (I believe, but
    I can't be bothered to boot up my Macbook to check) on OSX, this will be
    ``.dylib`` files.

The main takeaway here is that _python modules are nothing special._ They're
just libraries, loaded at run-time dynamically, that follow specific conventions
that the Python interpreter relies on to be able to function the way it does.

Ok, so we know that a ``.pyd`` is essentially just a DLL. Cool. So how does the
Python interpreter know where to look for our module when we type ``import foo_bar``?

## Loading a Python module ##

!!! tip "Fast-forward"
    If you're already innately familiar with how Python searches for modules,
    you can go ahead and skip to the next section. If not, I suggest you read
    this part before continuing.

The search path for all modules globally available to Python happens in a couple
of areas; one of those is on the path specified by the environment variable
``PYTHONPATH``. If ``foo_bar.pyd`` is available on that path, typing ``import
foo_bar`` will cause the Python interpreter to attempt to load that ``.pyd``
file, inspect it for a suitable entry point and initialize it if found. Once it
does so, you'll be able to access the functions previously defined in its
_function table_.

Bear in mind; if you have another conventional ``foo_bar.py``module also
available on the ``PYTHONPATH``, ahead of the ``.pyd`` version, that module will
be used instead of the compiled version! It is therefore important to manage the
paths on your ``PYTHONPATH``, along with the namespaces of your modules.


## "Hello, Maya" ##

Before we write bindings to anything, we need _something_ to actually bind
to. Let's start with the timeless ``hello world``.

```
#include <maya/MGlobal.h>


void helloWorldMaya()
{
	MGlobal::displayInfo("Hello world from the Maya Python C extension!");

	return;
}
```

You should be able to tell what this does at a glance. Simple and
straightforward. However, instead of writing a bunch of boilerplate and an
``MPxCommand`` to fire off this function, we're going to skip all of that expose
it directly to Python instead. Which means we get to write a whole different set
of boilerplate instead!

```
#include "maya_python_c_ext_py_hello_world.h"
#include "maya_python_c_ext_hello_world.h"

#include <Python.h>

#include <stdio.h>


static const char HELLO_WORLD_MAYA_DOCSTRING[] = "Says hello world!";

static PyObject *pyHelloWorldMaya(PyObject *self, PyObject *args);

static PyObject *pyHelloWorldMaya(PyObject *self, PyObject *args)
{
	const char *inputString;
	if (!PyArg_ParseTuple(args, "s", &inputString)) {
		return NULL;
	}

	PyGILState_STATE pyGILState = PyGILState_Ensure();

	helloWorldMaya();

	PyObject *result = Py_BuildValue("s", inputString);

	PyGILState_Release(pyGILState);

	return result;
}
```

Ok, I typed a bunch of stuff up there that seems awfully scary. Let's break this
down line-by-line:

```
#include <Python.h>

static const char HELLO_WORLD_MAYA_DOCSTRING[] = "Says hello world!";

```

This is fairly straightforward; I'm writing a Python C extension and I'm going
to be using functionality from the Python libraries. I need the header.

I also define a docstring that I would like my function to have. I know, _fancy_.

```
static PyObject *pyHelloWorldMaya(PyObject *self, PyObject *args);
```

What is a ``PyObject``? If you look in the Python source code, you might lose
your sanity, so I'll summarize here: it's basically a typedef'ed type that
Python uses to store information about a pointer to an object, so that it can
treat the ``PyObject`` _itself as an object_. Yes, this is Inception-mode.

The reason for this is that in a release build, the ``PyObject`` only contains
the reference count for the object (which is used for determining when the
Python garbage collector is free to release the memory allocated to the object),
along with a pointer to the corresponding _type object_.


### Garbage collection ###

Python has an in-built garbage collector, which is another way of saying
that it attempts to manage the memory for you for the objects that you
use. There's actually two separate collectors, one being the **reference
counting** collector and the other **generational** collector, available in
the ``gc`` module, but we'll focus on the reference-counting one for
now.

How it works is that every object owned by Python (which is really just
a reference to an _actual_ object with the _actual_ data) has a simple counter
that tracks how many times a pointer to the object is copied or deleted and
is incremented/decremented as necessary. Once the counter reaches ``0``, the
object is free to be deallocated by the collector. It's a fairly simple
mechanism; however, we need to make sure that we are aware of how it works
since if we forget to manage the memory correctly, we could end up having a
memory leak in our bindings or worse, a crash.

More information on this is available [here](https://docs.python.org/3.6/c-api/intro.html#objects-types-and-reference-counts).

So what's the rest of the code doing, then?


### Parsing the input arguments ###

```
static PyObject *pyHelloWorldMaya(PyObject *self, PyObject *args)
{
	const char *inputString;
	if (!PyArg_ParseTuple(args, "s", &inputString)) {
		return NULL;
	}

	PyGILState_STATE pyGILState = PyGILState_Ensure();

	helloWorldMaya();

	PyObject *result = Py_BuildValue("s", inputString);

	PyGILState_Release(pyGILState);

	return result;
}
```

The signature of the function is something that Python expects from a
``PyCFunction`` type. This is just something you'll have to accept as convention
for now; we'll see why this is enforced later when we register this function in
the _function table_. Basically our function must return a pointer to a
``PyObject``, and take two pointers to ``PyObject``s as well.

The first few lines make use of ``PyArg_ParseTuple`` to basically parse the
arguments given to the Python function. For example, if I called the function
``foo_bar('oh noes')``, ``PyArg_ParseTuple`` would, with the given arguments
specified, interpret the first argument as a string, and store that in the
``inputString`` pointer. If you look at
the [documentation](https://docs.python.org/2/c-api/arg.html)
for it, you'll realize that the format specifiers are similar to that of
``printf`` in the standard library, but **be warned:** there are subtle
differences.

The next call we make is something you might not find in other "HOW TO WRITE
YOUR OWN PYTHON C EXTENSION" tutorials, and that's because it's Maya-specific.


### The Global Interpreter Lock (GIL) ###

This is a topic that comes up a lot, even for experienced TDs/programmers at
work, so I thought I'd take my stab at explaining what is really at its core a
fairly fundamental concept, if a bit complex.

**The Global Interpreter Lock in Python is a mutex**. That's it.

!!! tip "Mutex?"
    Ok, so for those of us who aren't familiar with what a mutex is, it's short for
    **mutual exclusion object**. It is basically a mechanism (more often than not
    implemented as a simple object with a unique ID) that allows a single thread
    within a running process to say, "Hey, I need to access this memory". The
    program then requests (either by itself or from the OS, even) a handle that can
    act as this program object. Once it is accquired by the thread, no other threads
    are allowed to access that memory, or **critical section**, until the mutex
    program object is **released** by the first thread owner.

Ok, so the _actual_ implementation of the GIL goes a little beyond a simple
mutex, but it essentially boils down to: the sole purpose of the GIL is to
ensure that Python objects (and the memory they point to) are protected from
multi-threaded access, making sure that Python bytecode can only be executed
from a single thread at a single time. This is required in the CPython
implementation (which is what Maya's Python, and likely your system's Python
installation as well is using) since the memory management implementation in
CPython **is not thread-safe**.

In order to ensure that we follow the conventions required by CPython
extensions, we must **accquire the GIL** before executing Python code
ourselves. This is _even more important_ in Maya, because in Maya, **the main
thread is not a Python thread**. (In case that wasn't already obvious, otherwise
things would be much, much slower.)

What we'll be doing is registering the GIL towards the main thread so that the
Python interpreter can "see" it and execute our code. While normally in a Python
C extension you probably wouldn't care about this, or would use
``PyEval_SaveThread`` and ``PyEval_RestoreThread`` instead, we make use of
``PyGILState_Ensure`` and ``PyGILState_Release`` to accquire and release the
lock. Thus, we see these two calls surrounding the beginning and end of the
relevant critical section that calls Python code.

### Returning ``PyObject`` values ###

### Writing the build script ###
