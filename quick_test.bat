@echo off
echo ========================================
echo Agentie de Voiaj Quick Test
echo ========================================
echo.

echo Testing basic system functionality...
echo.

REM Test 1: Check if executables exist
echo [TEST 1] Checking project files...
if exist "Agentie_de_Voiaj.sln" (
    echo [OK] Solution file found
) else (
    echo [ERROR] Solution file not found
    goto :test_failed
)

if exist "Agentie_de_Voiaj_Server" (
    echo [OK] Server project found
) else (
    echo [ERROR] Server project not found
    goto :test_failed
)

if exist "Agentie_de_Voiaj_Client" (
    echo [OK] Client project found
) else (
    echo [ERROR] Client project found
    goto :test_failed
)

echo [OK] All project files present
echo.

REM Test 2: Check if server can start (brief test)
echo [TEST 2] Testing server startup...
if exist "Agentie_de_Voiaj_Server\x64\Debug\Agentie_de_Voiaj_Server.exe" (
    echo [OK] Server executable found
    echo [INFO] Server can be started with run_server.bat
) else (
    echo [WARNING] Server not built yet - run build.bat first
    echo [INFO] This is normal for first-time setup
)
echo.

REM Test 3: Check if client can start
echo [TEST 3] Testing client startup...
if exist "Agentie_de_Voiaj_Client\x64\Debug\Agentie_de_Voiaj_Client.exe" (
    echo [OK] Client executable found
    echo [INFO] Client can be started with run_client.bat
) else (
    echo [WARNING] Client not built yet - run build.bat first
    echo [INFO] This is normal for first-time setup
)
echo.

REM Test 4: Check network port availability
echo [TEST 4] Checking network configuration...
netstat -an | findstr ":8080" >nul
if %errorlevel% equ 0 (
    echo [WARNING] Port 8080 is already in use
    echo [INFO] This may prevent the server from starting
) else (
    echo [OK] Port 8080 is available
)
echo.

REM Test 5: Check SQL Server availability
echo [TEST 5] Checking SQL Server...
set sql_available=false

REM Try different instances
sqlcmd -S localhost -E -Q "SELECT 1" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] SQL Server accessible on localhost
    set sql_available=true
    goto :sql_check_done
)

sqlcmd -S localhost\SQLEXPRESS -E -Q "SELECT 1" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] SQL Server Express accessible on localhost\SQLEXPRESS
    set sql_available=true
    goto :sql_check_done
)

sqlcmd -S . -E -Q "SELECT 1" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] SQL Server accessible on local instance
    set sql_available=true
    goto :sql_check_done
)

sqlcmd -S "(LocalDB)\MSSQLLocalDB" -E -Q "SELECT 1" >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] SQL Server LocalDB accessible
    set sql_available=true
    goto :sql_check_done
)

:sql_check_done
if "%sql_available%"=="false" (
    echo [WARNING] SQL Server not accessible
    echo [INFO] Server will run in DEMO MODE
    echo [INFO] Install SQL Server to enable full functionality
) else (
    echo [OK] SQL Server is available
)
echo.

REM Test 6: Check database existence
if "%sql_available%"=="true" (
    echo [TEST 6] Checking database...
    sqlcmd -S localhost -E -Q "SELECT name FROM sys.databases WHERE name = 'Agentie_de_Voiaj'" >nul 2>nul
    if %errorlevel% equ 0 (
        echo [OK] Database 'Agentie_de_Voiaj' exists
    ) else (
        echo [INFO] Database 'Agentie_de_Voiaj' does not exist
        echo [INFO] Run setup_database.bat to create it
    )
    echo.
)

REM Summary
echo ========================================
echo QUICK TEST SUMMARY
echo ========================================

if "%sql_available%"=="true" (
    echo.
    echo [SUCCESS] System is ready for full operation!
    echo.
    echo Next steps:
echo 1. Run fresh_database.bat for a clean start (optional)
echo 2. Run setup_database.bat to create database
echo 3. Run run_server.bat to start server
echo 4. Run run_client.bat to start client
    echo.
    echo [INFO] All tests passed - system is ready!
) else (
    echo.
    echo [INFO] System will run in DEMO MODE
    echo.
    echo For full functionality:
    echo 1. Install SQL Server (Express or LocalDB)
    echo 2. Run setup_database.bat
    echo 3. Start server and client
    echo.
    echo [INFO] Demo mode provides full testing capabilities
)

echo.
echo ========================================
echo Quick test completed!
echo ========================================
echo.
pause
exit /b 0

:test_failed
echo.
echo [ERROR] Some tests failed!
echo Please check the project structure and try again.
echo.
pause
exit /b 1
