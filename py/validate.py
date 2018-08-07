import maya.cmds as cmds

ARE_BINDINGS_VALID = False


def load_maya_plugin_and_validate_bindings(pluginPath):
    cmds.loadPlugin(pluginPath)
    import maya_python_c_ext

    ARE_BINDINGS_VALID = True

    return maya_python_c_ext


def unload_maya_plugin_and_invalidate_bindings(plugin):
    cmds.unloadPlugin(plugin)

    ARE_BINDINGS_VALID = False


def validate_bindings():
    return ARE_BINDINGS_VALID
