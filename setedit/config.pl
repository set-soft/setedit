#!/usr/bin/perl
# Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
# see copyrigh file for details
#

require "miscperl.pl";
require "conflib.pl";

$conf{'infview'}='yes';
$conf{'HAVE_BZIP2'}='yes';
$conf{'parser'}='parserbr.c';
$conf{'mp3lib'}='mpegsnd';
$conf{'mp3'}='yes';
$conf{'HAVE_MIXER'}='yes';
$conf{'intlShipped'}='no';
$TVCommandLine=0;

GetCache();
GetVersion('');

$TVVersionNeeded='1.1.0';
$ZLibVersionNeeded='1.1.2';
$DJGPPVersionNeeded='2.0.2';
# Allegro 3.1==3.0.1 3.11==3.0.11 3.12==3.0.12
$AllegroVersionNeeded='3.0.1';
# 3.9.x are WIPs
$AllegroNotNeeded='3.9.0';
unlink $ErrorLog;

SeeCommandLine();

print "Configuring SETEdit v$Version\n\n";
# Determine the OS
$OS=DetectOS();

if (($OS ne 'DOS') && (@conf{'mp3'} eq 'yes') && (@conf{'mp3lib'} ne 'mpegsnd'))
  {
   die "Only libmpegsnd is available for UNIX\n"
  }
if ($OS eq 'Win32')
  {
   $conf{'mp3'}='no';
   $conf{'HAVE_MIXER'}='no';
   $conf{'mp3lib'}='';
  }

$supportDir='makes/'.$supportDir;
LookForPrefix();
# Determine C flags
$CFLAGS=FindCFLAGS();
FindXCFLAGS();
# Determine C++ flags
$CXXFLAGS=FindCXXFLAGS();
FindXCXXFLAGS();
# Test for a working gcc
$GCC=CheckGCC();
# Check if gcc can compile C++
$GXX=CheckGXX();

# Is the right djgpp?
if ($OS eq 'DOS')
  {
   LookForDJGPP($DJGPPVersionNeeded);
   LookForAllegro($AllegroVersionNeeded,$AllegroNotNeeded);
  }

# Where is the TV library?
# $TVInclude and $TVLib
LookForTV();
# Is the right version?
TestTVVersion($TVVersionNeeded);

# Have libc international support? what about libintl or libiconv?
LookForIntlSupport();
# Is PCRE available?
LookForPCRE();
# Is ZLib available?
LookForZLib($ZLibVersionNeeded);
# Look for recode and version
LookForRecode();
# Look for xgettext
LookForGettextTools();
LookForMakeinfo();

print "\n";
#
# Create a list of RHIDE variables to configure the projects
#
$TVLib='../'.$TVLib if (substr($TVLib,0,2) eq '..');
$TVInclude='../'.$TVInclude if (substr($TVInclude,0,2) eq '..');
if ($OS eq 'DOS')
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=$(DJDIR)/include $(DJDIR)/lang/cxx $(DJDIR)/lib/gcc-lib';
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS=';
   $MakeDefsRHIDE[1].='intl ' unless ($OS ne 'DOS') && (@conf{'intl'} eq 'no');
   $MakeDefsRHIDE[1].='iconv ' if (@conf{'iconv'} eq 'yes');
   if ((@conf{'mp3'} eq 'yes') && (@conf{'HAVE_ALLEGRO'} eq 'yes'))
     {
      $MakeDefsRHIDE[1].=@conf{'mp3lib'}.' alleg ';
     }
   $MakeDefsRHIDE[1].='bz2 ' if (@conf{'HAVE_BZIP2'} eq 'yes');
  }
elsif ($OS eq 'UNIX')
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=/usr/include /usr/local/include /usr/include/g++ /usr/local/include/g++ /usr/lib/gcc-lib /usr/local/lib/gcc-lib';
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS=ncurses gpm m';
   $MakeDefsRHIDE[1].=' bz2' if (@conf{'HAVE_BZIP2'} eq 'yes');
   $MakeDefsRHIDE[1].=' '.@conf{'mp3lib'} if (@conf{'mp3'} eq 'yes');
  }
else # Win32
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=';
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS=stdc++';
   $MakeDefsRHIDE[1].=' bz2' if (@conf{'HAVE_BZIP2'} eq 'yes');
   $MakeDefsRHIDE[1].=' '.@conf{'mp3lib'} if (@conf{'mp3'} eq 'yes');
  }
