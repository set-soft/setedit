This directory contains the DOS installer program.

  The file Copyright 1999-2001 by Salvador Eduardo Tropea and is part of the
setedit so is distributed under the same terms.

  The installer isn't very easy to create and isn't basically needed for
people compiling from sources.

  To compile it you'll need some extra tools:

* upx: executable compressor.
  http://cdata.tvnet.hu/~ml/upx.html
* Allegro 3.1, if you use other version you could need to remove the
datfile.c from the project and use gpr2mak to generate the makefile.
  http://www.talula.demon.co.uk/allegro/
* dat: dat files packager from Allegro.
* exedat: to attach dat file to executables (also from Allegro).
* pmodstub: PMODE/DJ stub.
  ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2misc/
* cwsdpmi: DPMI Host for DOS.
  ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2misc/
* Perl 5.x
  ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2gnu/
* libwin 0.1.2 by Richard Dawe and RegDos Group:
  http://www.bigfoot.com/~richdawe/

  To generate the installer run "make installer" in the makes directory.


