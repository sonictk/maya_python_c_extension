#include "maya_python_c_ext_py_hello_world.h"
#include "maya_python_c_ext_hello_world.h"

#include <stdio.h>
#include <maya/MGlobal.h>


static PyObject *pyHelloWorldMaya(PyObject *self, PyObject *args)
{
	const char *inputString;
	if (!PyArg_ParseTuple(args, "s", &inputString)) {
		return NULL;
	}

	helloWorldMaya();
	MGlobal::displayInfo(inputString);

	PyObject *result = Py_BuildValue("hello world");

	return result;
}
