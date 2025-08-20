@echo off
echo ========================================
echo Database Test Script
echo ========================================
echo.
echo This script will test database creation step by step
echo.

REM Check if SQL Server is accessible
echo [STEP 1] Testing SQL Server connection...
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

:sql_not_found
echo ERROR: Cannot connect to any SQL Server instance!
pause
exit /b 1

:sql_found
echo SQL Server connection successful on: %sql_instance%
echo.

REM Step 2: Test database creation
echo [STEP 2] Testing database creation...
sqlcmd -S %sql_instance% -E -Q "IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'Agentie_de_Voiaj') CREATE DATABASE Agentie_de_Voiaj;"
if %errorlevel% neq 0 (
    echo ERROR: Failed to create database!
    pause
    exit /b 1
)
echo [OK] Database created/verified
echo.

REM Step 3: Test database connection
echo [STEP 3] Testing database connection...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -Q "SELECT 'Database connection successful' as Status;"
if %errorlevel% neq 0 (
    echo ERROR: Cannot connect to database!
    pause
    exit /b 1
)
echo [OK] Database connection successful
echo.

REM Step 4: Test table creation
echo [STEP 4] Testing table creation...
echo Running tables.sql...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -i "Agentie_de_Voiaj_Server\sql\tables.sql"
if %errorlevel% neq 0 (
    echo ERROR: Failed to create tables!
    echo Error code: %errorlevel%
    pause
    exit /b 1
)
echo [OK] Tables created successfully
echo.

REM Step 5: Verify tables exist
echo [STEP 5] Verifying tables exist...
sqlcmd -S %sql_instance% -E -d Agentie_de_Voiaj -Q "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_TYPE = 'BASE TABLE';"
if %errorlevel% neq 0 (
    echo ERROR: Cannot query tables!
    pause
    exit /b 1
)
echo [OK] Tables verified
echo.

echo ========================================
echo DATABASE TEST COMPLETED SUCCESSFULLY!
echo ========================================
echo.
echo Database: Agentie_de_Voiaj
echo Server: %sql_instance%
echo Status: Ready for use
echo.
pause
