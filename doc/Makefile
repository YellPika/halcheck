.PHONY: all format clean force

all: bin/slides.pdf

bin/%.pdf: %.tex force
	@mkdir -p $(@D)
	latexmk -pdf -shell-escape -interaction=nonstopmode -output-directory=build/$(*D) $<
	cp build/$*.pdf $@

format:
	latexindent $(shell find . -name '*.tex') -l -wd -s -c /tmp

clean:
	$(RM) -r build/* bin
