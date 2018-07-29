#!/usr/bin/env bash

# This is the GCC build script for the Maya hot reloadable deformer.
# usage: build.sh [debug|release]


# Process command line arguments
BuildType=$1;
if [ "$BuildType" = "debug" ]; then
    echo "Building in debug mode...";
else
    echo "Building in release mode...";
    BuildType=release;
fi;


# Create a build directory to store artifacts
BuildDir=$PWD/linuxbuild;
echo "Building in directory: $buildDir";
if [ ! -d $BuildDir ];
   mkdir -p $BuildDir;
fi;

pushd $BuildDir;


# Set up globals
MayaRootDir=/usr/bin/autodesk/maya2016;
MayaIncludeDir=$MayaRootDir/include;
ThirdPartyDir=/home/sonictk/Git/experiments/maya_hot_reload_example/thirdparty;

HostEntryPoint=$PWD/src/plugin_main.cpp;
LogicEntryPoint=$PWD/src/logic.cpp;

OutputLogicDLLFilename=$BuildDir/logic.so;
OutputHostMLLFilename=$BuildDir/maya_hot_reload_example.so;

# todo: finish this

# Setup all the compiler flags
CommonCompilerFlags=-DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wall -std=c++11 -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char -pthread -Wno-deprecated -Wno-reorder -ftemplate-depth-25 -fno-gnu-keywords;

# Add the include directories for header files
set CommonCompilerFlags=%CommonCompilerFlags% -I$MayaRootDir/include -I$ThirdPartyDir;

set CommonCompilerFlagsDebug=-ggdb -O0 $CommonCompilerFlags;
set CommonCompilerFlagsRelease=-O3 $CommonCompilerFlags;

set CompilerFlagsHostDebug=$CommonCompilerFlagsDebug $HostEntryPoint;
set CompilerFlagsHostRelease=$CommonCompilerFlagsRelease$ $HostEntryPoint;

set CompilerFlagsLogicDebug=$CommonCompilerFlagsDebug $LogicEntryPoint;
set CompilerFlagsLogicRelease=$CommonCompilerFlagsRelease $LogicEntryPoint;

echo "Build complete!";
popd;
