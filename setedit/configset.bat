:@Rem Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
:@Rem see copyrigh file for details
@
@Echo off
@del configure.cache
@set CFLAGS=-O2 -Wall -Werror -gstabs+3
@set CXXFLAGS=-O2 -Wall -Werror -gstabs+3 -fno-exceptions -fno-rtti
@configure.bat "--Xcppflags=-O3 -fomit-frame-pointer -ffast-math -fno-exceptions -fno-rtti" --shipped-intl
