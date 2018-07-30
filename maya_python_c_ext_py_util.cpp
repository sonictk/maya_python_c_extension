#include "maya_python_c_ext_py_util.h"
#include "maya_python_c_ext_util.h"


static PyObject *pyAddToActiveSelectionList(PyObject *self, PyObject *args)
{
	const char *nodeName;
	if (!PyArg_ParseTuple(args, "s", &nodeName)) {
		return NULL;
	}

	PyGILState_STATE pyGILState = PyGILState_Ensure();

	MGlobal::displayInfo(nodeName);

	MayaPythonCExtStatus stat = addToActiveSelectionList(nodeName);
	if (stat != MayaPythonCExtStatus::SUCCESS) {
		MGlobal::displayError("An error occurred!");
	}

	// NOTE: (sonictk) Converting the enum (short) to a Python int
	PyObject *result = Py_BuildValue("h", stat);

	PyGILState_Release(pyGILState);

	return result;
}
