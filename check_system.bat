@echo off
echo ========================================
echo Agentie de Voiaj System Check
echo ========================================
echo.

set all_checks_passed=true

REM Check Windows version
echo Checking Windows version...
ver | findstr "10.0\|11.0" >nul
if %errorlevel% neq 0 (
    echo [ERROR] Windows 10/11 required. Current version:
    ver
    set all_checks_passed=false
) else (
    echo [OK] Windows version compatible
)

REM Check Visual Studio
echo.
echo Checking Visual Studio...
where msbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] MSBuild not found. Please install Visual Studio with C++ tools.
    set all_checks_passed=false
) else (
    echo [OK] Visual Studio C++ tools found
    msbuild /version | findstr "Microsoft"
)

REM Check Qt installation
echo.
echo Checking Qt installation...
if exist "C:\Qt" (
    echo [OK] Qt directory found at C:\Qt
    dir "C:\Qt" /b | findstr "6\." >nul
    if %errorlevel% neq 0 (
        echo [WARNING] Qt 6.x not found in C:\Qt
    ) else (
        echo [OK] Qt 6.x found
    )
) else (
    echo [ERROR] Qt directory not found. Please install Qt 6.x
    set all_checks_passed=false
)

REM Check vcpkg
echo.
echo Checking vcpkg...
if exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo [OK] vcpkg found at %VCPKG_ROOT%
) else if exist "C:\vcpkg\vcpkg.exe" (
    echo [OK] vcpkg found at C:\vcpkg
    set VCPKG_ROOT=C:\vcpkg
) else (
    echo [ERROR] vcpkg not found. Please install vcpkg package manager.
    set all_checks_passed=false
)

REM Check required packages
if defined VCPKG_ROOT (
    echo.
    echo Checking required packages...
    
    %VCPKG_ROOT%\vcpkg.exe list | findstr "nlohmann-json" >nul
    if %errorlevel% neq 0 (
        echo [ERROR] nlohmann-json package not installed
        echo Run: vcpkg install nlohmann-json:x64-windows
        set all_checks_passed=false
    ) else (
        echo [OK] nlohmann-json package installed
    )
    
    %VCPKG_ROOT%\vcpkg.exe list | findstr "openssl" >nul
    if %errorlevel% neq 0 (
        echo [ERROR] openssl package not installed
        echo Run: vcpkg install openssl:x64-windows
        set all_checks_passed=false
    ) else (
        echo [OK] openssl package installed
    )
)

REM Check SQL Server
echo.
echo Checking SQL Server...
sqlcmd -S localhost -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] SQL Server not accessible
    echo Please ensure SQL Server is running and accessible
    set all_checks_passed=false
) else (
    echo [OK] SQL Server connection successful
    sqlcmd -S localhost -E -Q "SELECT @@VERSION" | findstr "Microsoft SQL Server"
)

REM Check ODBC drivers
echo.
echo Checking ODBC drivers...
odbcad32.exe /q | findstr "SQL Server" >nul
if %errorlevel% neq 0 (
    echo [WARNING] SQL Server ODBC driver not found
    echo This may cause database connection issues
) else (
    echo [OK] SQL Server ODBC driver found
)

REM Check network ports
echo.
echo Checking network configuration...
netstat -an | findstr ":8080" >nul
if %errorlevel% equ 0 (
    echo [WARNING] Port 8080 is already in use
    echo This may prevent the server from starting
) else (
    echo [OK] Port 8080 is available
)

REM Check project files
echo.
echo Checking project structure...
if not exist "Agentie_de_Voiaj.sln" (
    echo [ERROR] Solution file not found
    set all_checks_passed=false
) else (
    echo [OK] Solution file found
)

if not exist "Agentie_de_Voiaj_Server" (
    echo [ERROR] Server project directory not found
    set all_checks_passed=false
) else (
    echo [OK] Server project directory found
)

if not exist "Agentie_de_Voiaj_Client" (
    echo [ERROR] Client project directory not found
    set all_checks_passed=false
) else (
    echo [OK] Client project directory found
)

REM Check build scripts
echo.
echo Checking build scripts...
if not exist "build.bat" (
    echo [ERROR] build.bat not found
    set all_checks_passed=false
) else (
    echo [OK] build.bat found
)

if not exist "run_server.bat" (
    echo [ERROR] run_server.bat not found
    set all_checks_passed=false
) else (
    echo [OK] run_server.bat found
)

if not exist "run_client.bat" (
    echo [ERROR] run_client.bat not found
    set all_checks_passed=false
) else (
    echo [OK] run_client.bat found
)

REM Summary
echo.
echo ========================================
echo System Check Summary
echo ========================================

if "%all_checks_passed%"=="true" (
    echo.
    echo [SUCCESS] All system checks passed!
    echo.
    echo Your system is ready to build and run Agentie de Voiaj.
    echo.
    echo Next steps:
    echo 1. Run build.bat to build the project
    echo 2. Run setup_database.bat to setup the database
    echo 3. Run run_server.bat to start the server
    echo 4. Run run_client.bat to start the client
) else (
    echo.
    echo [FAILURE] Some system checks failed!
    echo.
    echo Please resolve the issues above before proceeding.
    echo.
    echo Common solutions:
    echo - Install missing software (Visual Studio, Qt, SQL Server)
    echo - Install required vcpkg packages
    echo - Configure SQL Server and ODBC
    echo - Check network and firewall settings
)

echo.
pause

