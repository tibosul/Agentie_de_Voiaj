@echo off
echo ========================================
echo Setting up Agentie de Voiaj Database
echo ========================================

REM Check if SQL Server is accessible
echo Checking SQL Server connection...
echo Trying different SQL Server instances...

set sql_connected=false

REM Try localhost first
sqlcmd -S localhost -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server on localhost
    set sql_connected=true
    set sql_instance=localhost
    goto :sql_found
)

REM Try SQL Server Express
sqlcmd -S localhost\SQLEXPRESS -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server Express on localhost\SQLEXPRESS
    set sql_connected=true
    set sql_instance=localhost\SQLEXPRESS
    goto :sql_found
)

REM Try local instance
sqlcmd -S . -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] Connected to SQL Server on local instance
    set sql_connected=true
    set sql_instance=.
    goto :sql_found
)

REM Try LocalDB
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
echo Please ensure SQL Server is running and accessible. Try:
echo   1. Start SQL Server service (Services.msc)
echo   2. Install SQL Server Express or LocalDB
echo   3. Check firewall settings
echo   4. Run as Administrator
echo.
echo Common SQL Server instance names:
echo   - localhost (default instance)
echo   - localhost\SQLEXPRESS (SQL Server Express)
echo   - . (local instance)
echo   - (LocalDB)\MSSQLLocalDB (SQL Server LocalDB)
echo.
pause
exit /b 1

:sql_found
echo SQL Server connection successful on: %sql_instance%

echo SQL Server connection successful!

REM Create database if it doesn't exist
echo.
echo Creating database if it doesn't exist...
echo Note: If you want a completely fresh database, run fresh_database.bat first
sqlcmd -S %sql_instance% -E -Q "IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'Agentie_de_Voiaj') CREATE DATABASE Agentie_de_Voiaj;"
if %errorlevel% neq 0 (
    echo ERROR: Failed to create database!
    echo.
    echo Possible solutions:
    echo 1. Ensure SQL Server is running
    echo 2. Check if you have permissions to create databases
    echo 3. Try running as Administrator
    echo 4. Check SQL Server instance name (try: localhost\SQLEXPRESS)
    echo.
    pause
    exit /b 1
)

echo Database ready!

REM Run SQL scripts in order
echo.
echo Running database setup scripts...

REM Check if SQL files exist
echo Checking SQL script files...
if not exist "Agentie_de_Voiaj_Server\sql\tables.sql" (
    echo ERROR: tables.sql not found!
    echo Current directory: %CD%
    echo Looking for: Agentie_de_Voiaj_Server\sql\tables.sql
    pause
    exit /b 1
)
echo [OK] SQL script files found
echo.

REM Tables
echo Creating tables...
echo Running: sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\tables.sql"
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\tables.sql"
set tables_result=%errorlevel%
echo Tables creation returned: %tables_result%

if %tables_result% neq 0 (
    echo ERROR: Failed to create tables! (Error code: %tables_result%)
    echo.
    echo This usually means:
    echo 1. Database doesn't exist or is not accessible
    echo 2. SQL script has syntax errors
    echo 3. Insufficient permissions
    echo.
    echo Try running fresh_database.bat first to ensure clean database
    echo.
    pause
    exit /b 1
)

REM Indexes
echo Creating indexes...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\indexes.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create indexes (continuing...)
)

REM Stored procedures
echo Creating stored procedures...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\stored_procedures.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create stored procedures (continuing...)
)

REM Views
echo Creating views...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\views.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create views (continuing...)
)

REM Triggers
echo Creating triggers...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\triggers.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create triggers (continuing...)
)

REM Insert sample data
echo Inserting sample data...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\data_insertion.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to insert sample data (continuing...)
)

REM Extra data
echo Inserting extra data...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\extra.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to insert extra data (continuing...)
)

echo.
echo ========================================
echo Database setup completed!
echo ========================================
echo.
echo Database: Agentie_de_Voiaj
echo Server: %sql_instance%
echo Authentication: Windows Authentication
echo.
echo The database is now ready for use by the application.
echo.
pause

