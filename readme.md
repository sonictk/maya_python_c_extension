# Maya Python C++ Extension Example

This is an example of how to create a Python C++ extension for Maya. It also contains 
a full tutorial of how to write the code. The tutorial is also available [here](https://sonictk.github.io/maya_python_c_extension/).


# Building

## Requirements

- Windows (Linux should work, but is not the focus of this tutorial)
- Maya 2018 and above, along with the devkit for the corresponding version.

## Instructions

Run ``build.bat`` on Windows or ``build.sh`` if you are on Linux. A new ``msbuild``/``linuxbuild`` 
folder should be created with the built plugin and Python module inside.

If compile/link errors occur, check that the Maya library/header directories are set correctly in 
the build script. They're set to default locations by default.


# Credits

Siew Yi Liang (a.k.a **sonictk**)


# License

License details are available in the ``LICENSE`` file.
