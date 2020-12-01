@echo off

:: Default
if "%1"=="" goto :release
if "%1"=="default" goto :release
if "%1"=="release" goto :release
if "%1"=="dirty" goto :dirty
if "%1"=="clean" goto :clean
if "%1"=="c" goto :clean
goto :EOF

:dirty
pdflatex main
goto :EOF

:release
pdflatex main
biber main
pdflatex main
goto :EOF

:clean
for %%a in ("*.aux") do del "%%a"
for %%a in ("*.bbl") do del "%%a"
for %%a in ("*.bcf") do del "%%a"
for %%a in ("*.blg") do del "%%a"
for %%a in ("*.log") do del "%%a"
for %%a in ("*.pdf") do del "%%a"
for %%a in ("*.xml") do del "%%a"
for %%a in ("*.toc") do del "%%a"
goto :EOF
