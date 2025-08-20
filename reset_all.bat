@echo off
echo ========================================
echo COMPLETE SYSTEM RESET
echo ========================================
echo.
echo This script will:
echo 1. Clean all build artifacts
echo 2. Drop and recreate the database
echo 3. Rebuild all projects
echo.
echo WARNING: This will DELETE all data and start completely fresh!
echo.
set /p confirm="Are you sure you want to reset everything? (y/N): "
if /i not "%confirm%"=="y" (
    echo Operation cancelled.
    pause
    exit /b 0
)

echo.
echo Starting complete system reset...
echo.

REM Step 1: Clean everything
echo [STEP 1/4] Cleaning build artifacts...
call cleanup.bat
if %errorlevel% neq 0 (
    echo ERROR: Cleanup failed!
    pause
    exit /b 1
)

REM Step 2: Fresh database
echo.
echo [STEP 2/4] Creating fresh database...
call fresh_database.bat
if %errorlevel% neq 0 (
    echo ERROR: Database creation failed!
    pause
    exit /b 1
)

REM Step 3: Setup database with tables
echo.
echo [STEP 3/4] Setting up database tables...
call setup_database.bat
if %errorlevel% neq 0 (
    echo ERROR: Database setup failed!
    pause
    exit /b 1
)

REM Step 4: Build everything
echo.
echo [STEP 4/4] Building all projects...
call build.bat
if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo COMPLETE SYSTEM RESET SUCCESSFUL!
echo ========================================
echo.
echo Your system is now completely fresh and ready!
echo.
echo Next steps:
echo 1. Run run_server.bat to start the server
echo 2. Run run_client.bat to start the client
echo.
echo Enjoy your fresh start! 🎉
echo.
pause
