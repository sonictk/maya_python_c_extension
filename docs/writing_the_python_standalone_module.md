# Writing the Maya Python standalone module #

## The entry point ##

The entry point, as mentioned previously, must have a specific function
singature based off the desired module name.

```c++
static char MAYA_PYTHON_C_EXT_DOCSTRING[] = "An example Python C extension that makes use of Maya functionality.";

static PyMethodDef mayaPythonCExtMethods[] = {
    {"hello_world_maya", pyHelloWorldMaya, METH_VARARGS, HELLO_WORLD_MAYA_DOCSTRING},
    {NULL, NULL, 0, NULL}
};

extern "C" PyMODINIT_FUNC initmaya_python_c_ext()
{
    PyObject *module = Py_InitModule3("maya_python_c_ext",
                                      mayaPythonCExtMethods,
                                      MAYA_PYTHON_C_EXT_DOCSTRING);
    if (module == NULL) {
        return;
    }
}
```

Alright, let's break this down bit-by-bit again. The first line is fairly
obvious; we're just defining a docstring for the module itself.

The second line, however, introduces a new ``PyMethodDef`` type: this is what
we're going to use for defining the _function table_ available in our module to
the Python interpreter.


## Listing the available methods ##

``PyMethodDef`` is essentially a structure with 4 fields: the name of the
function desired (i.e. what we will actually type in the Python interpreter when
calling it), the callback to be executed, an enum indicating the type of
arguments that it accepts, and the docstring for the function itself that will
be displayed when someone calls ``.__doc__`` on the function object in the
Python interpreter. We define ``mayaPythonCExtMethods`` as an array of these
structures, with a _sentinel value_ of a structure with ``NULL`` fields to
indicate the bounds of the array.

We then pass that to the call ``Py_InitModule3`` in our entry point
``initmaya_python_c_ext``, which will allow Python to register our module and
register the table, thus making the functions available for use in the
interpreter. We export the entry point with ``extern "C"`` so that the compiler
doesn't
[mangle](https://sonictk.github.io/maya_hot_reload_example_public/getting_somewhere/#name-mangling-visibility) the
name of the symbol when it is compiled.

Ok, now we have all the code written. (Wasn't that fast?) We just need to
actually build the ``.pyd``now so that we can use it.


## Building the extension ##

This is where I will stray a little from past tutorials, and even the official
Python documentation: we will not be making use of ``distutils`` or CMake or
anything like that. We will write a ``build.bat``. The ``build.bat`` will build
the code, no other nonsense involved. (And trust me, if you go down the
``distutils`` route, there is a _whole lot_ of involvement.)

The ``build.bat`` script is provided in
the
[code for this repository](https://bitbucket.org/sonictk/maya_python_c_extension). Most
of it is fairly straightforward if you understand batch file syntax on Windows,
so I'll only focus on the parts that concern this project.

!!! tip "Crossing the platforms"
    On Linux, I provide a ``build.sh`` script, which is functionally similar to
    the Windows ``build.bat``. If you're only familiar with Bash syntax, you can
    take a look at that instead.

After the usual boilerplate of setting up the Visual Studio environment,
creating a build directory and all that jazz, we get to something interesting in
the script:

```batch
set CommonCompilerFlags=%CommonCompilerFlags% /I"%MayaRootDir%\include" /I "%MayaRootDir%\include\python2.7"
```

Notice that we're not including the system Python headers directly. We also
don't link against the system Python either:

```batch
set CommonLinkerFlags=%CommonLinkerFlags% "%MayaLibraryDir%\OpenMaya.lib" "%MayaLibraryDir%\OpenMayaAnim.lib" "%MayaLibraryDir%\OpenMayaFX.lib" "%MayaLibraryDir%\OpenMayaRender.lib" "%MayaLibraryDir%\OpenMayaUI.lib" "%MayaLibraryDir%\Foundation.lib" "%MayaLibraryDir%\IMFbase.lib" "%MayaLibraryDir%\clew.lib" "%MayaLibraryDir%\Image.lib" "%MayaLibraryDir%\python27.lib"
```

Why is this?

Well, recall that for Maya specifically, there is a Python interpreter embedded
directly in the application. It's also been modified slightly from the standard
Python interpreter; in order to make sure that we don't encounter any funny
issues down the line (read: crashes), we're going to link against Maya's version
instead. This does mean, however, that you should not attempt to use the module
we're building in a standard non-``mayapy`` Python interpreter. (You probably
wouldn't anyway, since the whole point is that we're making use of Maya-specific
functionality in our module that wouldn't work in standard Python.)

There's one more thing we need to do:

```batch
set PythonModuleExtension=pyd

set PythonModuleLinkerFlagsCommon=/export:initmaya_python_c_ext /out:"%BuildDir%\%ProjectName%.%PythonModuleExtension%"
```
Remember how we declared our entry point with C-linkage above? This is where we
export that symbol specifically. We need to do this on Windows, since symbols
are stripped from the binary in release mode otherwise.

The rest of the build script is mostly boilerplate to actually compile and link
the module. If you have trouble understanding it, I suggest reading
my
[previous tutorial](https://sonictk.github.io/maya_hot_reload_example_public/getting_somewhere/#writing-the-build-script) which
goes over each section of the build script in greater detail.

Do note: if you're going to just use my ``build.bat`` wholesale, make sure you
edit the ``MayaRootDir`` variable to point to the root of your Maya
installation. You should also make sure that the ``include`` folder from your
Maya devkit is placed in there as well, or else edit the
``MayaIncludeDir``variable to point to it as needed.


## Running it in ``mayapy`` ##

Run the following code in a ``mayapy`` interpreter:

```python
import maya.standalone
maya.standalone.initialize()

import sys
sys.path.append('path to the msbuild folder')

from maya_python_c_ext import *
hello_world_maya('oh noes')
```

You should see:
```
Hello world from the Maya Python C extension!
oh noes
'oh noes'
```

Great! Now in the next chapter, we'll focus on implementing one of the things
people have been complaining about: a missing OM1 to OM2 binding.
