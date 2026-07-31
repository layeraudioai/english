@echo off
cd ..
setlocal enabledelayedexpansion

REM Prompt user for inputs
set /P "word=Enter the new word: "
set /P "definition=Enter the definitive description: "

REM Ensure target folder exists
if not exist "words" (
    echo ERROR: 'words' folder not found.
    exit /b 1
)

cd words

REM Copy template folder to new folder
xcopy "template_word" "%word%" /E /I /Y >nul
if errorlevel 1 (
    echo ERROR: Failed to copy template_word.
    exit /b 1
)

cd "%word%"

REM Process all files in the new folder
for %%f in (*.*) do (
    REM Create a temp file for modified content
    set "outfile=%%~dpnf.tmp"
    > "!outfile!" (
        for /f "usebackq delims=" %%A in ("%%f") do (
            set "line=%%A"
            REM Replace placeholders
            set "line=!line:template=%word%!"
            set "line=!line:bremplate_word=%definition%!"
            echo(!line!
        )
    )
    REM Replace original file with modified file
    move /Y "!outfile!" "%%f" >nul
    move "%%f" "%word%"%%~xf
)

cd ..
endlocal
echo Done.
pause
