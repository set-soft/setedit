BPP=32
LIBS=-lAlCon `allegro-config --libs`
#LIBS=-lAlCon `allegro-config --static debug`
DIRLIB=lib
DIROBJ=obj
EXDIR=examples
INCLUDEDIR=include
SRCDIR=src
COMPILE_FLAGS= -O2
#COMPILE_FLAGS= -g
TARGETS=$(DIRLIB)/libAlCon.a $(EXDIR)/exhello $(EXDIR)/ex2 $(EXDIR)/exmouse $(EXDIR)/exkbd

all: $(TARGETS)

$(DIRLIB)/libAlCon.a: $(DIROBJ)/AlCon.o
	ar r $@ $<

$(DIROBJ)/AlCon.o: $(SRCDIR)/AlCon.c $(INCLUDEDIR)/AlCon.h
	gcc -o $@ -I$(INCLUDEDIR) -DBPP=$(BPP) $(COMPILE_FLAGS) -c $<

$(EXDIR)/exhello: $(EXDIR)/exhello.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $(COMPILE_FLAGS) $< $(LIBS)

$(EXDIR)/ex2: $(EXDIR)/ex2.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $(COMPILE_FLAGS) $< $(LIBS)

$(EXDIR)/exmouse: $(EXDIR)/exmouse.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $(COMPILE_FLAGS) $< $(LIBS)

$(EXDIR)/exkbd: $(EXDIR)/exkbd.c $(DIRLIB)/libAlCon.a
	gcc -o $@ -I$(INCLUDEDIR) -L$(DIRLIB) $(COMPILE_FLAGS) $< $(LIBS)

clean:
	$(RM) -fv $(TARGETS) $(DIROBJ)/*.o
   
