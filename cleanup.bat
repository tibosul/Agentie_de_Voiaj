@echo off
echo ========================================
echo Cleaning Agentie de Voiaj Project
echo ========================================

echo Removing build artifacts...

REM Remove server build files
if exist "Agentie_de_Voiaj_Server\x64" (
    echo Removing server build directory...
    rmdir /s /q "Agentie_de_Voiaj_Server\x64"
)

REM Remove client build files
if exist "Agentie_de_Voiaj_Client\x64" (
    echo Removing client build directory...
    rmdir /s /q "Agentie_de_Voiaj_Client\x64"
)

REM Remove test executables
if exist "Agentie_de_Voiaj_Server\tests\unit_tests.exe" (
    echo Removing server test executable...
    del "Agentie_de_Voiaj_Server\tests\unit_tests.exe"
)

if exist "Agentie_de_Voiaj_Client\tests\client_tests.exe" (
    echo Removing client test executable...
    del "Agentie_de_Voiaj_Client\tests\client_tests.exe"
)

if exist "Agentie_de_Voiaj_Client\tests\integration_tests.exe" (
    echo Removing integration test executable...
    del "Agentie_de_Voiaj_Client\tests\integration_tests.exe"
)

REM Remove temporary files
if exist "*.tmp" (
    echo Removing temporary files...
    del "*.tmp"
)

if exist "*.log" (
    echo Removing log files...
    del "*.log"
)

REM Remove solution build files
if exist "x64" (
    echo Removing solution build directory...
    rmdir /s /q "x64"
)

echo.
echo ========================================
echo Cleanup completed successfully!
echo ========================================
echo.
echo All build artifacts and temporary files have been removed.
echo.
echo Next steps:
echo 1. Run fresh_database.bat for clean database (optional)
echo 2. Run setup_database.bat to recreate database
echo 3. Run build.bat to rebuild projects
echo.
pause

