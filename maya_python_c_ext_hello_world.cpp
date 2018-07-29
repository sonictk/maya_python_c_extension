#include "maya_python_c_ext_hello_world.h"
#include <maya/MGlobal.h>


void helloWorldMaya()
{
	MGlobal::displayInfo("Hello world from the Maya Python C extension!");

	return;
}
