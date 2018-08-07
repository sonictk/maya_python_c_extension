#ifndef MAYA_PYTHON_C_EXT_PY_HELLO_WORLD_H
#define MAYA_PYTHON_C_EXT_PY_HELLO_WORLD_H

#include <Python.h>


static const char HELLO_WORLD_MAYA_DOCSTRING[] = "Says hello world!";


static PyObject *pyHelloWorldMaya(PyObject *module, PyObject *args);


#endif /* MAYA_PYTHON_C_EXT_PY_HELLO_WORLD_H */
