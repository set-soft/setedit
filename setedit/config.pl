#!/usr/bin/perl
# Copyright (C) 1999-2004 by Salvador E. Tropea (SET),
# see copyrigh file for details
#

require "miscperl.pl";
require "conflib.pl";

$conf{'infview'}='yes';
#$conf{'HAVE_BZIP2'}='yes';
$conf{'parser'}='parserbr.c';
$conf{'mp3lib'}='mpegsnd';
$conf{'mp3'}='yes';
$conf{'HAVE_MIXER'}='yes';
$conf{'HAVE_CALCULATOR'}='yes';
$conf{'HAVE_CALENDAR'}='yes';
$conf{'intlShipped'}='no';
$conf{'ToolsInstaller'}='no';
$conf{'ToolsDistrib'}='no';
$conf{'compressExe'}='undef';
$TVCommandLine=0;

# If the script is newer discard the cache.
#GetCache() unless (-M 'config.pl' < -M 'configure.cache');
GetVersion('');

$TVVersionNeeded='2.0.3';
$ZLibVersionNeeded='1.1.2';
$BZ2LibVersionNeeded='0.9.5d';
$DJGPPVersionNeeded='2.0.2';
$LibMIVersionNeeded='0.8.6';
# Allegro 3.1==3.0.1 3.11==3.0.11 3.12==3.0.12
$AllegroVersionNeeded='3.0.1';
# 5.0 will change the API, or maybe 6.0 but an API change is discussed
$AllegroNotNeeded='5.0.0';
# I never tested with an older version, you can try reducing it.
$GPMVersionNeeded='1.10';
# Workarounded 4.4+ missfeatures, so now I put here a fantasy value
$BrokenMakeinfo='99.99';
unlink $ErrorLog;

SeeCommandLine();

if ($JustSpec)
  {
   UpdateSpec();
   exit 0;
  }

print "Configuring SETEdit v$Version\n\n";
# Determine the OS
$OS=DetectOS();

if (($OS ne 'DOS') && (@conf{'mp3'} eq 'yes') && (@conf{'mp3lib'} ne 'mpegsnd'))
  {
   die "Only libmpegsnd is available for UNIX\n"
  }
# MP3 code works only for DOS, Linux and Solaris
if (($OS ne 'DOS') && ($OSf ne 'Linux') && ($OSf ne 'Solaris'))
  {
   $conf{'mp3'}='no';
   $conf{'HAVE_MIXER'}='no';
   $conf{'mp3lib'}='';
  }
if (($OS ne 'UNIX') && ($conf{'HAVE_AA'} eq 'yes'))
  {
   print "Currently AA-lib is usable only for UNIX version, please tell me if you think it should be changed.\n";
   $conf{'HAVE_AA'}='no';
  }

LookForBasicTools();
$supportDir='makes/'.$supportDir;

# TV goes first to find the configuration program.
# But we can't check TV functionality yet.
# Where is the TV library?
# $TVInclude and $TVLib
LookForTV();
LookForTVConfig();

# Determine C flags
$CFLAGS=FindCFLAGS();
FindXCFLAGS();
# Determine C++ flags
$CXXFLAGS=FindCXXFLAGS();
FindXCXXFLAGS();
# Extra lib directories
$LDExtraDirs=FindLDExtraDirs();
# Test for a working gcc
$GCC=CheckGCC();
# Which architecture are we using?
DetectCPU();
# Some platforms aren't easy to detect until we can compile.
DetectOS2();
# The prefix can be better determined if we know all the rest
# about the environment
LookForPrefix();
# Only gnu make have the command line and commands we use.
LookForGNUMake();
# Same for ar, it could be `gar'
$GAR=LookForGNUar();
# Similar for install tool.
LookForGNUinstall();
# Check if gcc can compile C++
$GXX=CheckGXX();

# Is the right djgpp?
if ($OS eq 'DOS')
  {
   LookForDJGPP($DJGPPVersionNeeded);
   LookForAllegro($AllegroVersionNeeded,$AllegroNotNeeded);
  }

# Is TV the right version?
TestTVVersion($TVVersionNeeded);
# Find the major version
if ($conf{'tv'}=~/(\d+)\.(\d+)\.(\d+)/)
  {
   $tvMajor=$1;
   $tvMiddle=$2;
   $tvMinor=$3;
  }

# Have libc international support? what about libintl or libiconv?
LookForIntlSupport();
# Is PCRE available?
LookForPCRE();
# Is ZLib available?
LookForZLib($ZLibVersionNeeded);
# Is BZip2 library available?
LookForBZ2Lib($BZ2LibVersionNeeded);
# Look for xgettext
LookForGettextTools();
LookForMakeinfo();
# Is a usable gpm there?
#LookForGPM($GPMVersionNeeded) if ($OS eq 'UNIX');
# Should we try X?
#LookForXlib() if (($OS eq 'UNIX') && ($tvMajor>=2));
# Needed by X libraries in some systems
LookForDL() if ($OS eq 'UNIX');
# An option to display screen savers ;-)
LookForAA() if ($OS eq 'UNIX');
# GDB/MI interface
LookForMI($LibMIVersionNeeded) if (($OS eq 'UNIX') || ($Compf eq 'Cygwin'));
#  Check if we can offer the distrib targets.
LookForToolsDistrib();
#  The installer needs tons of things, put it in makefile only if the user
# have some chance to succeed.
LookForToolsInstaller() if ($OS eq 'DOS');

print "\n";
#
# Create a list of RHIDE variables to configure the projects
#
$TVLib='../'.$TVLib if (substr($TVLib,0,2) eq '..');
$TVInclude='../'.$TVInclude if (substr($TVInclude,0,2) eq '..');
if ($OS eq 'DOS')
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=$(DJDIR)/include $(DJDIR)/lang/cxx $(DJDIR)/lib/gcc-lib';
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS=-lrhtv ';
   $MakeDefsRHIDE[1].='-l'.substr($stdcxx,2).' ';
   $MakeDefsRHIDE[1].='-lintl ' unless (@conf{'intl'} eq 'no');
   $MakeDefsRHIDE[1].='-liconv ' if (@conf{'iconv'} eq 'yes');
   if ((@conf{'mp3'} eq 'yes') && (@conf{'HAVE_ALLEGRO'} eq 'yes'))
     {
      $MakeDefsRHIDE[1].='-l'.@conf{'mp3lib'}.' -lalleg ';
     }
   $MakeDefsRHIDE[1].='-lbz2 ' if (@conf{'HAVE_BZIP2'} eq 'yes');
  }
