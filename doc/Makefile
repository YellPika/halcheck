.PHONY: all format clean force

all: bin/slides.pdf

bin/%.pdf: %.tex force
	@mkdir -p $(@D)
	(cd $(@D) && yes x | \
		TEXINPUTS=$(dir $(abspath $<)): \
		BIBINPUTS=$(dir $(abspath $<)): \
		latexmk -xelatex -shell-escape -interaction=errorstopmode $(abspath $<))

format:
	latexindent $(shell find . -name '*.tex') -l -wd -s -c /tmp

clean:
	$(RM) -r bin
