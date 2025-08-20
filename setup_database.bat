@echo off
echo ========================================
echo Setting up Agentie de Voiaj Database
echo ========================================

REM Check if SQL Server is accessible
echo Checking SQL Server connection...
sqlcmd -S localhost -E -Q "SELECT @@VERSION" >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Cannot connect to SQL Server!
    echo Please ensure SQL Server is running and accessible.
    echo You may need to:
    echo   1. Start SQL Server service
    echo   2. Enable SQL Server Authentication
    echo   3. Check firewall settings
    echo   4. Verify connection string in config.h
    pause
    exit /b 1
)

echo SQL Server connection successful!

REM Create database if it doesn't exist
echo.
echo Creating database if it doesn't exist...
sqlcmd -S localhost -E -Q "IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'AgentieVoiaj') CREATE DATABASE AgentieVoiaj;"
if %errorlevel% neq 0 (
    echo ERROR: Failed to create database!
    pause
    exit /b 1
)

echo Database ready!

REM Run SQL scripts in order
echo.
echo Running database setup scripts...

REM Tables
echo Creating tables...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\tables.sql"
if %errorlevel% neq 0 (
    echo ERROR: Failed to create tables!
    pause
    exit /b 1
)

REM Indexes
echo Creating indexes...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\indexes.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create indexes (continuing...)
)

REM Stored procedures
echo Creating stored procedures...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\stored_procedures.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create stored procedures (continuing...)
)

REM Views
echo Creating views...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\views.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create views (continuing...)
)

REM Triggers
echo Creating triggers...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\triggers.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to create triggers (continuing...)
)

REM Insert sample data
echo Inserting sample data...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\data_insertion.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to insert sample data (continuing...)
)

REM Extra data
echo Inserting extra data...
sqlcmd -S localhost -E -d AgentieVoiaj -i "Agentie_de_Voiaj_Server\sql\extra.sql"
if %errorlevel% neq 0 (
    echo WARNING: Failed to insert extra data (continuing...)
)

echo.
echo ========================================
echo Database setup completed!
echo ========================================
echo.
echo Database: AgentieVoiaj
echo Server: localhost
echo Authentication: Windows Authentication
echo.
echo The database is now ready for use by the application.
echo.
pause