$MakeDefsRHIDE[2]="RHIDE_OS_LIBS_PATH=$TVLib";
$MakeDefsRHIDE[2].=' ../libz' if (@conf{'zlibShipped'} eq 'yes');
$MakeDefsRHIDE[2].=' ../libbzip2' if (@conf{'HAVE_BZIP2'} eq 'yes');
$MakeDefsRHIDE[2].=' ../libpcre' if (@conf{'PCREShipped'} eq 'yes');
$MakeDefsRHIDE[2].=' ../gettext' if (@conf{'intlShipped'} eq 'yes');
$MakeDefsRHIDE[3]="TVISION_INC=$TVInclude";
$test='';
$test.=' ../libz' if (@conf{'zlibShipped'} eq 'yes');
$test.=' ../libbzip2' if (@conf{'HAVE_BZIP2'} eq 'yes');
$test.=' ../libpcre' if (@conf{'PCREShipped'} eq 'yes');
$test.=' ../gettext' if (@conf{'intlShipped'} eq 'yes');
$MakeDefsRHIDE[4]='SUPPORT_INC='.$test;
# The support libraries shouldn't generate dependencies
$MakeDefsRHIDE[0].=$test;
if (@conf{'static'} eq 'yes')
  {
   $MakeDefsRHIDE[5]='RHIDE_COMPILE_LINK=$(RHIDE_LD) $(RHIDE_LIBDIRS) $(LDFLAGS) -static $(RHIDE_LDFLAGS) $(C_EXTRA_FLAGS) -o $(OUTFILE)  $(OBJFILES) $(LIBRARIES) $(RHIDE_LIBS)';
  }
else
  {
   $MakeDefsRHIDE[5]='RHIDE_COMPILE_LINK=$(RHIDE_LD) $(RHIDE_LIBDIRS) $(LDFLAGS) $(RHIDE_LDFLAGS) $(C_EXTRA_FLAGS) -o $(OUTFILE)  $(OBJFILES) $(LIBRARIES) $(RHIDE_LIBS)';
  }
# Take out the CFLAGS and CPPFLAGS variables
$MakeDefsRHIDE[6]='RHIDE_COMPILE_C=$(RHIDE_GCC) $(RHIDE_INCLUDES) $(C_DEBUG_FLAGS) $(C_OPT_FLAGS)  $(C_WARN_FLAGS) $(C_C_LANG_FLAGS) $(C_EXTRA_FLAGS) $(LOCAL_OPT) $(RHIDE_OS_CFLAGS) -c $(SOURCE_NAME) -o $(OUTFILE)';
$MakeDefsRHIDE[7]='RHIDE_COMPILE_CC=$(RHIDE_GXX) $(RHIDE_INCLUDES) $(C_DEBUG_FLAGS) $(C_OPT_FLAGS)  $(C_WARN_FLAGS) $(C_C_LANG_FLAGS) $(C_CXX_LANG_FLAGS) $(C_EXTRA_FLAGS) $(RHIDE_OS_CXXFLAGS) $(LOCAL_OPT) -c $(SOURCE_NAME) -o $(OUTFILE)';
if ($OSflavor eq 'Mingw')
  {
   $MakeDefsRHIDE[8]='SPECIAL_LDFLAGS=-mconsole';
  }
CreateRHIDEenvs('makes/rhide.env','+mp3/libamp/rhide.env',
                '+mp3/mpegsound/rhide.env');
#
# Now pass the options in rhide.env to the makefiles
#
print "Configuring .mak files\n";
chdir('makes');
#system("perl patchenv.pl");
`perl patchenv.pl`;
chdir('..');
#
# Translate some options into variables suitable for defines and also
# compute some interdependencies.
#
if ($OS eq 'DOS')
  {
   $conf{'WITH_MP3'}=((@conf{'mp3'} eq 'yes') &&
     (@conf{'HAVE_ALLEGRO'} eq 'yes')) ? 'yes' : 'no';
  }
else
  {
   $conf{'WITH_MP3'}=((@conf{'mp3'} eq 'yes') &&
     (@conf{'mp3lib'} eq 'mpegsnd')) ? 'yes' : 'no';
  }
$conf{'HAVE_AMP'}=((@conf{'WITH_MP3'} eq 'yes') && (@conf{'mp3lib'} eq 'amp'))
                  ? 'yes' : 'no';
$conf{'HAVE_MPEGSOUND'}=((@conf{'WITH_MP3'} eq 'yes') && (@conf{'mp3lib'} eq 'mpegsnd'))
                  ? 'yes' : 'no';
CreateConfigH();
GenerateMakefile();
$ReplaceTags{'TVInclude'}=$TVInclude;
$ReplaceTags{'recode'}=$conf{'recode'} eq 'no' ? '@echo' : 'recode';
$ReplaceTags{'recode_sep'}=$conf{'recode_sep'};
$ReplaceTags{'copy_recode'}='perl utod.pl'; #($OS eq 'UNIX') ? 'perl utod.pl' : 'cp';
ReplaceText('doc/gnumake.in','doc/Makefile');
ReplaceText('internac/gnumake.in','internac/Makefile');
# Shipped gettext:
$ReplaceTags{'datadir'}=$conf{'prefix'}.'/share';
$ReplaceTags{'libdir'}=$conf{'prefix'}.'/lib';
$ReplaceTags{'CC'}=$GCC;
$ReplaceTags{'CFLAGS'}=$conf{'CFLAGS'};
ReplaceText('gettext/Makefile.in','gettext/Makefile');
`cp gettext/djgpp.h gettext/config.h` if $OS eq 'DOS';

