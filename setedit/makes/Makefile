srcdir=.
VPATH=$(srcdir)

ifneq ($(strip $(DJDIR)),)
export editor_OS:=DJGPP
endif

# If not DOS then just Linux, no other supported by now
ifeq ($(strip $(editor_OS)),)
export editor_OS:=Linux
endif

editorprjs=amp3 easydiag editor extra inffd librhuti libset sdgcline \
				settv testeasy install infview

allprjs=$(editorprjs) ../mp3/libamp/libamp ../mp3/mpegsound/mpegsnd

%.mak: %.gpr
	gpr2mak $<

editor: makes
	$(MAKE) -f editor.mak

makes: $(addsuffix .mak,$(allprjs))

# That's a problematic rule
#$(addsuffix .mak,$(editorprjs)): rhide.env

force-patch: makes
	perl patchenv.pl

clean:
	rm -f *.mak *.bak *.bkp

clean-o:
	rm -f obj*/*.o

clean-docs:
	cd ../doc; make clean-docs; cd ../makes

infview: makes
	$(MAKE) -f infview.mak

ifeq ($(editor_OS),DJGPP)
scrnsave:
	$(MAKE) -C ../scrnsave

distrib-all: distrib distrib-infview installer

distrib: editor scrnsave
	cd djgpp; compress.bat $(EXTRA_INS_OPS) ; cd ..

distrib-infview: infview
	cd djgpp; perl compinf.pl $(EXTRA_INS_OPS) ; cd ..

installer: distrib
	$(MAKE) -f install.mak
	cd ../install; perl create.pl; cd ..

#
# Installation prefix: MPREFIX, mprefix or DJDIR
#
ifneq ($(strip $(MPREFIX)),)
inst_prefix=$(MPREFIX)
else
 ifneq ($(strip $(mprefix)),)
 inst_prefix=$(mprefix)
 else
 inst_prefix=$(DJDIR)
 endif
endif

install: editor
	cd djgpp; compress.bat $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install; cd ..

install-infview: infview
	cd djgpp; perl compinf.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install; cd ..

else
#
# Installation prefix: MPREFIX, mprefix or /usr
#
ifneq ($(strip $(MPREFIX)),)
inst_prefix=$(MPREFIX)
else
 ifneq ($(strip $(mprefix)),)
 inst_prefix=$(mprefix)
 else
 inst_prefix=/usr
 endif
endif

distrib: editor
	cd linux; perl ./compress.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --fhs $(SET_USE_FHS); cd ..
   
distrib-infview: infview
	cd linux; perl ./compinf.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --fhs $(SET_USE_FHS); cd ..

install: editor
	cd linux; perl ./compress.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install --fhs $(SET_USE_FHS); cd ..

install-infview: infview
	cd linux; perl ./compinf.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install --fhs $(SET_USE_FHS); cd ..
endif
