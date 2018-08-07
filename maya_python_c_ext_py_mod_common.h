#ifndef MAYA_PYTHON_C_EXT_PY_MOD_COMMON_H
#define MAYA_PYTHON_C_EXT_PY_MOD_COMMON_H

#include "maya_python_c_ext_hello_world.h"
#include "maya_python_c_ext_py_hello_world.h"
#include "maya_python_c_ext_util.h"
#include "maya_python_c_ext_py_util.h"


static const char MAYA_PYTHON_C_EXT_DOCSTRING[] = "An example Python C extension that makes use of Maya functionality.";


// NOTE: (sonictk) This declares the available methods for the module
static PyMethodDef mayaPythonCExtMethods[] = {
	{"hello_world_maya", pyHelloWorldMaya, METH_VARARGS, HELLO_WORLD_MAYA_DOCSTRING},
	{"add_to_active_selection_list", pyAddToActiveSelectionList, METH_VARARGS, ADD_TO_ACTIVE_SELECTION_LIST_DOCSTRING},
	{NULL, NULL, 0, NULL}	// NOTE: (sonictk) Sentinel value for Python
};


#endif /* MAYA_PYTHON_C_EXT_PY_MOD_COMMON_H */
