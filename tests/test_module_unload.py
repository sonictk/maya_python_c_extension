import sys
import os
import maya.standalone

if __name__ == '__main__':
    maya.standalone.initialize()
    import maya.cmds as cmds

    pluginPath = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'msbuild', 'maya_python_c_ext.mll')

    print('loading plugin from {}'.format(pluginPath))

    cmds.loadPlugin(pluginPath)

    import maya_python_c_ext as mpce

    mpce.hello_world_maya('my string')

    cmds.unloadPlugin('maya_python_c_ext')

    maya_python_c_ext = None
    del sys.modules['maya_python_c_ext']

    mpce.hello_world_maya('my string')

    maya.standalone.uninitialize()
