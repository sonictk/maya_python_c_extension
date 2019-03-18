#include "maya_python_c_ext_plugin_main.h"


// NOTE: (sonictk) Disable name mangling for Python distutils since it really
// wants C-style compilation of exporting symbols. Also, since it hardcodes the
// name of the symbol that it exports, we have to use the snake-case naming
// for the entry point.
extern "C" PyMODINIT_FUNC initmaya_python_c_ext()
{
	if (PyType_Ready(&TestObjType) < 0) {
		return;
	}

	PyObject *module = Py_InitModule3("maya_python_c_ext",
									  mayaPythonCExtMethods,
									  MAYA_PYTHON_C_EXT_DOCSTRING);
	if (module == NULL) {
		return;
	}

	// TODO: (sonictk) Test code.
	// TestObjType.tp_new = PyType_GenericNew;

	Py_INCREF(&TestObjType);
	PyModule_AddObject(module, "TestObj", (PyObject *)&TestObjType);
}
