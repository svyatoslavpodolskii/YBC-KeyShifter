@echo off
setlocal
pushd "%~dp0"

where cmake >nul 2>nul || (echo CMake not found & exit /b 1)
where git >nul 2>nul || (echo Git not found & exit /b 1)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo Visual Studio Installer vswhere.exe not found.
	goto :fail
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
	echo No Visual C++ Build Tools installation was found.
	echo Install the standalone C++ Build Tools workload; the Visual Studio IDE is not required.
	goto :fail
)

set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
	echo MSVC environment script not found: "%VCVARS%"
	goto :fail
)

set "NINJA=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
if not exist "%NINJA%" for /f "delims=" %%i in ('where.exe ninja 2^>nul') do set "NINJA=%%i"
if not exist "%NINJA%" (
	echo Ninja was not found in the Build Tools installation or PATH.
	goto :fail
)

call "%VCVARS%" x64 >nul
if errorlevel 1 (
	echo Failed to initialize the MSVC x64 environment.
	goto :fail
)

if exist build\CMakeCache.txt (
	findstr /x /c:"CMAKE_GENERATOR:INTERNAL=Ninja" build\CMakeCache.txt >nul
	if errorlevel 1 (
		echo Removing CMake cache from the previous generator...
		cmake -E rm -rf build\CMakeCache.txt build\CMakeFiles
		if errorlevel 1 goto :fail
	)
)

cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="%NINJA%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :fail

cmake --build build --target ResampleCalc_VST3 ResampleCalc_Standalone --parallel
if errorlevel 1 goto :fail

echo.
echo Build complete.
echo VST3: build\ResampleCalc_artefacts\Release\VST3\YBC KeyShifter.vst3
popd
exit /b 0

:fail
popd
exit /b 1
