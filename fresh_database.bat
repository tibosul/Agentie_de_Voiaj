@echo off
echo ========================================
echo Fresh Database Creation
echo ========================================
echo.
echo This script will create a completely fresh database
echo by dropping the existing one (if it exists) and
echo creating a new one from scratch.
echo.
echo WARNING: This will DELETE all existing data!
echo.
set /p confirm="Are you sure you want to continue? (y/N): "
if /i not "%confirm%"=="y" (
    echo Operation cancelled.
    pause
    exit /b 0
)

echo.
echo Proceeding with fresh database creation...
echo.

REM Check if SQL Server is accessible
echo Checking SQL Server connection...
set sql_connected=false

REM Try different SQL Server instances
sqlcmd -S localhost -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server on localhost
    set sql_connected=true
    set sql_instance=localhost
    goto :sql_found
)

sqlcmd -S localhost\SQLEXPRESS -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server Express on localhost\SQLEXPRESS
    set sql_connected=true
    set sql_instance=localhost\SQLEXPRESS
    goto :sql_found
)

sqlcmd -S . -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server on local instance
    set sql_connected=true
    set sql_instance=.
    goto :sql_found
)

sqlcmd -S "(LocalDB)\MSSQLLocalDB" -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server LocalDB
    set sql_connected=true
    set sql_instance=(LocalDB)\MSSQLLocalDB
    goto :sql_found
)

:sql_not_found
echo ERROR: Cannot connect to any SQL Server instance!
echo.
echo Please ensure SQL Server is running and accessible.
echo.
pause
exit /b 1

:sql_found
echo SQL Server connection successful on: %sql_instance%
echo.

REM Run the fresh database script
echo Running fresh database creation script...
echo Running: sqlcmd -S %sql_instance% -E -i "Agentie_de_Voiaj_Server\sql\fresh_database.sql"
sqlcmd -S %sql_instance% -E -i "Agentie_de_Voiaj_Server\sql\fresh_database.sql"
set sql_result=%errorlevel%
echo sqlcmd returned: %sql_result%

if %sql_result% neq 0 (
    echo ERROR: Failed to create fresh database! (Error code: %sql_result%)
    echo.
    echo Possible solutions:
    echo 1. Ensure SQL Server is running
    echo 2. Check if you have permissions to create/drop databases
    echo 3. Try running as Administrator
    echo 4. Check if fresh_database.sql exists
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Fresh database created successfully!
echo ========================================
echo.
echo Database: Agentie_de_Voiaj (fresh)
echo Server: %sql_instance%
echo.
echo The database is now ready for table creation.
echo Run setup_database.bat to create tables and insert data.
echo.
pause