elsif ($OS eq 'UNIX')
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=/usr/include /usr/local/include /usr/include/g++ /usr/local/include/g++ /usr/lib/gcc-lib /usr/local/lib/gcc-lib';
   if (@conf{'static'} eq 'yes')
      { $libs=TVConfigOption('slibs'); }
   else
      { $libs=TVConfigOption('dlibs'); }
   chop $libs;
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS='.$libs.' ';
   #
   # The following are TV dependencies
   #
   ## RHIDE doesn't know about anything different than DJGPP and Linux so -lstdc++ must
   ## be added for things like FreeBSD or SunOS.
   #$MakeDefsRHIDE[1].=substr($stdcxx,2).' '; # unless ($OSf eq 'Linux');
   #$MakeDefsRHIDE[1].='ncurses m ';
   ## No for UNIX!! $MakeDefsRHIDE[1].='intl ' unless (@conf{'intl'} eq 'no');
   #$MakeDefsRHIDE[1].='gpm ' if @conf{'HAVE_GPM'} eq 'yes';
   #$MakeDefsRHIDE[1].=$conf{'X11Lib'}.' ' if ($conf{'HAVE_X11'} eq 'yes');
   if ($conf{'dl'} eq 'yes')
     {
      $MakeDefsRHIDE[1].=($OSf eq 'QNXRtP') ? '' : '-ldl ';
     }
   $MakeDefsRHIDE[1].='-lbz2 ' if @conf{'HAVE_BZIP2'} eq 'yes';
   $MakeDefsRHIDE[1].='-l'.@conf{'mp3lib'}.' ' if (@conf{'mp3'} eq 'yes');
   $MakeDefsRHIDE[1].='-lintl ' if ((($OSf eq 'FreeBSD') || ($OSf eq 'QNXRtP')) && ($conf{'intl'} eq 'yes'));
   $MakeDefsRHIDE[1].='-laa ' if ($conf{'HAVE_AA'} eq 'yes');
  }
else # Win32
  {
   $MakeDefsRHIDE[0]='RHIDE_STDINC=';
   $libs=TVConfigOption('slibs');
   chop $libs;
   $MakeDefsRHIDE[1]='RHIDE_OS_LIBS= '.$libs.' ';
   #$MakeDefsRHIDE[1]='RHIDE_OS_LIBS=rhtv stdc++ gdi32 ';
   #$MakeDefsRHIDE[1].='intl ' unless (@conf{'intl'} eq 'no');
   $MakeDefsRHIDE[1].='-lbz2 ' if (@conf{'HAVE_BZIP2'} eq 'yes');
   $MakeDefsRHIDE[1].='-l'.@conf{'mp3lib'}.' ' if (@conf{'mp3'} eq 'yes');
  }
$MakeDefsRHIDE[1].='-lz ';
$MakeDefsRHIDE[1].='-lpcre '    if @conf{'HAVE_PCRE_LIB'} eq 'yes';
$MakeDefsRHIDE[1].='-lmigdb '   if @conf{'HAVE_GDB_MI'} eq 'yes';
$MakeDefsRHIDE[1].='-lmss '     if @conf{'mss'} eq 'yes';
$MakeDefsRHIDE[1].='-lefence '  if @conf{'efence'} eq 'yes';
$MakeDefsRHIDE[1].='-ltvfintl ' if @conf{'tvfintl'} eq 'yes';

$MakeDefsRHIDE[2]="RHIDE_OS_LIBS_PATH=";
# Before the system one
$MakeDefsRHIDE[2].=' -L../gettext '  if (@conf{'intlShipped'} eq 'yes');
# QNX Workaround
$MakeDefsRHIDE[2].='-L/lib ' if ($OSf eq 'QNXRtP');
# Libraries for TV
$libs=TVConfigOption('dir-libs');
chop $libs;
$MakeDefsRHIDE[2].=$libs;
# Extra libraries path
$libs=$LDExtraDirs;
$libs=~s/(\S+)/-L$1/g;
$MakeDefsRHIDE[2].=' '.$libs;
# Shipped replacements
$MakeDefsRHIDE[2].=' -L../libz'     if (@conf{'zlibShipped'} eq 'yes');
$MakeDefsRHIDE[2].=' -L../libbzip2' if (@conf{'bz2libShipped'} eq 'yes');
$MakeDefsRHIDE[2].=' -L../libpcre'  if (@conf{'PCREShipped'} eq 'yes');
$MakeDefsRHIDE[2].=' -L../libmigdb/src' if (@conf{'migdbShipped'} eq 'yes');
# Subprojects generates libraries in the makes directory
$MakeDefsRHIDE[2].=' -L.';

$MakeDefsRHIDE[3]="TVISION_INC=$TVInclude";

$test='';
$test.=' ../libz' if (@conf{'zlibShipped'} eq 'yes');
$test.=' ../libbzip2' if (@conf{'bz2libShipped'} eq 'yes');
$test.=' ../libpcre' if (@conf{'PCREShipped'} eq 'yes');
$test.=' ../gettext' if (@conf{'intlShipped'} eq 'yes');
$test.=' '.$conf{'X11IncludePath'} if (@conf{'HAVE_X11'} eq 'yes');
$test.=' ../libmigdb/src' if (@conf{'migdbShipped'} eq 'yes');
$test.=$conf{'EXTRA_INCLUDE_DIRS'};
$MakeDefsRHIDE[4]='SUPPORT_INC='.$test;

# The support libraries shouldn't generate dependencies
$MakeDefsRHIDE[0].=$test;
# Nor compatlayer headers
$MakeDefsRHIDE[0].=' $(TVISION_INC)/cl';
if (@conf{'static'} eq 'yes')
  {
   $MakeDefsRHIDE[5]='RHIDE_COMPILE_LINK=$(RHIDE_LD) $(RHIDE_LIBDIRS) $(LDFLAGS) -static $(RHIDE_LDFLAGS) $(C_EXTRA_FLAGS) -o $(OUTFILE)  $(OBJFILES) $(LIBRARIES) $(RHIDE_LIBS)';
  }
else
  {
   $MakeDefsRHIDE[5]='RHIDE_COMPILE_LINK=$(RHIDE_LD) $(RHIDE_LIBDIRS) $(LDFLAGS) $(RHIDE_LDFLAGS) $(C_EXTRA_FLAGS) -o $(OUTFILE)  $(OBJFILES) $(LIBRARIES) $(RHIDE_LIBS)';
  }
#$MakeDefsRHIDE[5].=' -lrhtv '.@conf{'prefix'}.'/lib/libmss.a' if (@conf{'mss'} eq 'yes');
#if (@conf{'mss'} eq 'yes')
#  {
#   $test=@conf{'prefix'}.'/lib/libmss.a $(LIBRARIES)';
#   $MakeDefsRHIDE[5]=~s/\$\(LIBRARIES\)/$test/;
#  }
# Take out the CFLAGS and CPPFLAGS variables
$MakeDefsRHIDE[6]='RHIDE_COMPILE_C=$(RHIDE_GCC) $(RHIDE_INCLUDES) $(C_DEBUG_FLAGS) $(C_OPT_FLAGS)  $(C_WARN_FLAGS) $(C_C_LANG_FLAGS) $(C_EXTRA_FLAGS) $(LOCAL_OPT) $(RHIDE_OS_CFLAGS) -c $(SOURCE_NAME) -o $(OUTFILE)';
$MakeDefsRHIDE[7]='RHIDE_COMPILE_CC=$(RHIDE_GXX) $(RHIDE_INCLUDES) $(C_DEBUG_FLAGS) $(C_OPT_FLAGS)  $(C_WARN_FLAGS) $(C_C_LANG_FLAGS) $(C_CXX_LANG_FLAGS) $(C_EXTRA_FLAGS) $(RHIDE_OS_CXXFLAGS) $(LOCAL_OPT) -c $(SOURCE_NAME) -o $(OUTFILE)';
$MakeDefsRHIDE[8]='STDCPP_LIB='.$stdcxx;
$MakeDefsRHIDE[9]='RHIDE_LIBS=$(LIBS) $(RHIDE_OS_LIBS)';
$MakeDefsRHIDE[10]='RHIDE_LIBDIRS=$(LIB_DIRS)';
if ($Compf eq 'MinGW')
  {
   $MakeDefsRHIDE[11]='SPECIAL_LDFLAGS=-mconsole';
  }
