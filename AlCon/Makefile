BPP=32
LIBS=-lAlCon `allegro-config --libs`
DIRLIB=lib
DIROBJ=obj
EXDIR=examples
INCLUDEDIR=include
SRCDIR=src

all: $(DIRLIB)/libAlCon.a $(EXDIR)/exhello $(EXDIR)/ex2 $(EXDIR)/exmouse $(EXDIR)/exkbd

$(DIRLIB)/libAlCon.a: $(DIROBJ)/AlCon.o
	ar r $@ $<

$(DIROBJ)/AlCon.o: $(SRCDIR)/AlCon.c $(INCLUDEDIR)/AlCon.h
	gcc -o $@ -I$(INCLUDEDIR) -DBPP=$(BPP) -O2 -c $<

$(EXDIR)/exhello: $(EXDIR)/exhello.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $< $(LIBS)

$(EXDIR)/ex2: $(EXDIR)/ex2.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $< $(LIBS)

$(EXDIR)/exmouse: $(EXDIR)/exmouse.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $< $(LIBS)

$(EXDIR)/exkbd: $(EXDIR)/exkbd.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $< $(LIBS)
   
