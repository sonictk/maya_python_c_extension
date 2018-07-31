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



## Running it in Maya ##

## When to use this method ##