#
# BC++ Makefile
#
# ** Main sources
#$column=19;
#$a=ExtractItemsMak('makes/editor.mak',$column,1);
## Eliminate assembler modules
#$a=~s/(\w+)\.s//g;
## To obj format
#ToBCCObjs($a);
#$ReplaceTags{'SETEDIT_OBJS_BCC'}=$a;
## ** Easydiag library
#$column=19;
#$a=ExtractItemsMak('makes/easydiag.mak',$column);
#ToBCCObjs($a);
#$ReplaceTags{'EASYDIAG_OBJS_BCC'}=$a;
## ** Librhuti library
#$column=19;
#$a=ExtractItemsMak('makes/librhuti.mak',$column);
#ToBCCObjs($a);
#$ReplaceTags{'LIBRHUTI_OBJS_BCC'}=$a;
## ** SETTVUti
#$column=19;
#$a=ExtractItemsMak('makes/settv.mak',$column);
#ToBCCObjs($a);
#$ReplaceTags{'SETTVUTI_OBJS_BCC'}=$a;
#$ReplaceTags{'STATIC_LIB_BCC'}=@conf{'static'} eq 'yes' ? 'cw32.lib' : '';
#ReplaceText('WinNT/bccmake.in','WinNT/Makefile');
`perl confignt.pl`;

#
# Select the desired parser for the calculator
#
system('cp -p calcu/'.@conf{'parser'}.' calcu/parser.c');

print "\nSuccessful configuration!\n\n";

GiveAdvice();
# Avoid caching it
$conf{'force-intlShipped'}='no';
CreateCache();
unlink $ErrorLog;


sub SeeCommandLine
{
 my $i;

 foreach $i (@ARGV)
   {
    if ($i eq '--help')
      {
       ShowHelp();
       die "\n";
      }
    elsif ($i=~'--prefix=(.*)')
      {
       $conf{'prefix'}=$1;
      }
    elsif ($i eq '--static')
      {
       $conf{'static'}='yes';
      }
    elsif ($i eq '--dynamic')
      {
       $conf{'static'}='no';
      }
    elsif ($i eq '--fhs')
      {
       $conf{'fhs'}='yes';
      }
    elsif ($i eq '--no-fhs')
      {
       $conf{'fhs'}='no';
      }
    elsif ($i eq '--libset')
      {
       $conf{'libset'}='yes';
      }
    elsif ($i eq '--no-libset')
      {
       $conf{'libset'}='no';
      }
    elsif ($i eq '--infview')
      {
       $conf{'infview'}='yes';
      }
    elsif ($i eq '--no-infview')
      {
       $conf{'infview'}='no';
      }
    elsif ($i eq '--no-bzip2')
      {
       $conf{'HAVE_BZIP2'}='no';
      }
    elsif ($i eq '--bzip2')
      {
       $conf{'HAVE_BZIP2'}='yes';
      }
    elsif ($i eq '--parser-adv')
      {
       $conf{'parser'}='parserbr.c';
      }
    elsif ($i eq '--parser-brs')
      {
       $conf{'parser'}='small.c';
      }
    elsif ($i eq '--parser-ml')
      {
       $conf{'parser'}='parserml.c';
      }
    elsif ($i eq '--with-amp')
      {
       $conf{'mp3lib'}='amp';
      }
    elsif ($i eq '--with-mpegsnd')
      {
       $conf{'mp3lib'}='mpegsnd';
      }
    elsif ($i eq '--without-mp3')
      {
       $conf{'mp3'}='no';
      }
    elsif ($i=~'--Xcflags=(.*)')
      {
       $conf{'XCFLAGS'}=$1;
      }
    elsif ($i=~'--Xcppflags=(.*)')
      {
       $conf{'XCXXFLAGS'}=$1;
      }
    elsif ($i eq '--with-mixer')
      {
       $conf{'HAVE_MIXER'}='yes';
      }
    elsif ($i eq '--without-mixer')
      {
       $conf{'HAVE_MIXER'}='no';
      }
    elsif ($i eq '--shipped-intl')
      {
       $conf{'force-intlShipped'}='yes';
      }
    elsif ($i=~'--tv-include=(.*)')
      {
       $conf{'TV_INCLUDE'}=$1;
       $TVCommandLine=1;
      }
    elsif ($i=~'--tv-lib=(.*)')
      {
       $conf{'TV_LIB'}=$1;
       $TVCommandLine=1;
      }
    else
      {
       ShowHelp();
       die "Unknown option: $i\n";
      }
   }
}

