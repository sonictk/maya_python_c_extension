#include "maya_python_c_ext_util.h"

#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>


MayaPythonCExtStatus addToActiveSelectionList(const char *name)
{
	MStatus stat;

	MSelectionList objList;
	stat = objList.add(name);
	if (!stat) {
		return MayaPythonCExtStatus::NODE_DOES_NOT_EXIST;
	}

	MSelectionList activeSelList;
	stat = MGlobal::getActiveSelectionList(activeSelList, true);
	if (!stat) {
		return MayaPythonCExtStatus::UNABLE_TO_GET_ACTIVE_SELECTION;
	}

	stat = activeSelList.merge(objList);
	if (!stat) {
		return MayaPythonCExtStatus::UNABLE_TO_MERGE_SELECTION_LISTS;
	}

	stat = MGlobal::setActiveSelectionList(activeSelList);
	if (!stat) {
		return MayaPythonCExtStatus::UNABLE_TO_SET_ACTIVE_SELECTION;
	}

	return MayaPythonCExtStatus::SUCCESS;
}
