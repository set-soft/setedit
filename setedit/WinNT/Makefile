DEST_BIN_DIR = c:\tools
DEST_SET_DIR = c:\sdoc\setedit

ZLIB_DIR = ..\libz
PCRE_DIR = ..\libpcre

# Borland C++
CC       = bcc32
CXX      = $(CC)
INC      = -I. -I.\inc -I..\..\tvision\winnt\include -I..\..\tvision\include \
	-I..\include -I..\setedit\include -I..\settvuti\include -I..\easydiag \
	-I..\infview\include -I..\librhuti -I..\sdg\include -I..\mp3 -I..\extra \
	-I..\calcu -I$(ZLIB_DIR) -I$(PCRE_DIR) -I..\libbzip2
CDEBUGFLAGS = -O2 -6 -T -DNDEBUG
#CDEBUGFLAGS = -6 -v -vi -y -Od
CFLAGS   = -c -q -D_BCPP -a8 -w-aus -WR -WC -X -w-hid -w-par -w-ccc\
	-D__inline__=inline $(CDEBUGFLAGS)
CXXFLAGS = -P -RT- $(CFLAGS)
LFLAGS   = -WR -WC

# Microsoft Visual C++
#CXX      = cl
#INC      = -I.\include\msvc -I.\include -I..\include
#CXXFLAGS = -c -TP -D_MSVC -MDd

CLSDIR   = ..\classes

.SUFFIXES: .cc .c .obj

all: setedit.exe

##--------------------------------------------------------------------------
## SETEDIT
## INFVIEW
## MAINSRC
## SDG
## EXTRA
## MP3
##--------------------------------------------------------------------------
SETEDIT_OBJS_ =  +accehtml.obj +advice.obj +ampdiag.obj +intermp3.obj +mp3list.obj\
+mp3play.obj +ascii.obj +boardmix.obj +bufun.obj +calcu.obj +calendar.obj\
+ceditor.obj +codepage.obj +completi.obj +deedidia.obj +doedidia.obj\
+dskascii.obj +dskcalen.obj +dskclip.obj +dskclose.obj +dskedito.obj +dskhelp.obj\
+dskman.obj +dskmessa.obj +dskmp3.obj +dskwin.obj +dstfile.obj +dumpfile.obj\
+edcolor.obj +edconst.obj +edfonts.obj +editdiag.obj +editmain.obj +editmenu.obj\
+editorfo.obj +editpale.obj +editwind.obj +edkeys.obj +edmsg.obj +edprefs.obj\
+edprint.obj +edprj.obj +edspecs.obj  +dyncat.obj  +mixer.obj\
+mixersb.obj +mixoss.obj +strncpyz.obj +strndup.obj +winoldap.obj +fstrcmp.obj\
+getctxhl.obj +gzfiles.obj +inf.obj +infbase.obj +manview.obj +nhiscoll.obj\
+ninfview.obj +ninfwind.obj +nmanwind.obj +sinf.obj +sman.obj +intgrep.obj\
+keytrans.obj +linelen.obj +loadcle.obj +loaddefl.obj +loadfunc.obj +loadshl.obj\
+loadusew.obj +macrocom.obj +menuload.obj +mli.obj +mliasimp.obj +mlicsdg.obj\
+mliediti.obj +mliedito.obj +mlisdg.obj +msignal.obj +nceditor.obj +ndskwasc.obj\
+ndskwcal.obj +ndskwcli.obj +ndskwclo.obj +ndskwedi.obj +ndskwhel.obj\
+ndskwman.obj +ndskwmes.obj +ndskwmp3.obj +ndskwprj.obj +neditwin.obj\
+nfileedi.obj +nsindica.obj +ntedcol.obj +parser.obj +pathtool.obj +pmacros.obj\
+runprog.obj +sceditor.obj +scresave.obj +sdginter.obj +sdskasci.obj\
+sdskcale.obj +sdskclip.obj +sdskclos.obj +sdskedit.obj +sdskhelp.obj\
+sdskman.obj +sdskmess.obj +sdskmp3.obj +sdskprj.obj +search.obj +sedcolle.obj\
+slpinter.obj +splinman.obj +ssindica.obj +ssyntax.obj +tedcolle.obj +tips.obj\
+tpaltext.obj +tsindica.obj +txhgen.obj +winlistd.obj
SETEDIT_OBJS  = $(SETEDIT_OBJS_:+=setedit\)

{..\setedit}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\setedit\streams}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\streams}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\names}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
   
{..\infview}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\infview\streams}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\infview\names}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<