sub ShowHelp
{
 print "Available options:\n\n";
 print "--help         : displays this text.\n";
 print "--prefix=path  : defines the base directory for installation.\n";
 print "--static       : force to create an statically linked executable.\n";
 print "--dynamic      : generated a dynamically linked executable [default].\n";
 print "--fhs          : force the FHS layout under UNIX.\n";
 print "--no-fhs       : force to not use the FHS layout under UNIX.\n";
 print "--libset       : create libset, needed to compile RHIDE.\n";
 print "--no-libset    : don't create libset [default].\n";
 print "--infview      : also compile InfView [default].\n";
 print "--no-infview   : don't compile InfView, just the editor.\n";
 print "--no-bzip2     : don't include support for bzip2 files.\n";
 print "--bzip2        : include support for bzip2 (in case you disabled it).\n";
 print "--parser-adv   : use the advanced parser for calc. [default]\n";
 print "--parser-brs   : use the small Burton's parser for calculator\n";
 print "--parser-ml    : use the small ML's parser for calculator\n";
 print "--with-amp     : use libamp for MP3 support [DOS only]\n";
 print "--with-mpegsnd : use libmpegsnd for MP3 support [default]\n";
 print "--without-mp3  : disable MP3 support\n";
 print "--Xcflags=val  : special C flags used for MP3 libraries\n";
 print "--Xcppflags=val: special C++ flags used for MP3 libraries\n";
 print "--with-mixer   : include code to control the mixer [default]\n";
 print "--without-mixer: don't include code to control the mixer\n";
 print "--shipped-intl : force to use the shipped gettext library [DOS only]\n";
 print "--tv-include=pa: path for Turbo Vision includes\n";
 print "--tv-lib=path  : path for Turbo Vision libraries\n";
 print "  Note: if you use --tv-include you should also use --tv-lib\n";
}

sub GiveAdvice
{
 if (@conf{'HAVE_PCRE_LIB'} eq 'no')
   {
    print "\n";
    print "* The Perl Compatible Regular Expressions support was disabled install PCRE\n";
    print "  library v2.0 or newer and reconfigure to get support.\n";
   }
 if (($OS eq 'DOS') && (@conf{'HAVE_ALLEGRO'} eq 'no') &&
     (@conf{'mp3'} eq 'yes'))
   {
    print "* The MPEG-3 files support was disabled, install Allegro library v$AllegroVersionNeeded\n";
    print "  or newer (not a WIP!) and reconfigure to get support.\n";
   }
 if ((@conf{'recode'} eq 'no') && ($OS eq 'UNIX'))
   {
    print "* The 'recode' tool isn't installed internationalization could be broken\n";
   }
 if (@conf{'xgettext'} eq 'no')
   {
    print "* The 'xgettext' tools aren't installed internat. files can't be created.\n";
   }
 if (@conf{'makeinfo'} eq 'no')
   {
    print "* The 'makeinfo' tool isn't installed documentation can't be created.\n";
   }
}

sub LookForRecode
{
 my $test;

 print 'Looking for recode: ';
 if (@conf{'recode'})
   {
    print @conf{'recode'}." (cached)\n";
    return;
   }
 $test=`recode --version`;
 if ($test=~/(\d+\.\d+(\.\d+)?)/)
   {
    print "$1\n";
    $conf{'recode'}=$1;
    if (CompareVersion($test,'3.5'))
      {
       $conf{'recode_sep'}='..';
      }
    else
      {
       $conf{'recode_sep'}=':';
      }
   }
 else
   {
    print "no\n";
    $conf{'recode'}='no';
   }
}

sub LookForGettextTools
{
 my $test;

 print 'Looking for xgettext: ';
 if (@conf{'xgettext'})
   {
    print @conf{'xgettext'}." (cached)\n";
    return;
   }
 $test=`xgettext --version`;
 if ($test=~/(\d+\.\d+(\.\d+)?)/)
   {
    print "$1\n";
    $conf{'xgettext'}=$1;
   }
 else
   {
    print "no\n";
    $conf{'xgettext'}='no';
   }
}

sub LookForMakeinfo
{
 my $test;

 print 'Looking for makeinfo: ';
 if (@conf{'makeinfo'})
   {
    print @conf{'makeinfo'}." (cached)\n";
    return;
   }
 $test=`makeinfo --version`;
 if ($test=~/(\d+\.\d+(\.\d+)?)(.*)(\d+\.\d+(\.\d+)?)/)
   {
    print "$4\n";
    $conf{'makeinfo'}=$4;
   }
 elsif ($test=~/(\d+\.\d+(\.\d+)?)/)
   {
    print "$1\n";
    $conf{'makeinfo'}=$1;
   }
 else
   {
    print "no\n";
    $conf{'makeinfo'}='no';
   }
}

