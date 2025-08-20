@echo off
echo ========================================
echo Building Agentie de Voiaj Solution
echo ========================================

REM Check if Visual Studio is available
where msbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: MSBuild not found. Please install Visual Studio or build tools.
    pause
    exit /b 1
)

REM Build Server
echo.
echo Building Server...
cd "Agentie_de_Voiaj_Server"
msbuild Agentie_de_Voiaj_Server.vcxproj /p:Configuration=Debug /p:Platform=x64
if %errorlevel% neq 0 (
    echo ERROR: Server build failed!
    pause
    exit /b 1
)
echo Server built successfully!

REM Build Client
echo.
echo Building Client...
cd "..\Agentie_de_Voiaj_Client"
msbuild Agentie_de_Voiaj_Client.vcxproj /p:Configuration=Debug /p:Platform=x64
if %errorlevel% neq 0 (
    echo ERROR: Client build failed!
    pause
    exit /b 1
)
echo Client built successfully!

REM Build Tests
echo.
echo Building Tests...
cd "..\Agentie_de_Voiaj_Server\tests"
msbuild unit_tests.cpp /Fe:unit_tests.exe /std:c++17
if %errorlevel% neq 0 (
    echo WARNING: Server tests build failed (continuing...)
)

cd "..\..\Agentie_de_Voiaj_Client\tests"
msbuild client_tests.cpp /Fe:client_tests.exe /std:c++17
if %errorlevel% neq 0 (
    echo WARNING: Client tests build failed (continuing...)
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Server executable: Agentie_de_Voiaj_Server\x64\Debug\Agentie_de_Voiaj_Server.exe
echo Client executable: Agentie_de_Voiaj_Client\x64\Debug\Agentie_de_Voiaj_Client.exe
echo.
pause

