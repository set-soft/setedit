#
#  This makefile is deprecated, you should use it only if the "new" mechanism
#  fails. Please read the INSTALL.txt instructions.
#  If you must use this file please report it.
#
DEST_BIN_DIR = c:\setedit
DEST_SET_DIR = c:\setedit

TV_LIB   = ..\..\tvision\winnt\rhtv.lib



all: setedit.exe

# Borland C++

CC       = bcc32
LD       = bcc32
CXX      = $(CC)
INC      = -I.\include -I..\include

!ifdef DEBUG
CDEBUGFLAGS = -v -vi -y -6 -Od
!else
CDEBUGFLAGS = -O2 -6 -T -DNDEBUG
!endif

!ifdef DYNRTL
DYNRTLDFLAGS = -WR
!endif

CFLAGS   = -D_BCPP -w-aus -w-ccc -a8 $(DYNRTLDFLAGS) -WC -X -q $(CDEBUGFLAGS) \
	-D__inline__=inline -DFOR_EDITOR
# Note: rch == Unreachable code. BC++ 5.5 is idiot to detect code that is
#              disabled on purpose. i.e. if (0) ...
CXXFLAGS = -P -RT- -w-hid -w-par -w-rch $(CFLAGS)
LDFLAGS  = $(DYNRTLDFLAGS) -WC

INC      = -I. -I.\inc -I..\..\tvision\winnt\include -I..\..\tvision\include \
	-I..\include -I..\setedit\include -I..\settvuti\include -I..\easydiag \
	-I..\infview\include -I..\librhuti -I..\sdg\include -I..\mp3 -I..\extra \
	-I..\calcu -I..\libz -I..\libpcre -I..\libbzip2 -I..\holidays

{..\setedit}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\setedit\streams}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\streams}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\names}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\librhuti}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\easydiag}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\infview}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\infview\streams}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\infview\names}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\calcu}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\settvuti}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\settvuti\streams}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\settvuti\names}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\sdg}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\extra}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\mp3}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<
{..\mainsrc}.cc{objs}.obj:
	$(CXX) -c $(CXXFLAGS) $(INC) -nobjs $<

{..\mainsrc}.c{objs}.obj:
	$(CC) -c $(CFLAGS) $(INC) -nobjs $<
{..\calcu}.c{objs}.obj:
	$(CC) -c $(CFLAGS) $(INC) -nobjs $<
{..\extra}.c{objs}.obj:
	$(CC) -c $(CFLAGS) $(INC) -nobjs $<
	
objs\edspecs.obj: ..\mainsrc\edspecs.cc
objs\bufun.obj: ..\mainsrc\bufun.cc