sub LookForTV
{
 my @dirsI,@dirsL,$parent;

 print 'Looking for installed Turbo Vision: ';
 if (@conf{'TV_INCLUDE'} && @conf{'TV_LIB'} && !$TVCommandLine)
   {
    $TVInclude=@conf{'TV_INCLUDE'};
    $TVLib=@conf{'TV_LIB'};
    print "\n\tInclude dir: $TVInclude (cached)\n";
    print "\tLibrary dir: $TVLib (cached)\n";
    return;
   }
 #$parent=ParentDir();
 $parent='..';
 if ($OS eq 'DOS')
   {
    @dirsI=("$parent/tvision/include",$ENV{'DJDIR'}.'/include/tvision');
    @dirsL=("$parent/tvision/djgpp",$ENV{'DJDIR'}.'/lib');
   }
 elsif ($OS eq 'UNIX')
   {
    @dirsI=("$parent/tvision/include",'/usr/include/rhtvision');
    if (@conf{'static'} eq 'yes')
      {
       @dirsL=("$parent/tvision/linux",'/usr/lib');
      }
    else
      {
       @dirsL=("$parent/tvision/linuxso","$parent/tvision/linux",'/usr/lib');
      }
   }
 elsif (($OS eq 'Win32') && ($OSflavor eq 'Mingw'))
   {
    @dirsI=("$parent/tvision/include",@conf{'prefix'}.'/include/tvision');
    @dirsL=("$parent/tvision/win32",@conf{'prefix'}.'/lib');
   }
 @dirsI=($conf{'TV_INCLUDE'},@dirsI) if $conf{'TV_INCLUDE'};
 $TVInclude=LookForFile('tv.h',@dirsI);
 if (!length($TVInclude))
   {
    print "\n\nError: Can't find an installed version, please install Turbo Vision first.\n";
    print "The headers should be installed in one of the following directories:\n@dirsI\n";
    CreateCache();
    die "Missing library\n";
   }
 print "\n\tInclude dir: $TVInclude\n";
 @dirsL=($conf{'TV_LIB'},@dirsL) if $conf{'TV_LIB'};
 $TVLib=LookForFile('librhtv.[as]*',@dirsL);
 if (!length($TVLib))
   {
    print "\nError: Can't find a compiled version, please compile Turbo Vision first.\n";
    print "The library should be installed in one of the following directories:\n@dirsL\n";
    CreateCache();
    die "Missing library\n";
   }
 print "\tLibrary dir: $TVLib\n";
 $conf{'TV_INCLUDE'}=$TVInclude;
 $conf{'TV_LIB'}=$TVLib;
}

