@echo off
REM    This is the Windows build script for the Maya example Python C extension.
REM    usage: build.bat [debug|release] [2012|2015|2017]
REM    e.g. ``build.bat release 2012`` will build in release mode using MSVC 2012 (Maya 2016 release).
REM         ``build.bat debug 2017`` will build in debug mode using MSVC 2017.
REM    If no arguments are specified, will default to building in release mode with MSVC 2015. (Maya 2018 release).

echo Build script started executing at %time% ...

REM Process command line arguments
set BuildType=%1
if "%BuildType%"=="" (set BuildType=release)

set MSVCCompilerVersion=%2
if "%MSVCCompilerVersion%"=="" (set MSVCCompilerVersion=2015)


REM    Set up the Visual Studio environment variables for calling the MSVC compiler;
REM    we do this after the call to pushd so that the top directory on the stack
REM    is saved correctly; the check for DevEnvDir is to make sure the vcvarsall.bat
REM    is only called once per-session (since repeated invocations will screw up
REM    the environment)
if not defined DevEnvDir (
    if "%MSVCCompilerVersion%"=="2017" (
        call "%vs2017installdir%\VC\Auxiliary\Build\vcvarsall.bat" x64
        goto start_build
    )

    if "%MSVCCompilerVersion%"=="2015" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
        goto start_build
    )

    if "%MSVCCompilerVersion%"=="2012" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
        goto start_build
    )
)

:start_build

REM    Make a build directory to store artifacts; remember, %~dp0 is just a special
REM    FOR variable reference in Windows that specifies the current directory the
REM    batch script is being run in
set BuildDir=%~dp0msbuild

if "%BuildType%"=="clean" (
    REM This allows execution of expressions at execution time instead of parse time, for user input
    setlocal EnableDelayedExpansion
    echo Cleaning build from directory: %BuildDir%. Files will be deleted^^!
    echo Continue ^(Y/N^)^?
    set /p ConfirmCleanBuild=
    if "!ConfirmCleanBuild!"=="Y" (
        echo Removing files in %BuildDir%...
        del /s /q %BuildDir%\*.*
    )
    goto end
)


echo Building in directory: %BuildDir%

if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%


REM    Set up globals
set MayaRootDir=C:\Program Files\Autodesk\Maya2018
set MayaIncludeDir=%MayaRootDir%\include

set ProjectName=maya_python_c_ext

set MayaPluginEntryPoint=%~dp0%ProjectName%_plugin_main.cpp
set PythonModuleEntryPoint=%~dp0%ProjectName%_py_mod_main.cpp


REM    We pipe errors to null, since we don't care if it fails
del *.pdb > NUL 2> NUL


REM    Setup all the compiler flags
set CommonCompilerFlags=/c /MP /W3 /WX- /Gy /Zc:wchar_t /Zc:forScope /Zc:inline /openmp /fp:precise /nologo /EHsc /MD /D REQUIRE_IOSTREAM /D _CRT_SECURE_NO_WARNINGS /D _BOOL /D NT_PLUGIN /D _WINDLL /D _MBCS /Gm- /GS /Gy /Gd /TP /Fo"%BuildDir%\%ProjectName%.obj"

REM    Add the include directories for header files
set CommonCompilerFlags=%CommonCompilerFlags% /I"%MayaRootDir%\include" /I "%MayaRootDir%\include\python2.7"

set CommonCompilerFlagsDebug=/Zi /Od %CommonCompilerFlags%
set CommonCompilerFlagsRelease=/O2 %CommonCompilerFlags%

set MayaPluginCompilerFlagsDebug=%CommonCompilerFlagsDebug% %MayaPluginEntryPoint%
set MayaPluginCompilerFlagsRelease=%CommonCompilerFlagsRelease% %MayaPluginEntryPoint%

set PythonModuleCompilerFlagsDebug=%CommonCompilerFlagsDebug% %PythonModuleEntryPoint%
set PythonModuleCompilerFlagsRelease=%CommonCompilerFlagsRelease% %PythonModuleEntryPoint%


