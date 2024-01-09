.PHONY: all format clean force

all: bin/intro.pdf bin/design.pdf

bin/design.pdf: bin/%.pdf: %.tex force
	@mkdir -p $(@D)
	latexrun -O build -o $@ $<

bin/intro.pdf: bin/%.pdf: %.md force
	@mkdir -p $(@D)
	pandoc -o $@ $<

format:
	latexindent $(shell find . -name '*.tex') -l -wd -s -c /tmp

clean:
	$(RM) -r build bin
