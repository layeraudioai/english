@echo off
cd ..
echo Get ready
pause
cd %~dp0
echo %CD%
cd words
echo %CD%
for /D %%d in (*) do (
    clear
    echo "%%d"
    cd "%%d"
    ..\..\tools\gbmin -c "%%d.json" "%%d.json.h" 0
    Start /B ..\..\tools\gbmin -m "%%d.json.h" "%%d.json.min.h" 0
    cd ..
    clear
)
echo all done
pause
