doc = ram_paper_ssa
pdfs=$(doc:%=%.pdf)
dvis=$(doc:%=%.dvi)

all : $(pdfs)

#%.pdf: %.ps
#	ps2pdf -dCompatibilityLevel=1.3 -dMAxSubsetPct=100 -dSubsetFonts=true\
#		-dEmbedAllFonts=true -sPAPERSIZE=letter $<
#
#%.ps: %.dvi
#	dvips -Ppdf -G0 -tletter $<
#
%.pdf: %.tex includes/*.tex %.bib
	pdflatex $<
	bibtex `basename $< .tex`
	pdflatex $<
	pdflatex $<

clean:
	rm -f $(dvis) $(pdfs)
	rm -f *.aux *.bbl *.log *.blg

