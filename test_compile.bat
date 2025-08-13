@echo off
cd /d "C:\Users\steli\OneDrive\Desktop\Agentie_de_Voiaj\Agentie_de_Voiaj_Server"

echo Testing compilation after fixes...
echo.

:: Try to compile main project files
echo Testing header includes...
echo #include "Socket_Server.h" > test_headers.cpp
echo #include "Database_Manager.h" >> test_headers.cpp  
echo #include "utils.h" >> test_headers.cpp
echo #include "config.h" >> test_headers.cpp
echo int main() { return 0; } >> test_headers.cpp

:: Check if cl.exe is available (would need Visual Studio Developer Command Prompt)
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo cl.exe not found. Please run this from Visual Studio Developer Command Prompt.
    echo Or use Visual Studio to build the solution directly.
    echo.
    echo Fixed issues:
    echo - Winsock header conflicts resolved
    echo - Socket_Server friend class access fixed  
    echo - SOCKET_ERROR constant renamed to SOCKET_COMM_ERROR
    echo - size_t to int conversion warning fixed
    pause
    exit /b 1
)

echo Compiling test...
cl /EHsc /std:c++17 /I"C:\Users\steli\vcpkg\installed\x64-windows\include" /D_CRT_SECURE_NO_WARNINGS test_headers.cpp /link /LIBPATH:"C:\Users\steli\vcpkg\installed\x64-windows\lib" ws2_32.lib rpcrt4.lib psapi.lib crypt32.lib

if %errorlevel% == 0 (
    echo.
    echo SUCCESS: Headers compile without errors!
    echo.
    echo Now try building the full project in Visual Studio.
) else (
    echo.
    echo ERRORS: Check compilation output above.
)

del test_headers.cpp
if exist test_headers.exe del test_headers.exe  
if exist test_headers.obj del test_headers.obj

pause