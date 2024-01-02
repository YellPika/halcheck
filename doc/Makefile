.PHONY: all format clean force

all: bin/slides.pdf

bin/%.pdf: %.tex force
	@mkdir -p $(@D)
	(cd build/$(*D) && yes x | \
		TEXINPUTS=$(dir $(abspath $<)): \
		BIBINPUTS=$(dir $(abspath $<)): \
		latexmk -lualatex -shell-escape -interaction=errorstopmode $(abspath $<))
	cp build/$*.pdf $@

format:
	latexindent $(shell find . -name '*.tex') -l -wd -s -c /tmp

clean:
	$(RM) -r build/* bin