{..\mainsrc}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\mainsrc}.c{setedit}.obj:
	$(CC) $(CFLAGS) $(INC) -nsetedit $<
{..\calcu}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<
{..\calcu}.c{setedit}.obj:
	$(CC) $(CFLAGS) $(INC) -nsetedit $<

{..\sdg}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<

{.\}.c{setedit}.obj:
	$(CC) $(CFLAGS) -A -DSTDC_HEADERS=1 $(INC) -nsetedit $<

{..\extra}.c{setedit}.obj:
	$(CC) $(CFLAGS) $(INC) -nsetedit $<
{..\extra}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<

{..\mp3}.cc{setedit}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsetedit $<

##--------------------------------------------------------------------------
## LIBRHUTI
##--------------------------------------------------------------------------
LIBRHUTI_OBJS_ =  +abstorel.obj +back2sl.obj +basename.obj +fexpand.obj +io.obj\
+specs.obj +splitfn.obj +stricat.obj +stridown.obj +stridup.obj +strifree.obj
LIBRHUTI_OBJS  = $(LIBRHUTI_OBJS_:+=librhuti\)

{..\librhuti}.cc{librhuti}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nlibrhuti $<

librhuti.lib: $(LIBRHUTI_OBJS)
	del $@
	tlib /E /P32 $@ @&&!
+$(**: = &^
+)
!

##--------------------------------------------------------------------------
## EASYDIAG
##--------------------------------------------------------------------------
EASYDIAG_OBJS_ =  +lablchek.obj +lablradi.obj +snostate.obj +tcheck2.obj\
+tmiclust.obj +tsbutton.obj +tschkarr.obj +tschkbox.obj +tshscrol.obj\
+tshzgrp.obj +tsinplin.obj +tsinplpi.obj +tslabel.obj +tslider.obj +tslistbo.obj\
+tsradbot.obj +tssortlb.obj +tssosslb.obj +tstatext.obj +tstextsc.obj\
+tstsorlb.obj +tsvegrp.obj +tsview.obj +tsviewco.obj +ttextscr.obj
EASYDIAG_OBJS  = $(EASYDIAG_OBJS_:+=easydiag\)

{..\easydiag}.cc{easydiag}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -neasydiag $<

easydiag.lib: $(EASYDIAG_OBJS)
	del $@
	tlib /E /P32 $@ @&&!
+$(**: = &^
+)
!

##--------------------------------------------------------------------------
## SETTVUTI
##--------------------------------------------------------------------------
SETTVUTI_OBJS_ =  +diaghelp.obj +dktclock.obj +fileopen.obj +fiopeaid.obj\
+inputsca.obj +ninputsc.obj +nostatex.obj +nprogbar.obj +nprogres.obj\
+ntnocast.obj +ntnosost.obj +setstack.obj +sprogbar.obj +sprogres.obj\
+stnocast.obj +stnonost.obj +tdiagaid.obj +tdiagrow.obj +tinppipe.obj\
+tnocastc.obj +tnosostr.obj +tprogbar.obj +tprogdia.obj +tprogrdi.obj\
+tprogres.obj +tstrlbox.obj +uisarray.obj +ussarray.obj +viewplus.obj
SETTVUTI_OBJS = $(SETTVUTI_OBJS_:+=settvuti\)

{..\settvuti}.cc{settvuti}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsettvuti $<
{..\settvuti\streams}.cc{settvuti}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsettvuti $<
{..\settvuti\names}.cc{settvuti}.obj:
	$(CXX) $(CXXFLAGS) $(INC) -nsettvuti $<

settvuti.lib: $(SETTVUTI_OBJS)
	del $@
	tlib /E /P32 $@ @&&!
+$(**: = &^
+)
!


libs: $(SETEDIT_OBJS) librhuti.lib easydiag.lib settvuti.lib

#!!! The order of libraries is important
setedit.exe: $(SETEDIT_OBJS) librhuti.lib easydiag.lib settvuti.lib
	$(CC) $(LFLAGS) -esetedit.exe $(SETEDIT_OBJS) cw32.lib librhuti.lib easydiag.lib settvuti.lib ..\..\tvision\winnt\rhtv.lib $(ZLIB_DIR)\zlib.lib $(PCRE_DIR)\pcre.lib ..\libbzip2\bzip2.lib

dirs:
	-mkdir setedit
	-mkdir librhuti
	-mkdir easydiag
	-mkdir settvuti
	
install: setedit.exe doc
	copy setedit.exe $(DEST_BIN_DIR)
	xcopy /S /E ..\cfgfiles\*.* $(DEST_SET_DIR)
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
	-del setedit\*.obj
	-del librhuti\*.obj
	-del easydiag\*.obj
	-del settvuti\*.obj
	-del *.exe
	-del *.obj
	-del *.lib
	-del *.tds
