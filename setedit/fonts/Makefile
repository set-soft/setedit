font_names=boxround cntdown ocr rombios thin

%.exe: %.cc common1.h common2.h
	gcc -o $@ $<

fonts: $(addsuffix .exe,$(font_names))

all: fonts

