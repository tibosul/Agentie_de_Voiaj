@echo off
echo === Copierea DLL-urilor Qt pentru Debug ===

REM Qt Core, GUI, Widgets, Network - Debug versions
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Cored.dll" "x64\Debug\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Guid.dll" "x64\Debug\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Widgetsd.dll" "x64\Debug\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Networkd.dll" "x64\Debug\" >nul

REM Crearea directoarelor pentru plugin-uri
if not exist "x64\Debug\platforms" mkdir "x64\Debug\platforms"
if not exist "x64\Debug\imageformats" mkdir "x64\Debug\imageformats"

REM Platform plugins - Debug versions
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qwindowsd.dll" "x64\Debug\platforms\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qminimald.dll" "x64\Debug\platforms\" >nul

REM Image format plugins - Debug versions
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qjpegd.dll" "x64\Debug\imageformats\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qgifd.dll" "x64\Debug\imageformats\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qsvgd.dll" "x64\Debug\imageformats\" >nul

echo DLL-urile si plugin-urile Qt au fost copiate cu succes in x64\Debug\

echo.
echo === Copierea DLL-urilor Qt pentru Release ===

REM Creaza directorul Release daca nu exista
if not exist "x64\Release\" mkdir "x64\Release\"

REM Qt Core, GUI, Widgets, Network - Release versions
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Core.dll" "x64\Release\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Gui.dll" "x64\Release\" >nul  
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Widgets.dll" "x64\Release\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Network.dll" "x64\Release\" >nul

REM Crearea directoarelor pentru plugin-uri
if not exist "x64\Release\platforms" mkdir "x64\Release\platforms"
if not exist "x64\Release\imageformats" mkdir "x64\Release\imageformats"

REM Platform plugins - Release versions
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qwindows.dll" "x64\Release\platforms\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qminimal.dll" "x64\Release\platforms\" >nul

REM Image format plugins - Release versions
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qjpeg.dll" "x64\Release\imageformats\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qgif.dll" "x64\Release\imageformats\" >nul
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qsvg.dll" "x64\Release\imageformats\" >nul

echo DLL-urile si plugin-urile Qt au fost copiate cu succes in x64\Release\

echo.
echo === Operatiunea s-a terminat cu succes! ===
pause