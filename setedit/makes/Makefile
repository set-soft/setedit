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

# Enable the .imk and .umk maintainance only in "maintainer mode"
ifdef MAINTAINER_MODE
MAKES_TARGET=makes
else
MAKES_TARGET=
endif

# That's the default target
editor: $(MAKES_TARGET)
	$(MAKE) -f editor.mkf

###########################################################################
#                                                                         #
#            Rules to generate makefiles from RHIDE's projects            #
#                                                                         #
###########################################################################

main_targets=editor infview libset install sdgcline testeasy
editorprjs=$(main_targets) amp3 easydiag extra inffd librhuti settv
mp3_targets=../mp3/libamp/libamp ../mp3/mpegsound/mpegsnd
allprjs=$(editorprjs) $(mp3_targets)

# The list of "targets"
mkfs_list=$(main_targets) $(mp3_targets)
# Makefiles for them
mkfs_files=$(addsuffix .mkf,$(mkfs_list))
# Dependencies for them
imks_files=$(addsuffix .imk,$(mkfs_list))

# Tool to extract dependencies
extrimk.exe: extrimk.cc
	$(RHIDE_GCC) -o $@ $< $(STDCPP_LIB)

# Tool to create temporal .mak files and call  extrimk
genimk.exe: genimk.cc extrimk.exe
	$(RHIDE_GCC) -o $@ $< $(STDCPP_LIB)

# Main project, the editor
editor.imk: editor.gpr amp3.gpr easydiag.gpr extra.gpr inffd.gpr librhuti.gpr \
	settv.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# InfView project
infview.imk: infview.gpr easydiag.gpr librhuti.gpr settv.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# LibSET for RHIDE
libset.imk: libset.gpr easydiag.gpr extra.gpr settv.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# DOS Installer
install.imk: install.gpr easydiag.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# Command line version of sdg, not fully maintained
sdgcline.imk: sdgcline.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# EasyDiag example/test, not fully maintained
testeasy.imk: testeasy.gpr easydiag.gpr settv.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# LibAMP
../mp3/libamp/libamp.imk: ../mp3/libamp/libamp.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

# LibMPEGSound (splay)
../mp3/mpegsound/mpegsnd.imk: ../mp3/mpegsound/mpegsnd.gpr genimk.cc extrimk.cc
	@./genimk.exe $@ $^

makes: extrimk.exe genimk.exe $(mkfs_files) $(imks_files)

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

infview: $(MAKES_TARGET)
	$(MAKE) -f infview.mkf

libset: $(MAKES_TARGET)
	$(MAKE) -f libset.mkf

ifeq ($(editor_OS),DJGPP)
scrnsave:
	$(MAKE) -C ../scrnsave

distrib-all: distrib distrib-infview installer

distrib: editor scrnsave
	cd djgpp; compress.bat $(EXTRA_INS_OPS) ; cd ..

distrib-infview: infview
	cd djgpp; perl compinf.pl $(EXTRA_INS_OPS) ; cd ..

distrib-source:
	cd djgpp; compress.bat --only-source $(EXTRA_INS_OPS) ; cd ..

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

distrib-source:
	cd linux; perl ./compress.pl --only-source $(EXTRA_INS_OPS) ; cd ..

install: editor
	cd linux; perl ./compress.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install --fhs $(SET_USE_FHS); cd ..

install-infview: infview
	cd linux; perl ./compinf.pl $(EXTRA_INS_OPS) --prefix $(inst_prefix) --install --fhs $(SET_USE_FHS); cd ..
endif
