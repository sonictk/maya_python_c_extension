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
yourself if you so wish), nor are we going to implement the other forms that
this call can take based on passing a different argument to
``listAdjustment``. (Also, simple convenience functions that are atomic in
nature anyway are far easier to debug, not to mention faster due to the lack of
branching going on.)

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
memory usage and making everyone happy. For now, we'll assume that this works...

> **Foreshadowing**
>
> Foreshadowing is a literary device in which a writer gives an advance hint of
> what is to come later in the story. Foreshadowing often appears at the
> beginning of a story, or a chapter, and it helps the reader develop
> expectations about the upcoming events.

(If you want to skip ahead to what I'm referring to, please
read [A possible problem](exposing_the_bindings_in_maya_plugin#a-possible-problem).)


## Implementing ``selectByName`` ##

Assuming that everything is going according to plan, let's go ahead and create
that missing OM2 binding we were talking about:

```c++
enum MayaPythonCExtStatus
{
    UNKNOWN_FAILURE = -1,
    NODE_DOES_NOT_EXIST = -2,
    UNABLE_TO_GET_ACTIVE_SELECTION = -3,
    UNABLE_TO_SET_ACTIVE_SELECTION = -4,
    UNABLE_TO_MERGE_SELECTION_LISTS = -5,
    SUCCESS = 0
};


MayaPythonCExtStatus addToActiveSelectionList(const char *name)
{
    MStatus stat;

    MSelectionList objList;
    stat = objList.add(name);
    if (!stat) {
        return MayaPythonCExtStatus::NODE_DOES_NOT_EXIST;
    }

    MSelectionList activeSelList;
    stat = MGlobal::getActiveSelectionList(activeSelList, true);
    if (!stat) {
        return MayaPythonCExtStatus::UNABLE_TO_GET_ACTIVE_SELECTION;
    }

    stat = activeSelList.merge(objList);
    if (!stat) {
        return MayaPythonCExtStatus::UNABLE_TO_MERGE_SELECTION_LISTS;
    }

    stat = MGlobal::setActiveSelectionList(activeSelList);
    if (!stat) {
        return MayaPythonCExtStatus::UNABLE_TO_SET_ACTIVE_SELECTION;
    }

    return MayaPythonCExtStatus::SUCCESS;
}
```

Again, this should be pretty simple to figure out what's going on. We check if
the object exists, and if it does, we add it to the actively-selected
items. But, for all intents and purposes, this is why you would use
``selectByName`` anyway, so it will fulfill our requirements for this particular 
example.

Let's go ahead and implement the Python binding for this:

```c++
static PyObject *pyAddToActiveSelectionList(PyObject *self, PyObject *args)
{
    const char *nodeName;
    if (!PyArg_ParseTuple(args, "s", &nodeName)) {
        return NULL;
    }

    PyGILState_STATE pyGILState = PyGILState_Ensure();

    MayaPythonCExtStatus stat = addToActiveSelectionList(nodeName);
    if (stat != MayaPythonCExtStatus::SUCCESS) {
        MGlobal::displayError("An error occurred!");
    }

    PyObject *result = Py_BuildValue("h", stat);

    PyGILState_Release(pyGILState);

    return result;
}
```

As before, we see that the signature of the function follows that of a
``PyCFunction`` type, where two pointers to ``PyObject``s are passed into the 
function. Again, we parse the first argument using ``PyArg_ParseTuple`` in order 
to extract the string that the caller passed into the Python function using the
``s`` format specifier. , we then ensure that the GIL is accquired once more
before executing our code, call our C++ function, and then return the status
code (since it's an enum, we return a ``short`` using the ``h`` format
specifier), while remembering to release the GIL when we're done with it.


## A possible problem ##

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


### Understanding why the segfault happens ###

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

5. Thus, when we call ``hello_world_maya`` the second time, we trigger
   a [segmentation fault](https://en.wikipedia.org/wiki/Segmentation_fault)
   since the function call basically is executing a jump instruction to a region
   of memory that was already deallocated by the OS and is no longer valid for
   use by the application.

What can we do to avoid this?

Unfortunately, not much that's non-intrusive; here's one solution:

```python
import maya.cmds as cmds

ARE_BINDINGS_VALID = False


def load_maya_plugin_and_validate_bindings(pluginPath):
    cmds.loadPlugin(pluginPath)
    import maya_python_c_ext

    ARE_BINDINGS_VALID = True

    return maya_python_c_ext


def unload_maya_plugin_and_invalidate_bindings(plugin):
    cmds.unloadPlugin(plugin)

    ARE_BINDINGS_VALID = False


def validate_bindings():
    return ARE_BINDINGS_VALID
```

As you can see, we write very simple wrappers around loading/unloading our
plugin. The wrapper functions just set a global variable called
``ARE_BINDINGS_VALID`` that we use to check if, well, the bindings are valid.

How would these work in practice? Like so:

```python
import sys
import os
import maya.standalone


if __name__ == '__main__':
    maya.standalone.initialize()
    import maya.cmds as cmds

    sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'py'))

    import validate

    pluginPath = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'msbuild', 'maya_python_c_ext.mll')
    print('loading plugin from {}'.format(pluginPath))

    mpce = validate.load_maya_plugin_and_validate_bindings(pluginPath)

    mpce.hello_world_maya('my string')

    validate.unload_maya_plugin_and_invalidate_bindings('maya_python_c_ext')

    if validate.validate_bindings():
        mpce.hello_world_maya('my string')
    else:
        print('The bindings are no longer valid!')

    maya.standalone.uninitialize()
```

It's not exactly what I would consider an ideal solution, but there rarely is
such a thing. Alternatively, you could consider having an intermediate plugin
handle validating the bindings (although you would then need to guarantee that
this intermediate plugin itself was loaded at all times as well), but that's a
level of complicatiion I don't want to get into right now.


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
