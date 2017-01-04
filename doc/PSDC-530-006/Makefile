%.pdf : %.tex
	@find . -name $*.aux -cnewer $*.tex -exec rm {} \;
	@if [ ! -f $*.aux ]; then \
		echo $(PDFLATEX) $*.tex; \
		$(PDFLATEX) $*.tex; \
	fi
	$(PDFLATEX) $*.tex
#
PDFLATEX = pdflatex
#
clean :
	$(RM) *.log *.dvi *.aux *.toc *.log *.out *~ core

empty : clean
	@for f in `find . -maxdepth 1 -name \*.pdf`; do \
	    if [ -f `echo $$f | sed 's/pdf/tex/'` ]; then \
		echo $(RM) $$f; \
		$(RM) $$f; \
	    fi; \
	done
