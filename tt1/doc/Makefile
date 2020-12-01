# Paquetes necesarios (Arch Linux)
# texlive-most texlive-lang biber
# Para Ubuntu/Debian
# texlive-full
# Y para windows o mac, MIKTEX o Texlive

.DEFAULT_GOAL := default

default: release

release: dirty
	biber main
	pdflatex main

dirty: Makefile
	pdflatex main

c: clean
clean:
	rm -f *.aux *.bbl *.bcf *.blg *.log *.pdf *.xml *.toc