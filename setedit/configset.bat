:@Rem Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
:@Rem see copyrigh file for details
@
@Echo off
@del configure.cache
@del Makefile
@set CFLAGS=-O2 -Wall -Werror -gstabs+3
@set CXXFLAGS=-O2 -Wall -Werror -gstabs+3 -fno-exceptions -fno-rtti
@echo "--Xcppflags=-O3 -fomit-frame-pointer -ffast-math -fno-exceptions -fno-rtti" --shipped-intl > perl.parm
@echo --no-prefix-h %1 %2 %3 %4 %5 %6 %7 %8 %9 >> perl.parm
@perl config.pl @perl.parm
@del perl.parm

