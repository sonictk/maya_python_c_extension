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


### How is this going to work? ###

We're going to eschew the official
Python
[``distutils``](https://docs.python.org/2/extending/building.html#building) for
this, and compile everything on our own. Before you start shouting "That's
un-Pythonic!" from the top of your horse, bear with it: there are very good
reasons for this which I'll explain later. 

We'll be making two types of Maya Python C++ Extensions: the first will actually
be TODO


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

In the next section, we'll go over a high-level overview of how a Python C/C++
extension works.
