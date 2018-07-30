#ifndef MAYA_PYTHON_C_EXT_PY_UTIL_H
#define MAYA_PYTHON_C_EXT_PY_UTIL_H

#include <maya/MGlobal.h>


static const char ADD_TO_ACTIVE_SELECTION_LIST_DOCSTRING[] = "Adds the specified object with the given name to the active selection list.";


static PyObject *pyAddToActiveSelectionList(PyObject *self, PyObject *args);


#endif /* MAYA_PYTHON_C_EXT_PY_UTIL_H */