REM    Setup all the linker flags
set CommonLinkerFlags= /nologo /incremental:no /opt:ref /manifest /manifestuac:"level='asInvoker' uiAccess='false'" /manifest:embed /subsystem:console /tlbid:1 /dynamicbase /nxcompat /machine:x64  /machine:x64 /dll

REM    Add all the Maya libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\OpenMayaAnim.lib" "%MayaRootDir%\lib\OpenMayaFX.lib" "%MayaRootDir%\lib\OpenMayaRender.lib" "%MayaRootDir%\lib\OpenMayaUI.lib" "%MayaRootDir%\lib\Foundation.lib" "%MayaRootDir%\lib\IMFbase.lib" "%MayaRootDir%\lib\clew.lib" "%MayaRootDir%\lib\Image.lib"  "%MayaRootDir%\lib\python27.lib"

REM    Now add the OS libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% Shlwapi.lib Kernel32.lib user32.lib gdi32.lib winspool.lib Shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib

set CommonLinkerFlagsDebug=%CommonLinkerFlags% /Debug
set CommonLinkerFlagsRelease=%CommonLinkerFlags%

set CommonLinkerFlags=/pdb:"%BuildDir%\%ProjectName%.pdb" /implib:"%BuildDir%\%ProjectName%.lib" "%BuildDir%\%ProjectName%.obj"

set MayaPluginExtension=mll
set PythonModuleExtension=pyd

set MayaPluginLinkerFlagsCommon=/export:initializePlugin /export:uninitializePlugin /out:"%BuildDir%\%ProjectName%.%MayaPluginExtension%"
set PythonModuleLinkerFlagsCommon=/export:initmaya_python_c_ext /out:"%BuildDir%\%ProjectName%.%PythonModuleExtension%"


set MayaPluginLinkerFlagsRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlags% %MayaPluginLinkerFlagsCommon%
set MayaPluginLinkerFlagsDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlags% %MayaPluginLinkerFlagsCommon%

set PythonModuleLinkerFlagsRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlags% %PythonModuleLinkerFlagsCommon%
set PythonModuleLinkerFlagsDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlags% %PythonModuleLinkerFlagsCommon%


if "%BuildType%"=="debug" (
    echo Building in debug mode...

    set MayaPluginCompilerFlags=%MayaPluginCompilerFlagsDebug%
    set MayaPluginLinkerFlags=%MayaPluginLinkerFlagsDebug%

    set PythonModuleCompilerFlags=%PythonModuleCompilerFlagsDebug%
    set PythonModuleLinkerFlags=%PythonModuleLinkerFlagsDebug%

) else (
    echo Building in release mode...

    set MayaPluginCompilerFlags=%MayaPluginCompilerFlagsRelease%
    set MayaPluginLinkerFlags=%MayaPluginLinkerFlagsRelease%

    set PythonModuleCompilerFlags=%PythonModuleCompilerFlagsRelease%
    set PythonModuleLinkerFlags=%PythonModuleLinkerFlagsRelease%
)

REM Now build the standalone Python module first
echo Compiling Python module (command follows)...
echo cl %PythonModuleCompilerFlags%
cl %PythonModuleCompilerFlags%
if %errorlevel% neq 0 goto error


:link
echo Linking Python module (command follows)...
echo link %PythonModuleLinkerFlags%
link %PythonModuleLinkerFlags%
if %errorlevel% neq 0 goto error


REM Now build the Maya plugin
echo Compiling Maya plugin (command follows)...
echo cl %MayaPluginCompilerFlags%
cl %MayaPluginCompilerFlags%
if %errorlevel% neq 0 goto error


:link
echo Linking (command follows)...
echo link %MayaPluginLinkerFlags%
link %MayaPluginLinkerFlags%
if %errorlevel% neq 0 goto error
if %errorlevel% == 0 goto success


:error
echo ***************************************
echo *      !!! An error occurred!!!       *
echo ***************************************
goto end


:success
echo ***************************************
echo *    Build completed successfully!    *
echo ***************************************
goto end


:end
echo Build script finished execution at %time%.
popd
exit /b %errorlevel%
