#!/usr/bin/env bash

# This is the GCC build script for the Maya example Python C extension.
# usage: build.sh [debug|release]

StartTime=`date +%T`;
echo "Build script started executing at ${StartTime}...";

# Process command line arguments
BuildType=$1;

if [ "$BuildType" == "" ]; then
    BuildType="release";
fi;


# Define colours to be used for terminal output messages
RED='\033[0;31m';
GREEN='\033[0;32m';
NC='\033[0m'; # No Color


# If cleaning builds, just delete build artifacts and exit immediately
if [ "$BuildType" == "clean" ]; then
    echo "Cleaning build from directory: $BuildDir. Files will be deleted!";
    read -p "Continue? (Y/N)" ConfirmCleanBuild;
    if [ $ConfirmCleanBuild == [Yy] ]; then
       echo "Removing files in: $BuildDir...";
       rm -rf $BuildDir;
    fi;

    exit 0;
fi;


# Create a build directory to store artifacts
BuildDir="${PWD}/linuxbuild";
echo "Building in directory: $BuildDir";
if [ ! -d "$BuildDir" ]; then
   mkdir -p "$BuildDir";
fi;


# Set up globals
MayaRootDir="/usr/bin/autodesk/maya2018";
MayaIncludeDir="$MayaRootDir/include";
MayaLibraryDir="$MayaRootDir/lib";

ProjectName="maya_python_c_ext";

MayaPluginEntryPoint="${PWD}/${ProjectName}_plugin_main.cpp";
PythonModuleEntryPoint="${PWD}/${ProjectName}_py_mod_main.cpp";

# Setup all the compiler flags
CommonCompilerFlags="-DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wall -std=c++11 -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char -pthread -Wno-deprecated -Wno-reorder -ftemplate-depth-25 -fno-gnu-keywords -o \"${BuildDir}/${ProjectName}.o\"";

# Add the include directories for header files
CommonCompilerFlags="${CommonCompilerFlags} -I${MayaRootDir}/include -I${MayaRootDir}/include/python2.7";

CommonCompilerFlagsDebug="-ggdb -O0 ${CommonCompilerFlags}";
CommonCompilerFlagsRelease="-O3 ${CommonCompilerFlags}";

MayaPluginCompilerFlagsDebug="${CommonCompilerFlagsDebug} ${MayaPluginEntryPoint}";
MayaPluginCompilerFlagsRelease="${CommonCompilerFlagsRelease} ${MayaPluginEntryPoint}";

PythonModuleCompilerFlagsDebug="${CommonCompilerFlagsDebug} ${PythonModuleEntryPoint}";
PythonModuleCompilerFlagsRelease="${CommonCompilerFlagsRelease} ${PythonModuleEntryPoint}";

# As per the Maya official Makefile:
# -Bsymbolic binds references to global symbols within the library.
# This avoids symbol clashes in other shared libraries but forces
# the linking of all required libraries.
CommonLinkerFlags="-Wl,-Bsymbolic -shared";

# Add all the Maya libraries to link against
CommonLinkerFlags="${CommonLinkerFlags} ${MayaLibraryDir}/libOpenMaya.so ${MayaLibraryDir}/libOpenMayaAnim.so ${MayaLibraryDir}/libOpenMayaFX.so ${MayaLibraryDir}/libOpenMayaRender.so ${MayaLibraryDir}/libOpenMayaUI.so ${MayaLibraryDir}/libFoundation.so ${MayaLibraryDir}/libclew.so ${MayaLibraryDir}/libOpenMayalib.so ${MayaLibraryDir}/libOpenMaya.so ${MayaLibraryDir}/libAnimSlice.so ${MayaLibraryDir}/libDeformSlice.so ${MayaLibraryDir}/libModifiers.so ${MayaLibraryDir}/libDynSlice.so ${MayaLibraryDir}/libKinSlice.so ${MayaLibraryDir}/libModelSlice.so ${MayaLibraryDir}/libNurbsSlice.so ${MayaLibraryDir}/libPolySlice.so ${MayaLibraryDir}/libProjectSlice.so ${MayaLibraryDir}/libImage.so ${MayaLibraryDir}/libShared.so ${MayaLibraryDir}/libTranslators.so ${MayaLibraryDir}/libDataModel.so ${MayaLibraryDir}/libRenderModel.so ${MayaLibraryDir}/libNurbsEngine.so ${MayaLibraryDir}/libDependEngine.so ${MayaLibraryDir}/libCommandEngine.so ${MayaLibraryDir}/libFoundation.so ${MayaLibraryDir}/libIMFbase.so";

