"""Build script for the Maya Python C extension, plain roti version.
This is meant to be run using the standard Python packaging tools: i.e.:

python setup.py build
"""
import os
import setuptools
from distutils.core import setup, Extension

mayaRootDirectory = "C:\\Program Files\\Autodesk\\Maya2018"
mayaIncludeDirectory = os.path.join(mayaRootDirectory, "include")
mayaLibraryDirectory = os.path.join(mayaRootDirectory, "lib")

mayaPythonCExtModule = Extension("maya_python_c_ext",
                                 define_macros=[("REQUIRE_IOSTREAM", None),
                                                ("_CRT_SECURE_NO_WARNINGS", None),
                                                ("_BOOL", None),
                                                ("NT_PLUGIN", None),
                                                ("_WINDLL", None),
                                                ("_MBCS", None)],
                                 include_dirs=[mayaIncludeDirectory, os.path.join(mayaIncludeDirectory, "python2.7")],
                                 libraries=[
                                     "OpenMaya",
                                     "OpenMayaAnim",
                                     "OpenMayaFX",
                                     "OpenMayaRender",
                                     "OpenMayaUI",
                                     "Foundation",
                                     "clew",
                                     "Image",
                                     "IMFbase",
                                     "python27"
                                 ],
                                 library_dirs=[mayaLibraryDirectory],
                                 sources=["maya_python_c_ext_plugin_main.cpp"],
                                 extra_compile_args=['/EHsc'])

setup(name="maya_python_c_ext",
      version="1.0.0",
      description="Maya Python C Extension - the traditional Python version",
      author="Siew Yi Liang",
      ext_modules = [mayaPythonCExtModule])
