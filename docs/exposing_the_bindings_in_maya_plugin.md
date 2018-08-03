# Exposing Python bindings directly in a Maya plugin #

In the last major chapter, we successfully (hopefully) wrote a basic Maya Python
C++ extension that utilized Maya functions to print the equivalent of ``hello world``
to ``stdout``. We also saw how to parse basic arguments and pass that to our C function.

Now, we're going to try and implement a simple binding that's been missing from
OM2 for a while: ``MGlobal::selectByName``.

From the documentation:

```c++
MStatus selectByName(const MString &name, MGlobal::ListAdjustment listAdjustment = kAddToList)
```

> Puts objects that match the give name on the active selection list.

In the interest of keeping things manageable, we're not going to implement the
regex functionality that the original binding has (You can certainly do that
yourself if you so wish).

Additionally, this time, we're going to implement things slightly differently;
instead of needing to compile a ``.pyd`` file, we're going to expose the
bindings to the end-user directly when the ``.mll`` plugin is loaded. No messing
with the ``PYTHONPATH``!


## The new entry point ##

To do this, we'll need to change the plugin so that it actually _is_ more like a
conventional Maya plugin. Again, I go over how this machinery works in my
[previous tutorial](https://sonictk.github.io/maya_hot_reload_example_public/getting_started/#what-are-we-even-doing)
in great detail.

```c++
#include "maya_python_c_ext_plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Siew Yi Liang";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";

PyObject *module = NULL;


MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    if (Py_IsInitialized()) {
        PyGILState_STATE pyGILState = PyGILState_Ensure();

        module = Py_InitModule3("maya_python_c_ext",
                                          mayaPythonCExtMethods,
                                          MAYA_PYTHON_C_EXT_DOCSTRING);

        MGlobal::displayInfo("Registered Python bindings!");

        if (module == NULL) {
            return MStatus::kFailure;
        }
        Py_INCREF(module);

        PyGILState_Release(pyGILState);
    }

    return MStatus::kSuccess;
}


MStatus uninitializePlugin(MObject obj)
{
    MStatus status;

    Py_DECREF(module);

    return status;
}

```

Looks fairly similar to the code earlier, doesn't it? There are some slight
differences, so let's go over them:

```c++
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }
```

Here, we check if the Python interpreter has been initialized yet, and if not,
we initialize it. This really shouldn't happen in practice, but it doesn't hurt
to be safe.

The next state of affairs to take care of is the aforementioned GIL:

```c++
        Py_INCREF(module);
```

Remember how I talked about reference counting previously? We increment the
count here after we verify that the module has indeed been initialized
appropriately.

We also implement the symmetric version of this in ``uninitializePlugin``:

```c++
    Py_DECREF(module);
```

At first glance, this seems fine; we're incrementing the reference count to the
module in the plugin's entry point, and decrementing it in the exit point. By
conventional wisdom, this should mean that our module will get cleaned up by the
reference collector in Python when the Maya plugin is unloaded, thus saving on
memory usage and making everyone happy.

Now, try this:

```
import maya.cmds as cmds

cmds.loadPlugin("maya_python_c_ext")
from maya_python_c_ext import *
hello_world_maya("this works")

cmds.unloadPlugin("maya_python_c_ext")
cmds.loadPlugin("maya_python_c_ext")

hello_world_maya("hmm, I wonder what happens now...?")
```

I'll wait.

...

Back yet? What did you see? Was it something similar to:

```
Stack trace:
  python27.dll!PyEval_GetFuncDesc
  python27.dll!PyEval_EvalFrameEx
  python27.dll!PyEval_EvalCodeEx
  python27.dll!PyRun_FileExFlags
  python27.dll!PyRun_InteractiveOneFlags
  python27.dll!PyRun_InteractiveLoopFlags
  python27.dll!PyRun_AnyFileExFlags
  python27.dll!Py_Main
  KERNEL32.DLL!BaseThreadInitThunk
  ntdll.dll!RtlUserThreadStart

Result: untitled
Fatal Error. Attempting to save in C:/Users/sonictk/AppData/Local/Temp/sonictk.20180801.0110.ma
```

...in case you skipped right here without trying it yourself (tsk, tsk), you
probably noticed what happened; you segfaulted the Python interpreter. Why is
this? Shouldn't the Python garbage collector have done its job and cleaned up
after us? Why are we still triggering a segmentation fault?

It turns out that, as usual, everything has to do with memory sooner or later,
and this is one area that a memory-managed language such as Python is severely
deficient in.

## Caveats ##

If you look closely at the script, you can see that we essentially perform the
following operations:

1. Load the DLL for our Maya plugin, which also initializes the Python bindings
   to the embedded Python interpreter inside of Maya.
   
2. We import the module that contains our bindings, and all its members into the
   global namespace for the Python interpreter.
   
3. If we recall, Python caches the modules that get loaded into memory. This is
   done so that subsequent ``import`` statements for the same module can just
   re-use the already-loaded module object instead of re-importing it
   again. Normally, this would be fine in a standard Python interpreter, where
   ``.pyd`` files aren't expected to be unloaded at run-time once they've
   already been loaded by the Python interpreter.

4. However, when we call ``cmds.unloadPlugin`` to unload the ``.mll`` file at
   run-time later, we have basically invalidated all memory to that module that
   was loaded previously. _Even though the Python interpreter is still holding a
   reference to it!_

5. Thus, when we call ``hello_world_maya`` the second time, we trigger a
   segmentation fault.
   
What can we do to avoid this? 

Unfortunately, not much that's easy; the scope of solving this problem is
outside the scope of this tutorial and I won't be delving into that right now.
I'm still undecided on the best method myself; I might make a follow-up to this
if I find a good solution in the future that I think is acceptable. For now,
though, the overriding principle here should be:

- Keep your plugin loaded around for as long as you need to call bindings into
  it, or use objects allocated from it. Once unloaded, all bets are off.
  
- If you are in a scenario where you _need_ to unload the plugin, _always_
  ensure that you load the plugin again _before_ making any calls that require
  memory from it.

!!! tip "Python and unloading modules"
    To read more about why unloading Python modules has been such a contentious
    issue, you can [look at the discussion thread](https://bugs.python.org/issue9072).
    

## When to use this method ##

So, with the aforementioned snafu, when would you want to make use of this
alternative technique for exposing the bindings? It depends on the situation,
but for me personally, I think this is most useful when:

- You do not need to guarantee availability of the bindings.

- The functionality you're trying to expose is inherently tied to the rest of
  the plugin. (i.e. debugging utilities for custom nodes)

- You're willing to create extra indirection in order to defend against the
  aforementioned problem, either by creating an intermediate plugin to act as a
  "guard" layer that checks for validity of the module loaded before allowing
  you to access the module, or just having Python wrapper functions that have
  globals set to indicate the validity of the modules that have been loaded.
  
Perhaps the use cases might be limited, but I think it's a good tool to keep in
mind nonetheless.
