srcdir=.
VPATH=$(srcdir)

.PHONY: editor

include rhide.env

ifneq ($(strip $(DJDIR)),)
export editor_OS:=DJGPP
endif

# If not DOS then just Linux, no other supported by now
ifeq ($(strip $(editor_OS)),)
export editor_OS:=Linux
endif

# That's the default target
editor: makes
	$(MAKE) -f editor.mkf

###########################################################################
#                                                                         #
#            Rules to generate makefiles from RHIDE's projects            #
#                                                                         #
###########################################################################

editorprjs=amp3 easydiag editor extra inffd librhuti libset sdgcline \
				settv testeasy install infview

allprjs=$(editorprjs) ../mp3/libamp/libamp ../mp3/mpegsound/mpegsnd

# The list of "targets"
mkfs_list=editor infview install libset sdgcline testeasy \
	../mp3/libamp/libamp ../mp3/mpegsound/mpegsnd
# Makefiles for them
mkfs_files=$(addsuffix .mkf,$(mkfs_list))
# Dependencies for them
imks_files=$(addsuffix .imk,$(allprjs))

# Tool to extract dependencies
extrimk.exe: extrimk.cc
	$(RHIDE_GCC) -o $@ $<

# Tool to create temporal .mak files and call  extrimk
genimk.exe: genimk.cc extrimk.exe
	$(RHIDE_GCC) -o $@ $<

# Main project, the editor
editor.imk: editor.gpr amp3.gpr easydiag.gpr extra.gpr inffd.gpr librhuti.gpr \
	settv.gpr genimk.exe
	@./genimk.exe $@ $^

# InfView project
infview.imk: infview.gpr easydiag.gpr librhuti.gpr settv.gpr genimk.exe
	@./genimk.exe $@ $^

# LibSET for RHIDE
libset.imk: libset.gpr easydiag.gpr extra.gpr settv.gpr genimk.exe
	@./genimk.exe $@ $^

# DOS Installer
install.imk: install.gpr easydiag.gpr genimk.exe
	@./genimk.exe $@ $^

# Command line version of sdg, not fully maintained
sdgcline.imk: sdgcline.gpr genimk.exe
	@./genimk.exe $@ $^

# EasyDiag example/test, not fully maintained
testeasy.imk: testeasy.gpr easydiag.gpr settv.gpr genimk.exe
	@./genimk.exe $@ $^

# LibAMP
../mp3/libamp/libamp.imk: ../mp3/libamp/libamp.gpr genimk.exe
	@./genimk.exe $@ $^

# LibMPEGSound (splay)
../mp3/mpegsound/mpegsnd.imk: ../mp3/mpegsound/mpegsnd.gpr genimk.exe
	@./genimk.exe $@ $^

makes: $(mkfs_files) $(imks_files)

###########################################################################
#                                                                         #
#            Clean targets                                                #
#                                                                         #
###########################################################################

clean:
	rm -f *.imk *.bak *.bkp

clean-o:
	rm -f obj*/*.o

clean-docs:
	cd ../doc; make clean-docs; cd ../makes

infview: makes
	$(MAKE) -f infview.mkf

libset: makes
	$(MAKE) -f libset.mkf

ifeq ($(editor_OS),DJGPP)
scrnsave:
	$(MAKE) -C ../scrnsave

distrib-all: distrib distrib-infview installer

distrib: editor scrnsave
	cd djgpp; compress.bat $(EXTRA_INS_OPS) ; cd ..

distrib-infview: infview
	cd djgpp; perl compinf.pl $(EXTRA_INS_OPS) ; cd ..

installer: distrib
	$(MAKE) -f install.mkf
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
