#ifndef MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H
#define MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H

// NOTE: (sonictk) Python has its own defines that require it to be included ahead
// of everything else.
#include <Python.h>


// NOTE: (sonictk) Setup Unity build here
#include "maya_python_c_ext_hello_world.cpp"
#include "maya_python_c_ext_py_hello_world.cpp"


static char MAYA_PYTHON_C_EXT_DOCSTRING[] = "An example Python C extension that makes use of Maya functionality.";


// NOTE: (sonictk) This declares the available methods for the module
static PyMethodDef mayaPythonCExtMethods[] = {
	{"hello_world_maya", pyHelloWorldMaya, METH_VARARGS, HELLO_WORLD_MAYA_DOCSTRING},
	{NULL, NULL, 0, NULL}	// NOTE: (sonictk) Sentinel value
};


#endif /* MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H */
