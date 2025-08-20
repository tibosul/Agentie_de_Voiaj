@echo off
echo ========================================
echo AVAILABLE SCRIPTS - Agentie de Voiaj
echo ========================================
echo.

echo [SYSTEM CHECK]
echo quick_test.bat          - Quick system health check
echo check_system.bat        - Detailed system verification
echo.

echo [DATABASE MANAGEMENT]
echo fresh_database.bat      - Drop and recreate database (FRESH START)
echo setup_database.bat      - Create database and tables
echo test_database.bat       - Test database step by step
echo.

echo [BUILD and CLEANUP]
echo build.bat               - Build all projects
echo cleanup.bat             - Remove build artifacts
echo reset_all.bat           - Complete system reset (FRESH START)
echo.

echo [RUNNING APPLICATIONS]
echo run_server.bat          - Start the server
echo run_client.bat          - Start the client
echo.

echo [TESTING]
echo run_tests.bat           - Run all tests
echo.

echo ========================================
echo RECOMMENDED WORKFLOWS
echo ========================================
echo.

echo [FIRST TIME SETUP]
echo 1. quick_test.bat
echo 2. fresh_database.bat (optional)
echo 3. setup_database.bat
echo 4. build.bat
echo 5. run_server.bat
echo 6. run_client.bat
echo.

echo [DAILY USE]
echo 1. run_server.bat
echo 2. run_client.bat
echo.

echo [WHEN YOU HAVE PROBLEMS]
echo 1. quick_test.bat (diagnose)
echo 2. reset_all.bat (fresh start)
echo.

echo [FOR TESTING]
echo run_tests.bat
echo.

echo ========================================
echo Script listing completed!
echo ========================================
echo.
pause
