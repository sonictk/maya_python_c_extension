# Writing a Maya Python C++ Extension #

## About ##

This is a tutorial on how to expose bindings from your C++ code to Python by
means of writing a Python C++ extension. Unlike standard Python C/C++
extensions, however, I will elaborate on specific steps to take when writing
bindings that take advantage of the Maya libraries, along with how the extension
will work within the Maya Python interpreter.

All the source code for this example node is available
[here](https://bitbucket.org/sonictk/maya_python_c_extension).


## Why should I care? ##

To be honest, I actually have started to become less reliant on Python as I
continue to write more and more code, especially in Maya, now that I can [hotload
my C++ code](https://bitbucket.org/sonictk/maya_hot_reload_example). However,
there are some very good reasons why we might want to write Python bindings for
our C++ code:

* Legacy code: the reality is that there is a lot of code out there that is
  already written in Python. Depending on the project, it might not be worth the
  cost to re-write the entire thing in C++ just for the sake of using a couple
  of utility functions. However, re-writing C++ functions from scratch back to
  Python is also a pain, not to mention that we'd take a _much_ larger
  performance hit than going the route of a Python C++ extension due to the
  larger amount of memory that would need to be marshaled between the two
  layers.
  
* Ease of debugging: while hot-reloadable code is nice, the fact remains that
  sometimes, it is easier to be able to call specific utility functions from
  both Python and C++ for the sake of inspecting scenes. Writing template
  ``MPxCommand`` boilerplate gets incredibly annoying after a while, along with
  making it more difficult to actually integrate with a Python codebase.
  (i.e. returning Python ``dict`` or other data types becomes impossible)
  
* Extending existing bindings: the reason I even bothered to write this tutorial
  in the first place is because there's been a lot of grumbling from both at
  work and outside of work from junior/senior people alike about the state of
  the Python OpenMaya bindings. My primary goal here is to show that it's
  actually fairly trivial to write your own bindings if needed.
  
  
## Why not use a wrapper for this? ##

When it comes to generating Python bindings for a C/C++ API, there are a
plethora of tools out there. 
[Boost.Python](https://www.boost.org/doc/libs/1_67_0/libs/python/doc/html/index.html),
[SWIG](http://www.swig.org/) and more recently, [pybind11](https://github.com/pybind/pybind11)
are all examples of wrappers that can help to automate the process of having to
write the bindings by hand. As a matter of fact, this is how the OM1 bindings
were generated in the first place (using SWIG). Ideally, this allows the
programmer to focus on the implementation without worrying about the details
involved with translating the code to Python.

At least, that's the theory. As ever, in practice, everything comes down to a
single principle: _your job is to work with memory, not write code._ If you
don't know what you're doing with the memory, you're guaranteed to be setting
yourself (or worse, others) up for failure. In this case, it means:

* Harder-to-debug abstraction layers whenever the bindings don't work as you
  expect. Writing Python bindings is already a level of abstraction that we're
  accepting; the last thing needed is to add _even more_ cruft on top of that.
  
* pybind11, at least, does not account for the GIL by default unless you make
  specific wrapper calls (e.g. ``call_go`` using the
  ``call_guard``policy). Personally, after giving it a go, I see zero benefit to
  using it over writing the bindings manually.
  
* Absolute control over the memory and how it's managed. Rather than code all
  sorts of machinery in order to perform simple tasks like cleaning up
  allocations at module destruction time, or wasting time with ``unique_ptr``
  and ``shared_ptr`` shennanigans, I find everything is much simpler when just
  writing the bindings by hand.
  
* To that point, not every project can afford to use a C++11-compliant
  compiler. (Remember, as recently as Maya 2016, the official compiler version
  was not even C++11 capable)!
  
* The historical results speak for themselves: The Maya OM1 bindings, which were
  generated via SWIG, are far slower than the hand-written bindings in OM2
  (which are also incidentally far easier to use and don't require the use of
  other wrapper classes such as ``MScriptUtil`` to handle converting between
  pointer and POD types). If you're going to eat of the cost of making bindings
  between a scripting language and a compiled one, you might as well try to
  minimize the overhead in the process. At scale, automatically-generated
  bindings just don't work.
  
!!! tip
    I encourage you to try the alternatives stated and decide for yourself which
    approach is easier at scale. Here, however, I will focus only on hand-writing
    the bindings and not on any unnecessary wrappers.


## How is this going to work? ##

We're going to eschew the official Python
[``distutils``](https://docs.python.org/2/extending/building.html#building) for
this, and compile everything on our own. Before you start shouting "That's
un-Pythonic!" from the top of your horse, bear with it: there are very good
reasons for this which I'll explain later. 

We'll be making two types of Maya Python C++ Extensions: the first will be a
traditional Python C extension that makes use of the Maya libraries; the second,
however, will be an actual Maya plugin that exposes the Python bindings
automatically once it's loaded in Maya. The nice thing about the second approach
is that if you want to expose your bindings from a Maya plugin, you can do so
just by loading the plugin itself without having add the ``.pyd/.so`` file onto
the ``PYTHONPATH``. It also sidesteps any dependency issues nicely (i.e. you
know for sure that your Python function call will succeed without having to
check if the required Maya plugin has yet been loaded.)


## Requirements ##

This tutorial will come from a *Windows-first approach*. Whenever platform-specific
information is appropriate, it will appear in the following form:

!!! tip "Crossing the platforms"
    Platform-specific information goes here.


### What you will need ###

- **Maya 2018 or higher**. 

- **Python 2.7.xx (Python 3.x series is _not_ a substitute!)**.

- A **C/C++ development environment** set up and ready to go. (If you want to see
  what my Emacs setup looks like, it's
  available [here](https://github.com/sonictk/lightweight-emacs).)
  
- On Windows, you will need the corresponding **Visual Studio** version that
  your version of Maya was compiled with. For Maya 2018, this is **Visual Studio
  2015**. 
  
!!! tip "Crossing the platforms"
    On Linux, you will need GCC 4.8.2 against the Linux kernel in CentOS 6.5.
    On OSX, you will need Xcode 7.3.1 against the 10.11 SDK.
  
- Spare time and an open mind.


### What you should know ###

- **Basic knowledge of C/C++**. I will focus on including only the code that is
  important; I expect you to be able to understand how to fill in the rest as
  needed. At the very least, you should be able to compile a Maya plug-in and
  run it using whatever toolchain you're familar with.

- **Basic knowledge of how Maya plugins work and how to write/build them**. 
  The repository has sample build scripts designed to work on Windows and Linux.
  
- **Basic knowledge of Python**. Obviously, you'll need to know basic Python
  syntax and the various data types available, since we're going to be
  marshaling them between it and C++.

In the next section, we'll go over a high-level overview of how a Python C/C++
extension works.