CreateRHIDEenvs('makes/rhide.env','+mp3/libamp/rhide.env',
                '+mp3/mpegsound/rhide.env');
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
UpdateSpec();
$ReplaceTags{'TVInclude'}=$TVInclude;
$ReplaceTags{'datadir'}=$conf{'prefix'}.'/share';
$ReplaceTags{'libdir'}=$conf{'prefix'}.'/lib';
$ReplaceTags{'CC'}=$GCC;
$ReplaceTags{'CXX'}=$GXX;
$ReplaceTags{'AR'}=$conf{'GNU_AR'};
$ReplaceTags{'CFLAGS'}=$conf{'CFLAGS'};
$ReplaceTags{'XCFLAGS'}=$conf{'XCFLAGS'};
$ReplaceTags{'CXXFLAGS'}=$conf{'CXXFLAGS'};
$ReplaceTags{'dyndir'}=$conf{'prefix'}.'/lib/setedit';
$ReplaceTags{'PREFIX'}=$conf{'prefix'};
$ReplaceTags{'MakeInfoNumbers'}=CompareVersion(@conf{'makeinfo'},'4.7') ? '-N' : '';
# Makefiles
ReplaceText('gettext/Makefile.in','gettext/Makefile');
ReplaceText('holidays/Makefile.in','holidays/Makefile');
ReplaceText('doc/gnumake.in','doc/Makefile');
ReplaceText('internac/gnumake.in','internac/Makefile');
ReplaceText('libbzip2/gnumake.in','libbzip2/Makefile');
ReplaceText('libpcre/gnumake.in','libpcre/Makefile');
ReplaceText('libz/gnumake.in','libz/Makefile');
`cp gettext/djgpp.h gettext/config.h`; # Currently only DOS config is available if $OS eq 'DOS';

#
# Generate BC++ and MSVC makefiles
#
`perl confignt.pl` if @conf{'MAINTAINER_MODE'} eq 'yes';

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

sub UpdateSpec
{
 $ReplaceTags{'version'}=$Version;
 ReplaceText('redhat/setedit.spec.in','redhat/setedit-'.$Version.'.spec');
}

