@echo off
echo ========================================
echo Running Agentie de Voiaj Tests
echo ========================================

REM Check if test executables exist
if not exist "Agentie_de_Voiaj_Server\tests\unit_tests.exe" (
    echo ERROR: Server unit tests not found!
    echo Please run build.bat first to build the tests.
    pause
    exit /b 1
)

if not exist "Agentie_de_Voiaj_Client\tests\client_tests.exe" (
    echo ERROR: Client tests not found!
    echo Please run build.bat first to build the tests.
    pause
    exit /b 1
)

REM Run server tests
echo.
echo ========================================
echo Running Server Unit Tests
echo ========================================
cd "Agentie_de_Voiaj_Server\tests"
unit_tests.exe
set server_test_result=%errorlevel%

echo.
echo Server tests completed with exit code: %server_test_result%

REM Run client tests
echo.
echo ========================================
echo Running Client Tests
echo ========================================
cd "..\..\Agentie_de_Voiaj_Client\tests"
client_tests.exe
set client_test_result=%errorlevel%

echo.
echo Client tests completed with exit code: %client_test_result%

REM Run integration tests
echo.
echo ========================================
echo Running Integration Tests
echo ========================================
integration_tests.exe
set integration_test_result=%errorlevel%

echo.
echo Integration tests completed with exit code: %integration_test_result%

REM Summary
echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Server Tests: %server_test_result% (0 = success, non-zero = failure)
echo Client Tests: %client_test_result% (0 = success, non-zero = failure)
echo Integration Tests: %integration_test_result% (0 = success, non-zero = failure)

if %server_test_result% equ 0 (
    echo Server Tests: PASSED
) else (
    echo Server Tests: FAILED
)

if %client_test_result% equ 0 (
    echo Client Tests: PASSED
) else (
    echo Client Tests: FAILED
)

if %integration_test_result% equ 0 (
    echo Integration Tests: PASSED
) else (
    echo Integration Tests: FAILED
)

if %server_test_result% equ 0 if %client_test_result% equ 0 if %integration_test_result% equ 0 (
    echo.
    echo ALL TESTS PASSED! ^_^
) else (
    echo.
    echo SOME TESTS FAILED! Please check the output above for details.
)

echo.
pause
