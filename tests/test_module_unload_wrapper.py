import sys
import os
import maya.standalone


if __name__ == '__main__':
    maya.standalone.initialize()
    import maya.cmds as cmds

    sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'py'))

    import validate

    pluginPath = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'msbuild', 'maya_python_c_ext.mll')
    print('loading plugin from {}'.format(pluginPath))

    mpce = validate.load_maya_plugin_and_validate_bindings(pluginPath)

    mpce.hello_world_maya('my string')

    validate.unload_maya_plugin_and_invalidate_bindings('maya_python_c_ext')

    if validate.validate_bindings():
        mpce.hello_world_maya('my string')
    else:
        print('The bindings are no longer valid!')

    maya.standalone.uninitialize()
