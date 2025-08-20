@echo off
echo ========================================
echo Starting Agentie de Voiaj Client
echo ========================================

REM Check if client executable exists
if not exist "Agentie_de_Voiaj_Client\x64\Debug\Agentie_de_Voiaj_Client.exe" (
    echo ERROR: Client executable not found!
    echo Please run build.bat first to build the project.
    pause
    exit /b 1
)

REM Check if server is running (optional)
echo Checking if server is accessible...
powershell -Command "try { $null = [System.Net.Sockets.TcpClient]::new('127.0.0.1', 8080); Write-Host 'Server is accessible'; exit 0 } catch { Write-Host 'Server is not accessible on port 8080'; exit 1 }"
if %errorlevel% neq 0 (
    echo WARNING: Server does not appear to be running on port 8080.
    echo The client may not be able to connect to the server.
    echo Please run run_server.bat first to start the server.
    echo.
    echo Press any key to continue anyway, or close this window to cancel...
    pause >nul
)

REM Start client
echo.
echo Starting client application...
echo.
echo Client will connect to server at 127.0.0.1:8080
echo Make sure the server is running before using the client.
echo.

cd "Agentie_de_Voiaj_Client\x64\Debug"
start "Agentie de Voiaj Client" Agentie_de_Voiaj_Client.exe

echo.
echo Client started successfully!
echo Client is running in a new window.
echo.
echo To stop the client, close the client window.
echo.
pause

