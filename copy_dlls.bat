@echo off
echo Kopiere OpenAL DLLs ins Build-Verzeichnis...

set BUILD_DIR=out\build\x64-Debug
set OPENAL_DIR=extern\openal-soft

if not exist %BUILD_DIR% (
    echo Build-Verzeichnis existiert nicht: %BUILD_DIR%
    pause
    exit /b 1
)

echo Kopiere OpenAL32.dll...
copy "%OPENAL_DIR%\router\Win64\OpenAL32.dll" "%BUILD_DIR%\OpenAL32.dll" >nul

echo Kopiere soft_oal.dll...
copy "%OPENAL_DIR%\bin\Win64\soft_oal.dll" "%BUILD_DIR%\soft_oal.dll" >nul

echo Kopiere Runtime DLLs...
copy "%OPENAL_DIR%\alsoft-config\libgcc_s_seh-1.dll" "%BUILD_DIR%\libgcc_s_seh-1.dll" >nul
copy "%OPENAL_DIR%\alsoft-config\libstdc++-6.dll" "%BUILD_DIR%\libstdc++-6.dll" >nul
copy "%OPENAL_DIR%\alsoft-config\libwinpthread-1.dll" "%BUILD_DIR%\libwinpthread-1.dll" >nul

echo DLLs erfolgreich kopiert!
echo.
echo Du kannst jetzt Pong.exe starten.
pause