sub SeeCommandLine
{
 my $i;

 $conf{'HAVE_AA_from_user'}='no';
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
    elsif ($i eq '--no-prefix-h')
      {
       $conf{'no-prefix-h'}=1;
      }
    elsif ($i eq '--no-intl')
      {
       $conf{'no-intl'}='yes';
       $conf{'xgettext'}='no';
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
       $conf{'bz2libShipped'}='no';
       $conf{'bz2lib'}='no';
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
       $conf{'mp3'}='yes';
      }
    elsif ($i eq '--with-mpegsnd')
      {
       $conf{'mp3lib'}='mpegsnd';
       $conf{'mp3'}='yes';
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
    elsif ($i=~'--cflags=(.*)')
      {
       $conf{'CFLAGS'}=$1;
      }
    elsif ($i=~'--cxxflags=(.*)' ||
           $i=~'--cppflags=(.*)')
      {
       $conf{'CXXFLAGS'}=$1;
      }
    elsif ($i eq '--debug')
      {
       $conf{'XCFLAGS'}=
       $conf{'XCXXFLAGS'}='-O3 -fomit-frame-pointer -ffast-math -gstabs+3';
       $conf{'CFLAGS'}=
       $conf{'CXXFLAGS'}='-O2 -Wall -Werror -gstabs+3';
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
    elsif ($i eq '--comp-exe')
      {
       $conf{'compressExe'}='yes';
      }
    elsif ($i eq '--no-comp-exe')
      {
       $conf{'compressExe'}='no';
      }
    elsif ($i eq '--with-mss')
      {
       $conf{'mss'}='yes';
      }
    elsif ($i eq '--without-mss')
      {
       $conf{'mss'}='no';
      }
    elsif ($i eq '--with-efence')
      {
       $conf{'efence'}='yes';
      }
    elsif ($i eq '--without-efence')
      {
       $conf{'efence'}='no';
      }
    elsif ($i=~'--x-include=(.*)')
      {
       $conf{'X11IncludePath'}=$1;
      }
    elsif ($i=~'--x-lib=(.*)')
      {
       $conf{'X11LibPath'}=$1;
      }
    elsif ($i eq '--with-aa')
      {
       $conf{'HAVE_AA'}='yes';
      }
    elsif ($i eq '--without-aa')
      {
       $conf{'HAVE_AA'}='no';
       $conf{'HAVE_AA_from_user'}='yes';
      }
    elsif ($i eq '--enable-maintainer-mode')
      {
       $conf{'MAINTAINER_MODE'}='yes';
      }
    elsif ($i eq '--source-bzip2')
      {
       $conf{'source-bzip2'}='yes';
      }
    elsif ($i eq '--just-spec')
      {
       $JustSpec=1;
      }
    elsif ($i eq '--with-calc')
      {
       $conf{'HAVE_CALCULATOR'}='yes';
      }
    elsif ($i eq '--without-calc')
      {
       $conf{'HAVE_CALCULATOR'}='no';
      }
    elsif ($i eq '--with-calend')
      {
       $conf{'HAVE_CALENDAR'}='yes';
      }
    elsif ($i eq '--without-calend')
      {
       $conf{'HAVE_CALENDAR'}='no';
      }
    elsif ($i=~'--include=(.*)')
      {
       $conf{'EXTRA_INCLUDE_DIRS'}.=" $1";
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
 print "Available options:\n";
 # Targets
 print "\nOptional targets:\n";
 print "--infview       : also compile InfView [default].\n";
 print "--no-infview    : don't compile InfView, just the editor.\n";
 print "--libset        : create libset, needed to compile RHIDE.\n";
 print "--no-libset     : don't create libset [default].\n";
 # Paths
 print "\nPaths:\n";
 print "--prefix=path   : defines the base directory for installation.\n";
 print "--no-prefix-h   : don't define the prefix in the configuration header\n";
 print "--fhs           : force the FHS layout under UNIX.\n";
 print "--no-fhs        : force to not use the FHS layout under UNIX.\n";
 print "--tv-include=pat: path for Turbo Vision includes\n";
 print "  Note: if you use --tv-include you should also use --tv-lib\n";
 print "--tv-lib=path   : path for Turbo Vision libraries\n";
 print "--include=path  : Add this path for includes. Repeat for each dir.\n";
 # Libs
 print "\nLibraries and optional support:\n";
 print "--no-intl       : don't use international support.\n";
 print "--shipped-intl  : force to use the shipped gettext library [DOS only]\n";
 print "--bzip2         : include support for bzip2.\n";
 print "--no-bzip2      : don't include support for bzip2 files.\n";
 print "--with-aa       : support for AA-lib [used for UNIX].\n";
 print "--without-aa    : without AA-lib support.\n";
 print "--parser-adv    : use the advanced parser for calc. [default]\n";
 print "--parser-brs    : use the small Burton's parser for calculator\n";
 print "--parser-ml     : use the small ML's parser for calculator\n";
 print "--without-mp3   : disable MP3 support\n";
 print "--with-amp      : use libamp for MP3 support [DOS only]\n";
 print "--with-mpegsnd  : use libmpegsnd for MP3 support [default]\n";
 print "--with-mixer    : include code to control the mixer [default]\n";
 print "--without-mixer : don't include code to control the mixer\n";
 print "--with-mss      : compiles with MSS memory debugger.\n";
 print "--without-mss   : compiles without MSS [default].\n";
 print "--with-efence   : compiles with Electric Fence memory debugger.\n";
 print "--without-efence: compiles without Electric Fence [default].\n";
 print "--with-calc     : compiles the internal calculator [default].\n";
 print "--without-calc  : no internal calculator.\n";
 print "--with-calend   : compiles the internal calendar [default].\n";
 print "--without-calend: no internal calendar.\n";
 # Compilation
 print "\nCompilation options:\n";
 print "--enable-maintainer-mode:\n";
 print "                : enables header dependencies and other stuff needed\n";
 print "                  for developement, not just use the editor.\n";
 print "--static        : force to create an statically linked executable.\n";
 print "                  Currently it only affect TV lib.\n";
 print "--dynamic       : generated a dynamically linked executable [default].\n";
 print "--cflags=val    : normal C flags [default is env. CFLAGS]\n";
 print "--cppflags=val  : normal C++ flags [default is env. CXXFLAGS]\n";
 print "--Xcflags=val   : special C flags used for MP3 libraries\n";
 print "--Xcppflags=val : special C++ flags used for MP3 libraries\n";
 print "--debug         : selects C/C++ switches for debugging\n";
 print "--comp-exe      : compress all executables with UPX\n";
 print "--no-comp-exe   : don't compress any executables with UPX\n";
 # Others
 print "\nOthers:\n";
 print "--source-bzip2  : use bzip2 for tarballs\n";
 print "--just-spec     : update RPMs spec file and exit.\n";

 print "\n";
 print "--help          : displays this text.\n";

 #print "--x-include=path: X11 include path [/usr/X11R6/lib].\n";
 #print "--x-lib=path    : X11 library path [/usr/X11R6/include].\n";
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
 if (@conf{'xgettext'} eq 'no')
   {
    print "* The 'xgettext' tools aren't installed internat. files can't be created.\n";
   }
 if (@conf{'makeinfo'} eq 'no')
   {
    print "* The 'makeinfo' tool isn't installed documentation can't be created.\n";
   }
 if (@conf{'makeinfo'} eq 'broken')
   {
    print "* The 'makeinfo' tool is an unsupported version (broken?).\n";
    print "  Do you want to help solving it? contact me.\n";
   }
 if (@conf{'GNU_Make'} ne 'make')
   {
    print "* Please use $conf{'GNU_Make'} instead of make command.\n";
   }
 if (($OS eq 'DOS') && (@conf{'ToolsInstaller'} eq 'no'))
   {
    print "* Some tools to create the installer aren't installed or are installed in a\n";
    print "  directory that I couldn't find. The installer target was disabled.\n";
   }
 if (@conf{'ToolsDistrib'} eq 'no')
   {
    print "* Some tools to create the distribution aren't installed. The distrib target\n";
    print "  was disabled.\n";
   }
 if ((@conf{'PCREShipped'} eq 'yes') && ($OS eq 'UNIX'))
   {
    print "* Using shipped PCRE lib, to avoid wasting system resources you should install\n";
    print "  the library and also the development package (i.e. libpcre3 and libpcre3-dev)\n";
   }
 if ((@conf{'zlibShipped'} eq 'yes') && ($OS eq 'UNIX'))
   {
    print "* Using shipped zlib, to avoid wasting system resources you should install\n";
    print "  the library and also the development package (i.e. libz-dev)\n";
   }
 if ((@conf{'bz2libShipped'} eq 'yes') && ($OS eq 'UNIX'))
   {
    print "* Using shipped bzip2 lib, to avoid wasting system resources you should install\n";
    print "  the library and also the development package (i.e. libbz2 and libbz2-dev)\n";
   }
 if ((@conf{'HAVE_AA'} eq 'no') && ($OS eq 'UNIX'))
   {
    print "* AA lib is not installed (or isn't functional) so you won't get a nice console\n";
    print "  screen saver\n";
   }
 if ((@conf{'HAVE_GDB_MI'} eq 'no') && ($OS eq 'UNIX'))
   {
    print "* NO DEBUG SUPPORT! The migdb library isn't available, debug features disabled.\n";
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
 $test=RunRedirect('xgettext --version',$ErrorLog);
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
 my ($test,$ver);

 print 'Looking for makeinfo: ';
 if (@conf{'makeinfo'} && (@conf{'makeinfo'} ne 'no') && (@conf{'makeinfo'} ne 'broken'))
   {
    print @conf{'makeinfo'}." (cached)\n";
    return;
   }
 $test=RunRedirect('makeinfo --version',$ErrorLog);
 if ($test=~/(\d+\.\d+(\.\d+)?)(.*)(\d+\.\d+(\.\d+)?)/)
   {
    $ver=$4;
   }
 elsif ($test=~/(\d+\.\d+(\.\d+)?)/)
   {
    $ver=$1;
   }
 else
   {
    print "no\n";
    $conf{'makeinfo'}='no';
    return;
   }
 print "$ver";
 if (CompareVersion($ver,$BrokenMakeinfo))
   {
    $conf{'makeinfo'}='broken';
    print " Broken!\n";
   }
 else
   {
    $conf{'makeinfo'}=$ver;
    print " OK\n";
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
    @dirsL=("$parent/tvision/makes",$ENV{'DJDIR'}.'/lib');
   }
 elsif ($OS eq 'UNIX')
   {
    @dirsI=("$parent/tvision/include",'/usr/include/rhtvision','/usr/local/include/rhtvision');
    @dirsL=("$parent/tvision/makes",'/usr/lib','/usr/local/lib');
   }
 elsif ($OS eq 'Win32')
   {
    @dirsI=("$parent/tvision/include",@conf{'prefix'}.'/include/tvision');
    @dirsL=("$parent/tvision/makes",@conf{'prefix'}.'/lib');
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
 $TVLib=LookForFile('librhtv*dylib',@dirsL) if (!length($TVLib));
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

sub LookForTVConfig
{
 my $test;

 print 'Looking for Turbo Vision config program: ';
 $test=RunRedirect('rhtv-config --version',$ErrorLog);
 if ($test=~/Turbo Vision/)
   {
    print "installed, OK\n";
    $tvConfig='rhtv-config';
   }
 else
   {
    $tvConfig=$TVInclude.'/../rhtv-config';
    $test=RunRedirect($tvConfig.' --version',$ErrorLog);
    if ($test=~/Turbo Vision/)
      {
       print "$tvConfig, OK\n";
      }
    else
      {
       print "\nError: Can't find the Turbo Vision configuration program.\n";
       print "Please try updating TV, then reconfigure TV and compile.\n";
       CreateCache();
       die "Missing tool\n";
      }
   }
}

sub TVConfigOption
{
 return `$tvConfig --$_[0]`;
}

sub LookForPCRE
{
 my ($test205,$test206,$t2,$test,$dir,$subpcre);

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
#ifdef SUBPCRE
 #include <pcre/pcre.h>
#else
 #include <pcre.h>
#endif
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
 $subpcre=0;
 $test=RunGCCTest($GCC,'c',$test206,"-lpcre");
 if ($test ne "OK\n")
   {
    $test=RunGCCTest($GCC,'c',$test206,"-lpcre -DSUBPCRE");
    $subpcre=1;
   }
 if ($test eq "OK\n")
   {
    print "v2.0.6 or better OK\n";
    $conf{'HAVE_PCRE_LIB'}='yes';
    $conf{'HAVE_PCRE206'}='yes';
    $conf{'PCREShipped'}='no';
    $conf{'PCRE_HEADER'}=$subpcre ? '<pcre/pcre.h>' : '<pcre.h>';
    return;
   }
 print 'no 2.0.6+, ';
 # See if 2.0+ is installed
 $subpcre=0;
 $test=RunGCCTest($GCC,'c',$test205,"-lpcre");
 if ($test ne "OK\n")
   {
    $test=RunGCCTest($GCC,'c',$test205,"-lpcre -DSUBPCRE");
    $subpcre=1;
   }
 if ($test eq "OK\n")
   {
    print "v2.0 or better OK\n";
    $conf{'HAVE_PCRE_LIB'}='yes';
    $conf{'HAVE_PCRE206'}='no';
    $conf{'PCREShipped'}='no';
    $conf{'PCRE_HEADER'}=$subpcre ? '<pcre/pcre.h>' : '<pcre.h>';
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
 $conf{'PCRE_HEADER'}='<pcre.h>';
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
 printf("%d.%d.%d",ALLEGRO_VERSION,ALLEGRO_SUB_VERSION,ALLEGRO_WIP_VERSION);
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
    print "\nSorry, not compatible with this Allegro version\n\n";
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
 my ($test,$ver);

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
}
';
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

sub LookForBZ2Lib
{
 my $vNeed=$_[0];
 my $test,$ver;

 print 'Looking for BZip2 library: ';
 $test=@conf{'bz2lib'};
 if ($test)
   {
    print "$test (cached) OK\n";
    return;
   }
 $test='
#include <stdio.h>
#include <bzlib.h>
int main(void)
{
 printf("%s",bzlibVersion());
 return 0;
}';
 # I saw version 1.0.1 on Solaris with a header having BZ2_* but accepting the
 # old names!? C++ won't let it happend.
 $ver=RunGCCTest($GXX,'cc',$test,'-lbz2');
 if (length($ver))
   {
    if (CompareVersion($ver,$vNeed))
      {
       print "$ver OK\n";
       $conf{'bz2libShipped'}='no';
       $conf{'bz2lib'}=$ver;
       $conf{'bz2libPre1'}='yes';
       $conf{'HAVE_BZIP2'}='yes';
       $conf{'HAVE_BZIP2PRE1'}='yes';
       return;
      }
   }
 else
   {
    $test='
#include <stdio.h>
#include <bzlib.h>
int main(void)
{
 printf("%s",BZ2_bzlibVersion());
 return 0;
}';
    $ver=RunGCCTest($GXX,'cc',$test,'-lbz2');
    if (length($ver))
      {
       if (CompareVersion($ver,$vNeed))
         {
           print "$ver OK\n";
           $conf{'bz2libShipped'}='no';
           $conf{'bz2lib'}=$ver;
           $conf{'bz2libPre1'}='no';
           $conf{'HAVE_BZIP2'}='yes';
           return;
         }
       print "no $vNeed+, ";
      }
    else
      {
       print 'not installed, ';
      }
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
 $conf{'bz2libShipped'}='yes';
 $conf{'bz2lib'}='shipped'; #$test
 $conf{'bz2libPre1'}='no';
 $conf{'HAVE_BZIP2'}='yes';
}

sub LookForDL
{
 my ($test,$ver,$header,$lib);

 print 'Looking for dl library: ';
 $test=@conf{'dl'};
 if ($test)
   {
    print "$test (cached) OK\n";
    return;
   }
 if ($OSf eq 'QNXRtP')
   {
    $lib='ltdl';
    $header='dlfcn';
   }
 else
   {
    $lib='dl';
    $header='link';
   }
 $test='
 #include <stdio.h>
 #include <'.$header.'.h>
 void test()
 {
  dlopen("test.o",0);
 }

 int main(void)
 {
  printf("OK");
  return 0;
 }';
 $ver=RunGCCTest($GCC,'c',$test,'-l'.$lib);

 if ($ver eq 'OK')
   {
    $conf{'dl'}='yes';
    $conf{'dl_header'}=$header;
   }
 else
   {
    $conf{'dl'}='no';
   }
 print "$conf{'dl'}\n";
}


sub LookForIntlSupport
{
 my $vNeed=$_[0];
 my ($test,$a,$djdir,$intllib,$intltest,$libdir);

 print 'Checking for international support: ';
 $conf{'intlShipped'}='no';
 $conf{'tvfintl'}='no';
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
}
';
 $intllib=(($OS eq 'DOS') || ($OS eq 'Win32') || ($OSf eq 'FreeBSD') || ($OSf eq 'QNXRtP')) ? '-lintl' : '';
 $libdir=$LDExtraDirs;
 $libdir=~s/(\S+)/-L$1/g;
 $test=RunGCCTest($GCC,'c',$intltest,"-I$TVInclude ".$libdir.' '.$intllib);
 if ($test ne "OK\n")
   {
    print "no, additional check required.\n";
    print "Checking for extra libs for international support: ";
    $test=RunGCCTest($GCC,'c',$intltest,"-I$TVInclude ".$intllib.' -liconv');
    if ($test ne "OK\n")
      {
       if ($OS eq 'DOS')
         {
          print "none found, using shipped one\n";
          $conf{'intl'}='yes';
          $conf{'iconv'}='no';
          $conf{'intlShipped'}='yes';
         }
       else
         {
          $test=RunGCCTest($GCC,'c',$intltest,"-I$TVInclude -L$TVLib -ltvfintl");
          if ($test ne "OK\n")
            {
             print "not found\n";
             $conf{'intl'}='no';
             $conf{'iconv'}='no';
            }
          else
            {
             print "not found, using fake dummy version\n";
             $conf{'intl'}='no';
             $conf{'iconv'}='no';
             $conf{'tvfintl'}='yes';
            }
         }
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
 my ($a,$old);
 my $text="/* Generated automatically by the configure script */";

 print "Generating configuration header: ";

 $conf{'FORCE_INTL_SUPPORT'}=$conf{'intlShipped'};
 $conf{'HAVE_DL_LIB'}=$conf{'dl'};
 $text.=ConfigIncDefYes('HAVE_ALLEGRO','Allegro library is available');
 $text.=ConfigIncDefYes('WITH_MP3','Enable MP3 support');
 $text.=ConfigIncDefYes('HAVE_AMP','MP3 support from libamp');
 $text.=ConfigIncDefYes('HAVE_MPEGSOUND','MP3 support from libmpegsound');
 $text.=ConfigIncDefYes('HAVE_PCRE_LIB','Perl Compatible Regular Expressions support');
 $text.=ConfigIncDefYes('HAVE_PCRE206','PCRE version 2.0.6 or newer');
 $text.=ConfigIncDefYes('HAVE_BZIP2','bzip2 compression support');
 $text.=ConfigIncDefYes('HAVE_BZIP2PRE1','old bzip2 version before 1.0') if(@conf{'HAVE_BZIP2'} eq 'yes');
 $text.=ConfigIncDefYes('HAVE_MIXER','Sound mixer support');
 $text.=ConfigIncDefYes('FORCE_INTL_SUPPORT','Gettext included with editor');
 #$text.=ConfigIncDefYes('HAVE_X11','X11 library and headers');
 $text.=ConfigIncDefYes('HAVE_AA','AA lib');
 $text.=ConfigIncDefYes('HAVE_GDB_MI','GDB/MI interface');
 $text.=ConfigIncDefYes('HAVE_CALCULATOR','Calculator');
 $text.=ConfigIncDefYes('HAVE_CALENDAR','Calendar');
 $text.=ConfigIncDefYes('HAVE_DL_LIB','Support for runtime dynamic libs');
 $text.="\n#define DL_HEADER_NAME <".$conf{'dl_header'}.".h>\n";
 $text.="#define PCRE_HEADER_NAME ".$conf{'PCRE_HEADER'}."\n";

 $text.="\n\n#define CONFIG_PREFIX \"";
 $a=$conf{'prefix'};
 $a=~s/\\/\\\\/g;
 $text.=$a unless $conf{'no-prefix-h'};
 $text.="\"\n";

 $text.="\n\n";
 $text.="#define SEOS_$OS\n";
 $text.="#define SEOS_STR \"$OS\"\n";
 $text.="#define SEOSf_$OSf\n";
 $text.="#define SEOSf_STR \"$OSf\"\n" if $OSf;
 $text.="#define SECPU_$CPU\n";
 $text.="#define SECPU_STR \"$CPU\"\n";
 $text.="#define SEComp_$Comp\n";
 $text.="#define SEComp_STR \"$Comp\"\n";
 $text.="#define SECompf_$Compf\n";
 $text.="#define SECompf_STR \"$Compf\"\n" if $Compf;
 $text.="\n#define MSS\n#include <mss.h>\n" if @conf{'mss'} eq 'yes';

 $old=cat('include/configed.h');
 if ($text eq $old)
   {
    print "no changes\n";
   }
 else
   {
    print "created new header\n";
    replace('include/configed.h',$text);
   }
}

sub GenerateMakefile
{
 my $text="# Generated automatically by the configure script";
 my ($libamp,$libset,$infview,$libbzip2,$libmpegsnd,$libz,$libpcre,$libintl);
 my ($installer,$distrib,$compExeEditor,$compExeInfview,$holidays,$mantmode);
 my ($aux,$extraIns,$extraInsVar,$libmigdb);

 print "Generating Makefile\n";

 # Give more priority to "prefix" than hardcoded value
 $mantmode=@conf{'MAINTAINER_MODE'} eq 'yes';
 $text.="\n\nifneq (\$(strip \$(prefix)),)\n";
 $text.="  MPREFIX=\$(prefix)\n";
 $text.="else\n";
 $text.="  MPREFIX=$conf{'prefix'}\n";
 $text.="endif\n";
 $text.="ifeq (\$(INSTALL),)\n";
 $text.="  INSTALL=@conf{'GNU_INSTALL'}\n";
 $text.="endif";
 $text.="\nlibdir=\$(MPREFIX)/lib";
 $text.="\nCFLAGS=$conf{'CFLAGS'}";
 $text.="\nCXXFLAGS=$conf{'CXXFLAGS'}";
 $text.="\nSET_USE_FHS=$conf{'fhs'}" if ($OS eq 'UNIX');
 $text.="\nMAINTAINER_MODE=1" if $mantmode;
 $text.="\nexport";

 #### Targets ####
 $libset=@conf{'libset'} eq 'yes';
 $libamp=@conf{'HAVE_AMP'} eq 'yes';
 $libmpegsnd=@conf{'HAVE_MPEGSOUND'} eq 'yes';
 $infview=@conf{'infview'} eq 'yes';
 $libbzip2=@conf{'bz2libShipped'} eq 'yes';
 $libz=@conf{'zlibShipped'} eq 'yes';
 $libpcre=@conf{'PCREShipped'} eq 'yes';
 $libintl=@conf{'intlShipped'} eq 'yes';
 $plasmas=$OS eq 'DOS';
 $installer=@conf{'ToolsInstaller'} eq 'yes';
 $distrib=@conf{'ToolsDistrib'} eq 'yes';
 $internac=@conf{'xgettext'} ne 'no';
 $docbasic=(@conf{'makeinfo'} ne 'no') && (@conf{'makeinfo'} ne 'broken');
 # TODO: I need to replace -soname by -h conditionaly like in TV.
 $holidays=(@conf{'dl'} eq 'yes') && ($OSf ne 'Solaris');
 $libmigdb=@conf{'migdbShipped'} eq 'yes';

 if (@conf{'compressExe'} eq 'undef')
   {# Default is to compress InfView and the editor only for non-UNIX targets
    $compExeEditor=$OS ne 'UNIX';
    $compExeInfview=1;
   }
 else
   {# The user specified an option
    $compExeEditor=@conf{'compressExe'} eq 'yes';
    $compExeInfview=@conf{'compressExe'} eq 'yes';
   }
 
 $text.="\n\n.PHONY: needed";
 $text.=" infview"    if ($infview);
 $text.=" plasmas"    if ($plasmas);
 $text.=" libbzip2"   if ($libbzip2);
 $text.=" libz"       if ($libz);
 $text.=" libmpegsnd" if ($libmpegsnd);
 $text.=" libpcre"    if ($libpcre);
 $text.=" libamp"     if ($libamp);
 $text.=" libintl"    if ($libintl);
 $text.=" installer"  if ($installer);
 $text.=" internac"   if ($internac);
 $text.=" doc-basic"  if ($docbasic);
 $text.=" holidays"   if ($holidays);
 $text.=" libmigdb"   if ($libmigdb);
 # all targets
 $text.="\n\nall: Makefile editor";
 $text.=" libset"    if ($libset);
 $text.=" infview"   if ($infview);
 $text.=" plasmas"   if ($plasmas);
 $text.=" installer" if ($installer);
 $text.="\n";

 $text.="\n\nMakefile: config.pl conflib.pl\n";
 $text.="\t\$(error Please reconfigure the package! Alternative: \"touch Makefile\")";

 # libamp
 if ($libamp)
   {
    $text.="\n\nlibamp:\n";
    $text.="\t\$(MAKE) -C mp3/libamp -f libamp.mkf";
   }
 # libmpegsnd
 if ($libmpegsnd)
   {
    $text.="\n\nlibmpegsnd:\n";
    $text.="\t\$(MAKE) -C mp3/mpegsound -f mpegsnd.mkf";
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
 # libmigdb
 if ($libmigdb)
   {
    $text.="\n\nlibmigdb:\n";
    $text.="\t\$(MAKE) -C libmigdb";
   }
 # i8n
 if ($internac)
   {
    $text.="\n\ninternac:\n";
    $text.="\t\$(MAKE) -C internac";
   }
 if ($docbasic)
   {
    $text.="\n\ndoc-basic:\n";
    $text.="\t\$(MAKE) -C doc txt info";
   }
 # needed (by editor)
 $text.="\n\n# Libraries not created by RHIDE projects\nneeded:";
 $text.=" libamp"     if ($libamp);
 $text.=" libmpegsnd" if ($libmpegsnd);
 $text.=" libbzip2"   if ($libbzip2);
 $text.=" libz"       if ($libz);
 $text.=" libpcre"    if ($libpcre);
 $text.=" libintl"    if ($libintl);
 $text.=" holidays"   if ($holidays);
 $text.=" libmigdb"   if ($libmigdb);
 $text.=" include/vername.h" if ($mantmode);
 #
 # MinGW tools I tested are broken and can't generate these targets
 #
 $text.=" internac"  if ($internac) && ($Compf ne 'MinGW');
 $text.=" doc-basic" if ($docbasic) && ($Compf ne 'MinGW');
 # editor
 $text.="\n\neditor: needed";
 $text.="\n\t\$(MAKE) -C makes";
 # libset
 if ($libset)
   {
    $text.="\n\nlibset:\n";
    $text.="\t\$(MAKE) -C makes libset";
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
 # installer
 if ($installer)
   {
    $text.="\n\ninstaller: editor\n";
    $text.="\t\$(MAKE) -C makes installer";
   }
 # holidays plug-ins
 if ($holidays)
   {
    $text.="\n\nholidays:\n";
    $text.="\t\$(MAKE) -C holidays";
   }
 # version name and revision header
 if ($mantmode)
   {
    $text.="\n\ninclude/vername.h: change.log\n";
    $text.="\tperl updaterev.pl";
   }

 $extraIns='';
 # Don't compress executables
 $extraIns.='--no-compress ' unless ($compExeEditor);
 # Sources tarball compressed with bzip2
 $extraIns.='--use-bzip2 '   if $conf{'source-bzip2'} eq 'yes';
 # .exe extension:
 # DOS uses a special script where the extension remains.
 # POSIX systems don't, but Cygwin uses the POSIX script.
 $extraIns.='--keep-extension ' if ($OS eq 'Win32');
 $extraInsVar='';
 $extraInsVar=" \"EXTRA_INS_OPS=$extraIns\"" if $extraIns;
 #### Installations ####
 # editor
 $text.="\n\ninstall-editor: editor\n";
 $text.="\t\$(MAKE) -C makes install";
 $text.=$extraInsVar;
 # libset
 if ($libset)
   {
    $text.="\n\ninstall-libset: libset\n";
    $text.="\t".GenInstallDir('0755','$(libdir)');
    $text.="\t".GenInstallFiles('0644','makes/libset.a','$(libdir)');
   }
 # infview
 if ($infview)
   {
    $text.="\n\ninstall-infview: infview\n";
    $text.="\t\$(MAKE) -C makes install-infview";
    $text.=$extraInsVar;
   }
 # all targets
 $text.="\n\ninstall: install-editor";
 $text.=" install-libset" if ($libset);
 $text.=" install-infview" if ($infview);
 $text.="\n";

 if ($distrib)
   {
    #### Distribution ####
    # editor
    $text.="\n\ndistrib-editor: needed\n";
    $text.="\t\$(MAKE) -C makes distrib $extraInsVar";
    # just sources
    $text.="\n\ndistrib-source:\n";
    $text.="\t\$(MAKE) -C makes distrib-source $extraInsVar";
    # infview
    if ($infview)
      {
       $text.="\n\ndistrib-infview: needed\n";
       $text.="\t\$(MAKE) -C makes distrib-infview $extraInsVar";
      }
    # all targets
    $text.="\n\ndistrib: distrib-editor";
    $text.=" distrib-infview" if ($infview);
    $text.="\n";
   }

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
 $text.="\trm -f mp3/mpegsound/*.a mp3/mpegsound/obj/*.o\n";
 $text.="\tcd scrnsave; \$(MAKE) clean-o; cd ..\n" if ($plasmas);
 $text.="\t\$(MAKE) -C libbzip2 clean\n" if ($libbzip2);
 $text.="\t\$(MAKE) -C libz clean\n" if ($libz);
 $text.="\t\$(MAKE) -C libpcre clean\n" if ($libpcre);
 $text.="\t\$(MAKE) -C holidays clean\n" if ($holidays);
 $text.="\t\$(MAKE) -C libmigdb clean\n" if ($libmigdb);

 replace('Makefile',$text);
}

sub LookForGPM
{
 my $vNeed=$_[0],$test;

 print 'Looking for gpm library: ';
 if (@conf{'gpm'})
   {
    print "@conf{'gpm'} (cached) OK\n";
    return;
   }
 $test='
#include <stdio.h>
#include <gpm.h>
int main(void)
{
 int version;
 printf("%s",Gpm_GetLibVersion(&version));
 return 0;
}
';
 $test=RunGCCTest($GCC,'c',$test,'-lgpm');
 if (!length($test))
   {
    #print "\nError: gpm library not found, please install gpm $vNeed or newer\n";
    #print "Look in $ErrorLog for potential compile errors of the test\n";
    #CreateCache();
    #die "Missing library\n";
    $conf{'HAVE_GPM'}='no';
    print " no, disabling mouse support\n";
    return;
   }
 if (!CompareVersion($test,$vNeed))
   {
    #print "$test, too old\n";
    #print "Please upgrade your gpm library to version $vNeed or newer.\n";
    #print "You can try with $test forcing the configure scripts.\n";
    #CreateCache();
    #die "Old library\n";
    $conf{'HAVE_GPM'}='no';
    print " too old, disabling mouse support\n";
    return;
   }
 $conf{'gpm'}=$test;
 $conf{'HAVE_GPM'}='yes';
 print "$test OK\n";
}

sub LookForToolsInstaller
{
 my ($list,$i,$test);

 print 'Tools for Installer:';
 if ($conf{'ToolsInstaller'} eq 'yes')
   {
    print " yes (cached)\n";
    return;
   }
 # Allegro, already tested
 if ($conf{'HAVE_ALLEGRO'} ne 'yes')
   {
    print " no Allegro library\n";
    return;
   }
 # PCRE, already tested
 if ($conf{'HAVE_PCRE_LIB'} ne 'yes')
   {
    print " no PCRE library\n";
    return;
   }
 # Various programs
 @list=('cwsdpmi.exe','cwsdpmi.doc','emu387.dxe',
        'pmodstub.exe','exedat.exe','dat.exe','groff.exe');
 foreach $i (@list)
   {
    print " $i";
    if (!(-e $ENV{'DJDIR'}.'/bin/'.$i))
      {
       print " no\n";
       return;
      }
   }
 # zip
 print ' zip';
 $test=RunRedirect('zip -h');
 if (!($test=~/zip/))
   {
    print " no\n";
    return;
   }
 # upx
 print ' upx';
 $test=RunRedirect('upx -V');
 if (!($test=~/upx/))
   {
    print " no\n";
    return;
   }
 # Libwin
 print ' libwin';
 $test='
#include <stdio.h>
#include <libwin.h>
void dummy(void) {
long hKey;
w95_reg_openkey(HKEY_LOCAL_MACHINE,"SOFTWARE",&hKey); }
int main(void)
{
 printf("Ok\n");
 return 0;
}
';
 $test=RunGCCTest($GCC,'c',$test,'-lwin');
 chop($test);
 if ($test ne 'Ok')
   {
    print " no\n";
    return;
   }
 print " OK!\n";
 $conf{'ToolsInstaller'}='yes';
}

sub LookForToolsDistrib
{
 my ($test);

 print 'Tools for Distrib:';
 if ($conf{'ToolsDistrib'} eq 'yes')
   {
    print " yes (cached)\n";
    return;
   }
 if ($OS eq 'UNIX')
   {# Should I test with other switches?
    # And the compressor? check for gzip?
    # tar
    print ' tar';
    `tar --help > test.txt 2>&1`;
    $test=cat('test.txt');
    unlink 'test.txt';
    if (!($test=~/tar/))
      {
       print " no\n";
       return;
      }
    print ' gzip';
    `gzip --help > test.txt 2>&1`;
    $test=cat('test.txt');
    unlink 'test.txt';
    if (!($test=~/gzip/))
      {
       print " no\n";
       return;
      }
   }
  else
    {
     # zip
     print ' zip';
     $test=RunRedirect('zip -h');
     if (!($test=~/zip/))
       {
        print " no\n";
        return;
       }
    }
 print " OK\n";
 $conf{'ToolsDistrib'}='yes';
}

sub LookForBasicTools
{
 my $test;
 if (($OS eq 'DOS') || ($OS eq 'Win32'))
   {
    #$test=RunRedirect('rm --version');
    $test=RunRedirect('cp --version');
    if (!($test=~/fileutils/))
      {
       print "Please install the fileutils package. The name is usually something like it:\n";
       print "filXXXb.zip where XXX is the version.\n";
       die "\n";
      }
   }
}

sub LookForXlib()
{
 my ($test,$o,$libs);

 print 'Looking for X11 libs: ';
 if (@conf{'HAVE_X11'})
   {
    print "@conf{'HAVE_X11'} (cached)\n";
    return;
   }
 $test='
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
Display *Test()
{ return XOpenDisplay(""); }
int main(void)
{
 printf("OK, %d.%d\n",X_PROTOCOL,X_PROTOCOL_REVISION);
 return 0;
}
';
 $conf{'X11LibPath'}='/usr/X11R6/lib' unless $conf{'X11LibPath'};
 $conf{'X11Lib'}='X11 Xmu' unless $conf{'X11Lib'};
 $libs=$conf{'X11Lib'};
 $libs=~s/(\S+)/-l$1/g;
 $o='';
 $o.='-I'.$conf{'X11IncludePath'} if $conf{'X11IncludePath'};
 $o.=" -L$conf{'X11LibPath'} $libs";
 $test=RunGCCTest($GCC,'c',$test,$o);
 if ($test=~/OK, (\d+)\.(\d+)/)
   {
    $conf{'HAVE_X11'}='yes';
    print "yes OK (X$1 rev $2)\n";
   }
 else
   {
    if (!$conf{'X11IncludePath'})
      {
       $conf{'X11IncludePath'}='/usr/X11R6/include';
       $o.="-I$conf{'X11IncludePath'} -L$conf{'X11LibPath'} $libs";
       if ($test=~/OK, (\d+)\.(\d+)/)
         {
          $conf{'HAVE_X11'}='yes';
          print "yes OK (X$1 rev $2)\n";
          return;
         }
      }
    $conf{'HAVE_X11'}='no';
    print "no, disabling X11 version\n";
   }
}

sub LookForAA
{
 my ($test);

 print 'Looking for AA library: ';
 $test=$conf{'HAVE_AA'};
 if (($test eq 'yes') || ($conf{'HAVE_AA_from_user'} eq 'yes'))
   {
    print "$test ";
    print (($conf{'HAVE_AA_from_user'} eq 'yes') ? "(user)" : "(cached)");
    print " OK\n";
    return;
   }
 $test='
 #include <stdio.h>
 #include <aalib.h>
 int main(void)
 {
  if (aa_autoinit(&aa_defparams))
     printf("OK\n");
  return 0;
 }';
 # AA lib v1.2 SuSE SLES 8 doesn't work for C++ (wrong typedef for struct)
 $test=RunGCCTest($GXX,'cc',$test,'-laa');
 $conf{'HAVE_AA'}=($test=~/OK$/) ? 'yes' : 'no';

 print "$conf{'HAVE_AA'}\n";
}

sub LookForMI()
{
 my $vNeed=$_[0];
 my ($ver,$test);

 print 'Looking for GDB/MI library: ';
 $ver=$conf{'HAVE_GDB_MI'};
 if ($ver)
   {
    print "$ver (cached) OK\n";
    return;
   }
 $conf{'migdbShipped'}='no';
 $test='
 #include <stdio.h>
 #include <mi_gdb.h>
 int main(void)
 {
  mi_set_gdb_exe("none");
  printf("%s\n",MI_VERSION_STR);
  return 0;
 }';
 $ver=RunGCCTest($GCC,'c',$test,'-lmigdb');
 chop($ver);
 if (length($ver))
   {
    if (CompareVersion($ver,$vNeed))
      {
       print "$ver OK\n";
       $conf{'HAVE_GDB_MI'}='yes';
       return;
      }
    print "no $vNeed+";
   }
 else
   {
    print "not installed";
   }
 if (-d 'libmigdb')
   {
    print ", using shipped one\n";
    $conf{'migdbShipped'}='yes';
    $conf{'HAVE_GDB_MI'}='yes';
    return;
   }
 print "\n";
 $conf{'HAVE_GDB_MI'}='no';
}

