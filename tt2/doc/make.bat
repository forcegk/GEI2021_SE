@echo off

set "FLAGS=--shell-escape"

:: Default
if "%1"=="" goto :refs
if "%1"=="default" goto :refs
if "%1"=="refs" goto :refs
if "%1"=="references" goto :refs
if "%1"=="release" goto :release
if "%1"=="dirty" goto :dirty
if "%1"=="clean" goto :clean
if "%1"=="c" goto :clean
goto :EOF

:dirty
pdflatex %FLAGS% main
goto :EOF

:refs
pdflatex %FLAGS% main
biber main
pdflatex %FLAGS% main
goto :EOF

:release
pdflatex %FLAGS% main
biber main
pdflatex %FLAGS% main
pdflatex %FLAGS% main
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
for %%a in ("*.pyg") do del "%%a"
for %%a in ("*.out") do del "%%a"
for %%a in ("preamble\*.aux") do del "%%a"
for %%a in ("_minted-main\*") do del "%%a"
rmdir _minted-main
goto :EOF
