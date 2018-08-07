import os
import sys
import maya.standalone

if __name__ == '__main__':
    maya.standalone.initialize()

    pluginDir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'msbuild')
    sys.path.append(pluginDir)

    import maya_python_c_ext as mpce

    print mpce.are_bindings_valid()

    mpce.hello_world_maya('my string')

    maya.standalone.uninitialize()
