#ifndef MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H
#define MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H

// NOTE: (sonictk) Python has its own defines that require it to be included ahead
// of everything else.
#include <Python.h>
#include "maya_python_c_ext_py_mod_common.h"

// NOTE: (sonictk) Setup Unity build here
#include "maya_python_c_ext_hello_world.cpp"
#include "maya_python_c_ext_util.cpp"

#include "maya_python_c_ext_py_hello_world.cpp"
#include "maya_python_c_ext_py_util.cpp"


#endif /* MAYA_PYTHON_C_EXT_PLUGIN_MAIN_H */
