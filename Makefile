compile:
	latex Ingrid.tex
	bibtex Ingrid

all: 
	latex Ingrid.tex
	bibtex Ingrid
	latex Ingrid.tex
	latex Ingrid.tex

ps: Ingrid.dvi
	dvips  -o Ingrid.ps Ingrid.dvi

pdf: ps
	ps2pdf -dMaxSubsetPct=100 -dCompatibilityLevel=1.3 -dSubsetFonts=true -dEmbedAllFonts=true  -dAutoFilterColorImages=false -dAutoFilterGrayImages=false Ingrid.ps Ingrid.pdf

macpdf: ps
	pstopdf Ingrid.ps Ingrid.pdf

stable: compile
	cp Ingrid.pdf fgcs08-paper.pdf

clean:
	rm -f *.aux *.ps Ingrid.pdf *~ *.log *.bbl *.mtc* *.dvi *.out *.blg *.inx
