.PHONY: all format clean force

all: bin/slides.pdf

bin/%.pdf: %.tex force
	@mkdir -p $(@D)
	latexrun -O build -o $@ $<

format:
	latexindent $(shell find . -name '*.tex') -l -wd -s -c /tmp

clean:
	$(RM) -r build bin