sub LookForPCRE
{
 my $test205,$test206,$t2,$test,$dir;

 print 'Looking for PCRE library: ';
 $test=@conf{'HAVE_PCRE_LIB'};
 if ($test)
   {
    print "$test (cached)\n";
    return;
   }
 $test205='
#include <stdio.h>
#include <stdlib.h>
#include <pcre.h>
int main(void)
{
 pcre *compiled;
 int flags=PCRE_MULTILINE | PCRE_CASELESS;
 const char *error;
 int   errorOffset,hits,max,*matchs;
 compiled=pcre_compile("test",flags,&error,&errorOffset,0);
 if (!compiled) return 1;
 max=(pcre_info(compiled,0,0)+1)*3;
 matchs=(int *)malloc(max*sizeof(int));
 hits=pcre_exec(compiled,0,"This is just a test 123",23,';
 $t2='0,matchs,max);
 if (hits!=1) return 1;
 printf("OK\n");
 return 0;
}
';
 $test206=$test205.'0,'.$t2;
 $test205.=$t2;

 # See if 2.0.6+ is installed
 $test=RunGCCTest($GCC,'c',$test206,"-lpcre");
 if ($test eq "OK\n")
   {
    print "v2.0.6 or better OK\n";
    $conf{'HAVE_PCRE_LIB'}='yes';
    $conf{'HAVE_PCRE206'}='yes';
    $conf{'PCREShipped'}='no';
    return;
   }
 print 'no 2.0.6+, ';
 # See if 2.0+ is installed
 $test=RunGCCTest($GCC,'c',$test205,"-lpcre");
 if ($test eq "OK\n")
   {
    print "v2.0 or better OK\n";
    $conf{'HAVE_PCRE_LIB'}='yes';
    $conf{'HAVE_PCRE206'}='no';
    $conf{'PCREShipped'}='no';
    return;
   }
 print 'no 2.0+, ';
 # See if the one shipped works here
 #$test=RunGCCTest($GCC,'c',$test206,"-L$supportDir -lpcre -Isupport");
 #if ($test ne "OK\n")
 #  {
 #   print "no shipped, disabling PCRE\n";
 #   $conf{'HAVE_PCRE_LIB'}='no';
 #   $conf{'HAVE_PCRE206'}='no';
 #   $conf{'PCREShipped'}='no';
 #   return;
 #  }

 $conf{'HAVE_PCRE_LIB'}='yes';
 $conf{'HAVE_PCRE206'}='yes';
 $conf{'PCREShipped'}='yes';
 print "using shipped one.\n";
}

sub TestTVVersion
{
 my $vNeed=$_[0];
 my $test;

 print 'Checking TV version: ';
 $test=@conf{'tv'};
 if ($test)
   {
    print "$test (cached) OK\n";
    return;
   }
 $test='
#include <stdio.h>
#include <tv.h>
int main(void)
{
 printf("%d.%d.%d",TV_MAJOR_VERSION,TV_MIDDLE_VERSION,TV_MINOR_VERSION);
 return 0;
}
';
 $test=RunGCCTest($GXX,'cc',$test,"$stdcxx -I$TVInclude");
 if (!CompareVersion($test,$vNeed))
   {
    print "\nWrong TV version, please use $vNeed or newer\n";
    print "Look in $ErrorLog for potential compile errors of the test\n";
    CreateCache();
    die "Wrong version\n";
   }
 print "$test OK\n";
 $conf{'tv'}=$test;
}


sub LookForAllegro
{
 my $vNeed=$_[0],$vMaxV=$_[1];
 my $test;

 print 'Looking for Allegro library: ';

 $test=@conf{'HAVE_ALLEGRO'};
 if ($test)
   {
    print "$test (cached)\n";
    return;
   }
 $test='
#include <stdio.h>
#include <allegro.h>
int main(void)
{
 allegro_init();
 // Defined by 3.9.x
 #ifdef ALLEGRO_WIP_VERSION
 print ("%d.%d.%d",ALLEGRO_VERSION,ALLEGRO_SUB_VERSION,ALLEGRO_WIP_VERSION);
 #else
 printf("%d.0.%d",ALLEGRO_VERSION,ALLEGRO_SUB_VERSION);
 #endif
 return 0;
}
';
 $MP3Support=0;
 $test=RunGCCTest($GCC,'c',$test,"-lalleg");
 if (!length($test))
   {
    print "not installed, disabling MP3 support.\n";
    $conf{'HAVE_ALLEGRO'}='no';
    return;
   }
 if (!CompareVersion($test,$vNeed))
   {
    print "$test -> No, disabling MP3 support.\n";
    print "\nWrong Allegro version, please use $vNeed or newer, but not greater or equal\n";
    print "to $vMaxV.\n\n";
    $conf{'HAVE_ALLEGRO'}='no';
    return;
   }
 if (CompareVersion($test,$vMaxV))
   {
    print "$test -> No, disabling MP3 support.\n";
    print "\nSorry, I need a non-WIP release\n\n";
    $conf{'HAVE_ALLEGRO'}='no';
    return;
   }
 $MP3Support=1;
 $conf{'HAVE_ALLEGRO'}='yes';
 print "$test OK\n";
}


sub LookForZLib
{
 my $vNeed=$_[0];
 my $test,$ver;

 print 'Looking for zlib: ';
 $test=@conf{'zlib'};
 if ($test)
   {
    print "$test (cached) OK\n";
    return;
   }
 $test='
#include <stdio.h>
#include <zlib.h>
int main(void)
{
 gzFile f;
 f=gzopen("","rb");
 printf("%s",ZLIB_VERSION);
 return 0;
}';
 $ver=RunGCCTest($GCC,'c',$test,'-lz');
 if (length($ver))
   {
    if (CompareVersion($ver,$vNeed))
      {
       print "$ver OK\n";
       $conf{'zlibShipped'}='no';
       $conf{'zlib'}=$ver;
       return;
      }
    print "no $vNeed+, ";
   }
 else
   {
    print 'not installed, ';
   }
 #$test=RunGCCTest($GCC,'c',$test,"-lz -Isupport -L$supportDir");
 #if (!CompareVersion($test,$vNeed))
 #  {
 #   print "no shipped\n";
 #   print "\n\nError: Can't find an installed version, please install zlib 1.1.2 or better first.\n";
 #   CreateCache();
 #   die "Missing library\n";
 #  }
 print "using shipped one\n";
 $conf{'zlibShipped'}='yes';
 $conf{'zlib'}='shipped'; #$test
}

sub LookForIntlSupport
{
 my $vNeed=$_[0];
 my ($test,$a,$djdir,$intllib,$intltest);

 print 'Checking for international support: ';
 $conf{'intlShipped'}='no';
 if ((@conf{'force-intlShipped'} eq 'yes') && ($OS eq 'DOS'))
   {
    print "using shipped one by user request.\n";
    $conf{'intl'}='yes';
    $conf{'iconv'}='no';
    $conf{'intlShipped'}='yes';
    return;
   }
 if (@conf{'no-intl'} eq 'yes')
   {
    print "disabled by user request.\n";
    $conf{'intl'}='no';
    $conf{'iconv'}='no';
    #`cp include/tv/nointl.h include/tv/intl.h`;
    return;
   }
 if (@conf{'intl'} eq 'yes')
   {
    print "yes (cached)\n";
    return;
   }
 if (@conf{'intl'} eq 'no')
   {
    print "no (cached)\n";
    return;
   }
 if ($OS eq 'DOS')
   { # gettext 0.10.32 port have a bug in the headers, correct it
    $djdir=@ENV{'DJDIR'};
    $a=cat("$djdir/include/libintl.h");
    if (length($a) && $a=~/\@INCLUDE_LOCALE_H@/)
      {
       $a=~s/\@INCLUDE_LOCALE_H\@//;
       replace("$djdir/include/libintl.h",$a);
      }
   }
 $intltest='
#include <stdio.h>
#define FORCE_INTL_SUPPORT
#include <tv/intl.h>
int main(void)
{
 printf("%s\n",_("OK"));
 return 0;
}';
 $intllib=$OS eq 'DOS' ? '-lintl' : '';
 $test=RunGCCTest($GCC,'c',$intltest,"-I$TVInclude ".$intllib);
 if ($test ne "OK\n")
   {
    print "no, additional check required.\n";
    print "Checking for extra libs for international support: ";
    $test=RunGCCTest($GCC,'c',$intltest,"-I$TVInclude ".$intllib.' -liconv');
    if ($test ne "OK\n")
      {
       print "none found, using shipped one\n";
       $conf{'intl'}='yes';
       $conf{'iconv'}='no';
       $conf{'intlShipped'}='yes';
      }
    else
      {
       print "-liconv, OK\n";
       $conf{'intl'}='yes';
       $conf{'iconv'}='yes';
      }
   }
 else
   {
    print "yes OK\n";
    $conf{'intl'}='yes';
    $conf{'iconv'}='no';
   }
}

sub CreateConfigH
{
 my $text="/* Generated automatically by the configure script */",$old;

 print "Generating configuration header\n";

 $conf{'FORCE_INTL_SUPPORT'}=$conf{'intlShipped'};
 $text.=ConfigIncDefYes('HAVE_ALLEGRO','Allegro library is available');
 $text.=ConfigIncDefYes('WITH_MP3','Enable MP3 support');
 $text.=ConfigIncDefYes('HAVE_AMP','MP3 support from libamp');
 $text.=ConfigIncDefYes('HAVE_MPEGSOUND','MP3 support from libmpegsound');
 $text.=ConfigIncDefYes('HAVE_PCRE_LIB','Perl Compatible Regular Expressions support');
 $text.=ConfigIncDefYes('HAVE_PCRE206','PCRE version 2.0.6 or newer');
 $text.=ConfigIncDefYes('HAVE_BZIP2','bzip2 compression support');
 $text.=ConfigIncDefYes('HAVE_MIXER','Sound mixer support');
 $text.=ConfigIncDefYes('FORCE_INTL_SUPPORT','Gettext included with editor');
 $text.="\n\n";
 $text.="#define SEOS_$OS\n#define SEOSf_$OSflavor\n";

 $old=cat('include/configed.h');
 replace('include/configed.h',$text) unless $text eq $old;
}

sub GenerateMakefile
{
 my $text="# Generated automatically by the configure script";
 my ($libamp,$libset,$infview,$libbzip2,$libmpegsnd,$libz,$libpcre,$libintl);

 print "Generating Makefile\n";

 $text.="\n\nMPREFIX=$conf{'prefix'}";
 $text.="\nlibdir=\$(MPREFIX)/lib";
 $text.="\nCFLAGS=$conf{'CFLAGS'}";
 $text.="\nCXXFLAGS=$conf{'CXXFLAGS'}";
 $text.="\nSET_USE_FHS=$conf{'fhs'}" unless ($OS eq 'DOS');
 $text.="\nexport";

 #### Targets ####
 $libset=@conf{'libset'} eq 'yes';
 $libamp=@conf{'HAVE_AMP'} eq 'yes';
 $libmpegsnd=@conf{'HAVE_MPEGSOUND'} eq 'yes';
 $infview=@conf{'infview'} eq 'yes';
 $libbzip2=@conf{'HAVE_BZIP2'};
 $libz=@conf{'zlibShipped'} eq 'yes';
 $libpcre=@conf{'PCREShipped'} eq 'yes';
 $libintl=@conf{'intlShipped'} eq 'yes';
 $plasmas=$OS eq 'DOS';
 $text.="\n\n.PHONY:" if ($infview || $plasmas || $libbzip2 || $libz || $libmpegsnd || $libamp);
 $text.=" infview" if ($infview);
 $text.=" plasmas" if ($plasmas);
 $text.=" libbzip2" if ($libbzip2);
 $text.=" libz" if ($libz);
 $text.=" libmpegsnd" if ($libmpegsnd);
 $text.=" libpcre" if ($libpcre);
 $text.=" libamp" if ($libamp);
 $text.=" libintl" if ($libintl);
 # all targets
 $text.="\n\nall: editor";
 $text.=" libset" if ($libset);
 $text.=" infview" if ($infview);
 $text.=" plasmas" if ($plasmas);
 $text.="\n";
 # libamp
 if ($libamp)
   {
    $text.="\n\nlibamp:\n";
    $text.="\t\$(MAKE) -C mp3/libamp -f libamp.mak";
   }
 # libmpegsnd
 if ($libmpegsnd)
   {
    $text.="\n\nlibmpegsnd:\n";
    $text.="\t\$(MAKE) -C mp3/mpegsound -f mpegsnd.mak";
   }
 # libbzip2
 if ($libbzip2)
   {
    $text.="\n\nlibbzip2:\n";
    $text.="\t\$(MAKE) -C libbzip2 libbz2.a";
   }
 # libz
 if ($libz)
   {
    $text.="\n\nlibz:\n";
    $text.="\t\$(MAKE) -C libz libz.a";
   }
 # libpcre
 if ($libpcre)
   {
    $text.="\n\nlibpcre:\n";
    $text.="\t\$(MAKE) -C libpcre libpcre.a";
   }
 # libintl
 if ($libintl)
   {
    $text.="\n\nlibintl:\n";
    $text.="\t\$(MAKE) -C gettext";
   }
 # editor
 $text.="\n\neditor:";
 $text.=" libamp"   if ($libamp);
 $text.=" libmpegsnd"   if ($libmpegsnd);
 $text.=" libbzip2" if ($libbzip2);
 $text.=" libz" if ($libz);
 $text.=" libpcre" if ($libpcre);
 $text.=" libintl" if ($libintl);
 $text.="\n\t\$(MAKE) -C makes";
 # libset
 if ($libset)
   {
    $text.="\n\nlibset:\n";
    $text.="\tcd makes; \$(MAKE) -f libset.mak; cd ..";
   }
 # infview
 if ($infview)
   {
    $text.="\n\ninfview:\n";
    $text.="\t\$(MAKE) -C makes infview";
   }
 # plasmas
 if ($plasmas)
   {
    $text.="\n\nplasmas:\n";
    $text.="\tcd scrnsave; \$(MAKE); cd ..";
   }

 #### Installations ####
 # editor
 $text.="\n\ninstall-editor: editor\n";
 $text.="\t\$(MAKE) -C internac\n" unless @conf{'xgettext'} eq 'no';
 $text.="\t\$(MAKE) -C makes install";
 # libset
 if ($libset)
   {
    $text.="\n\ninstall-libset: libset\n";
    $text.="\tinstall -d -m 0755 \$(libdir)\n";
    $text.="\tinstall -m 0644 makes/libset.a \$(libdir)";
   }
 # infview
 if ($infview)
   {
    $text.="\n\ninstall-infview: infview\n";
    $text.="\t\$(MAKE) -C makes install-infview";
   }
 # all targets
 $text.="\n\ninstall: install-editor";
 $text.=" install-libset" if ($libset);
 $text.=" install-infview" if ($infview);
 $text.="\n";

 #### Distribution ####
 # editor
 $text.="\n\ndistrib-editor:\n";
 $text.="\t\$(MAKE) -C internac\n" unless @conf{'xgettext'} eq 'no';
 $text.="\t\$(MAKE) -C makes distrib";
 # infview
 if ($infview)
   {
    $text.="\n\ndistrib-infview:\n";
    $text.="\t\$(MAKE) -C makes distrib-infview";
   }
 # all targets
 $text.="\n\ndistrib: distrib-editor";
 $text.=" distrib-infview" if ($infview);
 $text.="\n";

 $text.="\nclean:\n";
 $text.="\tcd makes; \$(MAKE) clean-o; \$(MAKE) clean-docs; cd ..\n";
 $text.="\trm -f configure.cache\n";
 $text.="\trm -f Makefile\n";
 $text.="\trm -f errormsg.txt\n";
 $text.="\trm -f doc/sdh.exe\n";
 $text.="\trm -f include/configed.h\n";
 $text.="\trm -rf makes/linux/result makes/linux/resultInf makes/linux/setedit-* makes/linux/infview-*\n" if ($OS eq 'UNIX');
 $text.="\trm -rf makes/djgpp/distinf makes/djgpp/distrib makes/djgpp/result makes/djgpp/resulinf\n" if ($OS eq 'DOS');
 $text.="\trm -f makes/*.a makes/*.exe makes/*.gdt\n";
 $text.="\trm -f mp3/libamp/*.a mp3/libamp/obj/*.o\n";
 $text.="\tcd scrnsave; \$(MAKE) clean-o; cd ..\n" if ($plasmas);
 $text.="\t\$(MAKE) -C libbzip2 clean\n" if ($libbzip2);
 $text.="\t\$(MAKE) -C libz clean\n" if ($libz);
 $text.="\t\$(MAKE) -C libpcre clean\n" if ($libpcre);

 replace('Makefile',$text);
}

