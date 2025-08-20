@echo off
echo ========================================
echo Starting Agentie de Voiaj Server
echo ========================================

REM Check if server executable exists
if not exist "Agentie_de_Voiaj_Server\x64\Debug\Agentie_de_Voiaj_Server.exe" (
    echo ERROR: Server executable not found!
    echo Please run build.bat first to build the project.
    pause
    exit /b 1
)

REM Check if database is accessible (optional)
echo Checking database connection...
REM Add database check logic here if needed

REM Start server
echo.
echo Starting server on port 8080...
echo Press Ctrl+C to stop the server
echo.
echo Server logs will appear below:
echo ========================================

cd "Agentie_de_Voiaj_Server\x64\Debug"
start "Agentie de Voiaj Server" Agentie_de_Voiaj_Server.exe

echo.
echo Server started successfully!
echo Server is running in a new window.
echo.
echo To stop the server, close the server window or press Ctrl+C in that window.
echo.
pause

