:@Rem Copyright (C) 1999-2003 by Salvador E. Tropea (SET),
:@Rem see copyrigh file for details
@
@echo .
@echo This batch file configures the packages using the settings that the
@echo *maintainer* uses for your own needs.
@echo These settings doesn't have to be the same you want.
@echo The most common defaults are obtained by just running configure.bat.
@echo If you want to reconfigure the package first delete configure.cache
@echo .
@pause
@echo off
@del configure.cache
@del Makefile
@set CFLAGS=-O2 -Wall -Werror -gstabs+3
@set CXXFLAGS=-O2 -Wall -Werror -gstabs+3 -fno-exceptions -fno-rtti
@echo "--Xcppflags=-O3 -fomit-frame-pointer -ffast-math -fno-exceptions -fno-rtti" --shipped-intl > perl.parm
@echo --enable-maintainer-mode --libset >> perl.parm
@echo --no-prefix-h %1 %2 %3 %4 %5 %6 %7 %8 %9 >> perl.parm
@perl config.pl @perl.parm
@del perl.parm

