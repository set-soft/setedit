font_names=boxround cntdown ocr rombios thin

%.exe: %.cc
	gcc -o $@ $<

fonts: $(addsuffix .exe,$(font_names))

all: fonts

