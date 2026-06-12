@echo off
cls
:: Force la console Windows à utiliser l'encodage UTF-8 pour afficher les accents correctement
@chcp 65001 > nul
echo ==================================================
echo COMPILATION X64 : AssistantIA.dll (Environnement Natif)
echo ==================================================

:: 1. ISOLEMENT : On isole l'environnement pour éviter l'accumulation au fil des lancements
setlocal

:: On force le script à travailler dans son propre dossier
cd /d "%~dp0"

:: Chemin racine de tes Build Tools de Visual Studio
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
set "VCVARS_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"

:: 2. ESQUIVE SÉCURISÉE : On ne passe direct au build que si MSVC est actif ET configuré en x64 !
if "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo [INFO] Environnement MSVC x64 developpeur deja actif. Passage direct au build.
    goto compile
)

:: Vérification du script de configuration de Microsoft
if not exist "%VCVARS_PATH%" (
    echo [ERREUR] Le script vcvarsall.bat est introuvable a l'adresse specifiee.
    echo Verifie le chemin VS_PATH dans ce fichier .bat
    pause
    exit /b
)

echo Configuration de l'environnement Windows SDK et MSVC en x64...

:: Nettoyage temporaire du PATH local pour vcvarsall.bat
set "PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\"

:: Cette ligne charge TOUTES les variables d'environnement nécessaires pour le 64-bit natif (amd64)
call "%VCVARS_PATH%" amd64

:compile
echo.
echo Compilation des ressources...
rc /fo "src\AssistantIA.res" "src\AssistantIA.rc"

echo.
echo Compilation de l'Assistant IA...
cl  /O2 /MT /D UNICODE /D _UNICODE /EHsc /utf-8 /LD src\AssistantIA.cpp src\AssistantIA.res Python314_Compile\python314.lib user32.lib shlwapi.lib gdi32.lib /I SDK_Npp /I Python314_Compile\include /Fe:AssistantIA.dll /link /DELAYLOAD:python314.dll delayimp.lib

echo.
if %ERRORLEVEL% EQU 0 (
    echo [SUCCES] AssistantIA.dll 64-bit générée avec succès !
) else (
    echo [ERREUR] La compilation a échoué.
)
echo ==================================================
:: On nettoie la session avant de quitter
endlocal
pause