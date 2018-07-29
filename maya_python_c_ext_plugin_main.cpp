#include "maya_python_c_ext_plugin_main.h"


PyMODINIT_FUNC initMayaPythonCExt()
{
	PyObject *module = Py_InitModule3("mayaPythonCExt", mayaPythonCExtMethods, MAYA_PYTHON_C_EXT_DOCSTRING);
	if (module == NULL) {
		return;
	}
}
