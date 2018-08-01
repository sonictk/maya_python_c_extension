# Exposing Python bindings directly in a Maya plugin #

In the last major chapter, we successfully (hopefully) wrote a basic Maya Python
C++ extension that utilized Maya functions to print the equivalent of ``hello world``
to ``stdout``. We also saw how to parse basic arguments and pass that to our C function.

Now, we're going to try and implement a simple binding that's been missing from
OM2 for a while: ``MGlobal::selectByName``.

From the documentation:

```
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

```
MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    if (Py_IsInitialized()) {
        PyGILState_STATE pyGILState = PyGILState_Ensure();

        PyObject *module = Py_InitModule3("maya_python_c_ext",
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

    return status;
}
```

Looks fairly similar to the code earlier, doesn't it? There are some slight
differences, so let's go over them:

```
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }
```

Here, we check if the Python interpreter has been initialized yet, and if not,
we initialize it. This really shouldn't happen in practice, but it doesn't hurt
    to be safe.

```
        Py_INCREF(module);
```

Remember how I talked about reference counting previously? We increment the
count here after we verify that the module has indeed been initialized
appropriately.

If you've been paying attention, however, you might notice something
odd. Where's the symmetric code for decrementing the reference count, or
uninitializing Python if we already initialized it?


## Managing module reference counting ##

Let's talk about the reference counting issue first. Conventional wisdom might 
make it seem that we should probably be decrementing the reference count for the 
module in the ``uninitializePlugin`` call for the module. You're free to try this 
as well (and you might not even notice any unwanted side-effects at first!). 

However, try this:

```
import maya.cmds as cmds

cmds.loadPlugin("maya_python_c_ext")
from maya_python_c_ext import *
hello_world_maya("this works")

cmds.unloadPlugin("maya_python_c_ext")
cmds.loadPlugin("maya_python_c_ext")

hello_world_maya("hmm, I wonder what happens now...?")
```

If you want to go ahead and try it yourself, go ahead. I'll wait.

...

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
this?

Well, recall the rules for the reference garbage collector in Python; once the
reference count reaches zero, the garbage collector is free to de-allocate the
memory for the object that the counter is associated with. Thus, when we
decrement the reference count to the module once we unload the plugin, shouldn't
the module be reloaded again once we make the second call to ``Py_InitModule``?

Not exactly, due to
an [unfortunate quality of Python](https://bugs.python.org/issue9072): the lack
of ability to **unload modules**.


## Running it in Maya ##

## When to use this method ##