EDITOR_OBJS_ =  +accehtml.obj +advice.obj +ampdiag.obj +intermp3.obj +mp3list.obj +mp3play.obj\
	+ascii.obj +boardmix.obj +bufun.obj +calcu.obj +calendar.obj +ceditor.obj +codepage.obj\
	+completi.obj +debug.obj +deedidia.obj +doedidia.obj +dskascii.obj +dskcalen.obj\
	+dskclip.obj +dskclose.obj +dskedito.obj +dskhelp.obj +dskman.obj +dskmessa.obj\
	+dskmp3.obj +dskwin.obj +dstfile.obj +dumpfile.obj +edcolor.obj +edconst.obj +edfonts.obj\
	+editdiag.obj +editmain.obj +editmenu.obj +editorfo.obj +editpale.obj +editwind.obj\
	+edkeys.obj +edmsg.obj +edprefs.obj +edprint.obj +edprj.obj +edspecs.obj \
	+dyncat.obj  +mixer.obj   +stackdbg.obj +strncpyz.obj\
	+strndup.obj +fstrcmp.obj +getctxhl.obj +gzfiles.obj +holidays.obj +inf.obj +infbase.obj\
	+manview.obj +nhiscoll.obj +ninfview.obj +ninfwind.obj +nmanwind.obj +sinf.obj +sman.obj\
	+intgrep.obj +keytrans.obj +linelen.obj +loadcle.obj +loaddefl.obj +loadfunc.obj\
	+loadnobkp.obj +loadshl.obj +loadusew.obj +macrocom.obj +menuload.obj +mli.obj\
	+mliasimp.obj +mlicsdg.obj +mliediti.obj +mliedito.obj +mlisdg.obj +msignal.obj\
	+nceditor.obj +ndskwasc.obj +ndskwcal.obj +ndskwcli.obj +ndskwclo.obj +ndskwedi.obj\
	+ndskwhel.obj +ndskwman.obj +ndskwmes.obj +ndskwmp3.obj +ndskwprj.obj +neditwin.obj\
	+nfileedi.obj +nsindica.obj +ntedcol.obj +parser.obj +pathlist.obj +pathtool.obj\
	+pclipper.obj +pmacros.obj +pperl.obj +pphp.obj +pvarious.obj +runprog.obj +sceditor.obj\
	+scresave.obj +sdginter.obj +sdskasci.obj +sdskcale.obj +sdskclip.obj +sdskclos.obj\
	+sdskedit.obj +sdskhelp.obj +sdskman.obj +sdskmess.obj +sdskmp3.obj +sdskprj.obj\
	+search.obj +sedcolle.obj +slpinter.obj +splinman.obj +ssindica.obj +ssyntax.obj +tags.obj\
	+tedcolle.obj +tips.obj +tpaltext.obj +tsindica.obj +txhgen.obj +winlistd.obj \
	+lablchek.obj +lablradi.obj +snostate.obj +tcheck2.obj +tmiclust.obj +tsbutton.obj\
	+tschkarr.obj +tschkbox.obj +tshscrol.obj +tshzgrp.obj +tsinplin.obj +tsinplpi.obj\
	+tslabel.obj +tslider.obj +tslistbo.obj +tsradbot.obj +tssortlb.obj +tssosslb.obj\
	+tstatext.obj +tstextsc.obj +tstsorlb.obj +tsvegrp.obj +tsview.obj +tsviewco.obj\
	+ttextscr.obj  +diaghelp.obj +dktclock.obj +fileopen.obj +fiopeaid.obj +inputsca.obj\
	+ninputsc.obj +nostatex.obj +nprogbar.obj +nprogres.obj +ntnocast.obj +ntnosost.obj\
	+setstack.obj +sprogbar.obj +sprogres.obj +stnocast.obj +stnonost.obj +tdiagaid.obj\
	+tdiagrow.obj +tinppipe.obj +tnocastc.obj +tnosostr.obj +tprogbar.obj +tprogdia.obj\
	+tprogrdi.obj +tprogres.obj +tstrlbox.obj +uisarray.obj +ussarray.obj +viewplus.obj \
	+abstorel.obj +back2sl.obj +basename.obj +fexpand.obj +io.obj +specs.obj +splitfn.obj\
	+stricat.obj +stridown.obj +stridup.obj +strifree.obj
EDITOR_OBJS = $(EDITOR_OBJS_:+=objs\)

EXTRA_LIBS = ..\libz\zlib.lib ..\libpcre\pcre.lib ..\libbzip2\bzip2.lib

EXTRALIB_MAKE_ARGS = _SUBMAKE=1 CC="$(CC)" LD="$(LD)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -f Makefile.bcc

libs: $(EXTRA_LIBS)

..\libz\zlib.lib:
	cd ..\libz
	$(MAKE) $(EXTRALIB_MAKE_ARGS)
	cd ..\winnt
	
..\libpcre\pcre.lib:
	cd ..\libpcre
	$(MAKE) $(EXTRALIB_MAKE_ARGS)
	cd ..\winnt

..\libbzip2\bzip2.lib:
	cd ..\libbzip2
	$(MAKE) $(EXTRALIB_MAKE_ARGS)
	cd ..\winnt
	
clean-zlib:
	cd ..\libz
	$(MAKE) $(EXTRALIB_MAKE_ARGS) clean
	cd ..\winnt
	
clean-pcre:
	cd ..\libpcre
	$(MAKE) $(EXTRALIB_MAKE_ARGS) clean
	cd ..\winnt
	
clean-bzip2:
	cd ..\libbzip2
	$(MAKE) $(EXTRALIB_MAKE_ARGS) clean
	cd ..\winnt
	
clean-libs: clean-zlib clean-pcre clean-bzip2

setedit.exe: $(EXTRA_LIBS) $(EDITOR_OBJS) 
	$(LD) $(LDFLAGS) -esetedit $(EDITOR_OBJS) $(EXTRA_LIBS) $(TV_LIB)

install: setedit.exe doc
	echo dummy >a.exe
	xcopy /I *.exe $(DEST_BIN_DIR)
	-del a.exe
	-del $(DEST_BIN_DIR)\a.exe
	xcopy /I /S /E ..\cfgfiles\*.* $(DEST_SET_DIR)
	cd ..\doc
	make -f Makefile.bcc DEST_SET_DIR=$(DEST_SET_DIR) install
	cd ..\winnt

doc:
	cd ..\doc
	make -f Makefile.bcc
	cd ..\winnt

clean-doc:
	cd ..\doc
	make -f Makefile.bcc clean
	cd ..\winnt

clean:
	-del objs\*.obj
	-del *.exe
	-del *.obj
	-del *.lib
	-del *.tds
	
clean-all: clean clean-libs clean-doc
