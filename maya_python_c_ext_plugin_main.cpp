#include "maya_python_c_ext_plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Siew Yi Liang";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";


MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
	if (!Py_IsInitialized()) {
		Py_Initialize();
	}

	if (Py_IsInitialized()) {
		// NOTE: (sonictk) Since the Maya main thread is not a Python thread, we
		// need to register the GIL (thread state structure) towards that thread
		// so that the Python interpreter can see it and execute the code without
		// crashing Maya.
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

	// NOTE: (sonictk) We do not decrement the reference count to the module
	// here, because on the off-chance that someone unloads the plugin and then
	// reloads it after, we want them to be able to still use the same function
	// call without having to re-import the module again fresh. Otherwise, we
	// will trigger a segfault.  While this means an end-user could be running
	// "older" code (if the library changed between compilations), this is
	// unlikely to happen in production. It's more likely that plugins will get
	// loaded/unloaded at times rather than the end-user re-compiling the plugin
	// to have different behaviour in the bindings.

	return status;
}
