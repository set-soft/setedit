#!/usr/bin/perl
# Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
# see copyrigh file for details
#

require "miscperl.pl";
require "conflib.pl";

SeeCommandLine();

$col=14;
$a=ExtractItemsMak('makes/editor.mak',$col);
$a=~s/mixersb\.c//;
$a=~s/mixoss\.c//;
$a=~s/djmdr.s//;
$a=~s/memmove\.s//;
$b=$a;
$a=~s/(\w+)\.(\w+)/\+$1\.obj/g;

$ReplaceTags{'EDITOR_OBJS'}=$a;

$b=~s/(\w+)\.(\w+)/\$\(OBJDIR\)\/$1\.o/g;
$ReplaceTags{'EDITOR_OBJS_MS'}=$b;
$b=0;

$col=14;
$a=ExtractItemsMak('makes/easydiag.mak',$col);
$a=~s/(\w+)\.(\w+)/\+$1\.obj/g;
$ReplaceTags{'EASYDIAG_OBJS'}=$a;

$col=14;
$a=ExtractItemsMak('makes/librhuti.mak',$col);
$a=~s/(\w+)\.(\w+)/\+$1\.obj/g;
$ReplaceTags{'LIBRHUTI_OBJS'}=$a;

$col=14;
$a=ExtractItemsMak('makes/libset.mak',$col);
$a=~s/(\w+)\.(\w+)/\+$1\.obj/g;
$ReplaceTags{'LIBSET_OBJS'}=$a;

$col=14;
$a=ExtractItemsMak('makes/settv.mak',$col);
$a=~s/(\w+)\.(\w+)/\+$1\.obj/g;
$ReplaceTags{'SETTVUTI_OBJS'}=$a;

$ReplaceTags{'DEST_BIN_DIR'} = $conf{'bindir'};
$ReplaceTags{'DEST_SET_DIR'} = $conf{'setdir'};

$ReplaceTags{'DYNRTL'} = $conf{'dynrtl'} ? "DYNRTL = 1" : "";

ReplaceText('WinNT/bccmake.in','WinNT/Makefile');
ReplaceText('WinNT/msvcmake.in','WinNT/Makefile.nmk');
$a='';

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
    elsif ($i=~'--bindir=(.*)')
      {
       $conf{'bindir'}=$1;
      }
    elsif ($i=~'--setdir=(.*)')
      {
       $conf{'setdir'}=$1;
      }
    elsif ($i eq '--with-dynrtl')
      {
       $conf{'dynrtl'} = 1;
      }
    else
      {
       ShowHelp();
       die "Unknown option: $i\n";
      }
   }
   $conf{'bindir'} = 'c:\setedit' if $conf{'bindir'} eq '';
   $conf{'setdir'} = 'c:\setedit' if $conf{'setdir'} eq '';
}

sub ShowHelp
{
 print "Available options:\n\n";
 print "--help         : displays this text.\n";
 print "--bindir=path  : defines the directory where *.exe files will be installed.\n";
 print "--setdir=path  : defines the directory where documentation andsupport files\n" .
       "                 will be installed.\n";
 print "--with-dynrtl  : compile with DLL runtime.\n";
}
