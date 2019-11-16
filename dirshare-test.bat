@echo off

if "%~1"=="" (
    echo.
    echo NEED .EXE
    goto error
)

if not exist ".test" mkdir .test
if not exist ".test\src" mkdir .test\src
if not exist ".test\dist" mkdir .test\dist

SET data=%random%%random%%random%%random%русские буквы%random%%random%%random%%random%
echo %data% > .test\src\src.txt


start cmd /c %1 -t sd -i .test\src ^> .test\server-out.txt

TIMEOUT 2

%1 -t cf -a localhost -i src.txt -o .test\dist\dist.txt > .test\client-out.txt



echo. 
echo *********************
echo         server
echo *********************
type .test\server-out.txt

echo. 
echo *********************
echo         client
echo *********************
type .test\client-out.txt

fc .test\src\src.txt .test\dist\dist.txt > nul

if errorlevel 1 goto error

goto suc

:error
echo. 
echo =====================
echo         FAIL
echo =====================
goto end

:suc
echo. 
echo =====================
echo        SUCCESS
echo =====================

:end