# Now add the OS libraries to link against
CommonLinkerFlags="${CommonLinkerFlags} libm libdl";

CommonLinkerFlagsDebug="${CommonLinkerFlags} -ggdb -O0";
CommonLinkerFlagsRelease="${CommonLinkerFlags} -O2";

MayaPluginExtension="so";
PythonModuleExtension="${MayaPluginExtension}";

MayaPluginLinkerFlagsCommon="-o ${BuildDir}/${ProjectName}.${MayaPluginExtension}";
PythonModuleLinkerFlagsCommon="-o ${BuildDir}/${ProjectName}.${PythonModuleExtension}";

MayaPluginLinkerFlagsRelease="${CommonLinkerFlagsRelease} ${CommonLinkerFlags} ${MayaPluginLinkerFlagsCommon}";
MayaPluginLinkerFlagsDebug="${CommonLinkerFlagsDebug} ${CommonLinkerFlags} ${MayaPluginLinkerFlagsCommon}";

PythonModuleLinkerFlagsRelease="${CommonLinkerFlagsRelease} ${CommonLinkerFlags} ${PythonModuleLinkerFlagsCommon}";
PythonModuleLinkerFlagsDebug="${CommonLinkerFlagsDebug} ${CommonLinkerFlags} ${PythonModuleLinkerFlagsCommon}";


if [ "$BuildType" == "debug" ]; then
    echo "Building in debug mode...";

    MayaPluginCompilerFlags="${MayaPluginCompilerFlagsDebug}";
    MayaPluginLinkerFlags="${MayaPluginLinkerFlagsDebug}";

    PythonModuleCompilerFlags="${PythonModuleCompilerFlagsDebug}";
    PythonModuleLinkerFlags="${PythonModuleLinkerFlagsDebug}";
else
    echo "Building in release mode...";

    MayaPluginCompilerFlags="${MayaPluginCompilerFlagsRelease}";
    MayaPluginLinkerFlags="${MayaPluginLinkerFlagsRelease}";

    PythonModuleCompilerFlags="${PythonModuleCompilerFlagsRelease}";
    PythonModuleLinkerFlags="${PythonModuleLinkerFlagsRelease}";
fi;


# Now build the standalone Python module first
echo "Compiling Python module (command follows)...";
echo "g++ ${PythonModuleCompilerFlags}";
echo "";

g++ ${PythonModuleCompilerFlags};

if [ $? -ne 0 ]; then
    echo -e "${RED}***************************************${NC}";
    echo -e "${RED}*      !!! An error occurred!!!       *${NC}";
    echo -e "${RED}***************************************${NC}";
    exit 1;
fi;


echo "Linking Python module (command follows)...";
echo "ld ${PythonModuleLinkerFlags}";
echo "";

ld ${PythonModuleLinkerFlags};

if [ $? -ne 0 ]; then
    echo -e "${RED}***************************************${NC}";
    echo -e "${RED}*      !!! An error occurred!!!       *${NC}";
    echo -e "${RED}***************************************${NC}";
    exit 2;
fi;


# Now build the Maya plugin
echo "Compiling Maya plugin (command follows)...";
echo "g++ ${MayaPluginCompilerFlags}";
echo "";

g++ ${MayaPluginCompilerFlags};

if [ $? -ne 0 ]; then
    echo -e "${RED}***************************************${NC}";
    echo -e "${RED}*      !!! An error occurred!!!       *${NC}";
    echo -e "${RED}***************************************${NC}";
    exit 3;
fi;

echo "Linking (command follows)...";
echo "ld ${MayaPluginLinkerFlags}";
echo "";

ld ${MayaPluginLinkerFlags};

if [ $? -ne 0 ]; then
    echo -e "${RED}***************************************${NC}";
    echo -e "${RED}*      !!! An error occurred!!!       *${NC}";
    echo -e "${RED}***************************************${NC}";
    exit 4;
fi;


echo -e "${GREEN}***************************************${NC}";
echo -e "${GREEN}*    Build completed successfully!    *${NC}";
echo -e "${GREEN}***************************************${NC}";


EndTime=`date +%T`;
echo "Build script finished execution at ${EndTime}.";

exit 0;
