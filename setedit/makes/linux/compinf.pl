#!/usr/bin/perl
# Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
open(FIL,'../../infview/version.txt') || return 0;
$version=<FIL>;
chop($version);
# Cygwin's Perl bug workaround
$version=~ s/\r//;
close(FIL);
$version1=$version;
$version1=~ s/\.//g;

$r=cat('../../infview/infmain.cc');
if ($r!~/$version/)
  {
   die "Error! inconsistent version in infmain.cc\n";
  }

# Default prefix
$prefix='/usr';
# Environment prefix
$i=$ENV{'prefix'};
if (length($i))
  {
   $prefix=$i;
  }
else
  {
   $i=$ENV{'PREFIX'};
   if (length($i))
     {
      $prefix=$i;
     }
  }

# Parse command line
$nextisprefix=0; $nextisfhs=0;
$iMode=0;  # Installation mode
$iCompressExe=1;
$useFHS=0;
$ExeExtension='';
foreach $i (@ARGV)
  {
   if ($nextisprefix)
     {
      $prefix=$i;
      $nextisprefix=0;
     }
   elsif ($nextisfhs)
     {
      $useFHS=$i eq 'yes';
      $nextisfhs=0;
     }
   elsif ($i eq '--prefix')
     {
      $nextisprefix=1;
     }
   elsif ($i eq '--fhs')
     {
      $nextisfhs=1;
     }
   elsif ($i eq '--install')
     {
      $iMode=1;
     }
   elsif ($i eq '--no-compress')
     {
      $iCompressExe=0;
     }
   elsif ($i eq '--keep-extension')
     {
      $ExeExtension='.exe';
     }
   else
     {
      print "Unknown command line option: $i\n";
      print "Usage: compres.pl [--prefix path] [--install] [--no-compress]\n\n";
     }
  }

# Alternative prefix used for examples in INSTALL
if ($prefix eq '/usr/local')
  {
   $prefix_alt='/usr';
  }
else
  {
   $prefix_alt='/usr/local';
  }

# Check for make
$test=`make --version 2> /dev/null`;
if ($test=~/GNU Make/)
  {
   $Make='make';
  }
else
  {
   $test=`gmake --version 2> /dev/null`;
   if ($test=~/GNU Make/)
     {
      $Make='gmake';
     }
   else
     {
      die "Where is make!\n";
     }
  }

# Adjust install tool
$os=`uname`;
if ($os=~/SunOS/)
  { # Solaris install is quite different
   $inst_bin='$(INSTALL_BIN) -f $(@D) $<';
   $inst_data='$(INSTALL_DATA) -f $(@D) $<';
  }
else
  {
   $inst_bin='$(INSTALL_BIN) $< $@';
   $inst_data='$(INSTALL_DATA) $< $@';
  }
  
# Check for gzip
$i=`which gzip`;
if (!length($i))
  {
   print "\n************* Attention!! *************\n\n";
   print "You must install gzip to compress the documentation\n";
   print "Skipping compression, if you want to stop press ^C, if not press ENTER\n";
   <STDIN>;
  }

# Update the makefiles if needed
if (!$iMode)
  {
   print "Creating makefile: ";
   #system('cp -p ../linux.env ../rhide.env');
   chdir('..');
   system($Make.' makes');
   chdir('linux');
   print "done.\n\n";
  }

# Create the distribution tree if needed
print "Creating directories: ";
$base="infview-$version";
$bin_dir=$base.'/bin';
$cfg_dir1=$base.'/share';
$cfg_dir=$cfg_dir1.'/infview';

$Locale2  =$cfg_dir1.'/locale';
$spLocale1=$Locale2.'/es';
$spLocale =$spLocale1.'/LC_MESSAGES';
$deLocale1=$Locale2.'/de';
$deLocale =$deLocale1.'/LC_MESSAGES';

$baseFHS=$base;
$baseFHS.='/share' if $useFHS;
$doc_dir0=$baseFHS;
$doc_dir0.='/share' if ($os=~/FreeBSD/ && !$useFHS);
$doc_dir1=$doc_dir0.'/doc';
$doc_dir =$doc_dir1.'/infview';
$man_dir1=$baseFHS.'/man';
$man_dir=$man_dir1.'/man1';
$inf_dir=$baseFHS.'/info';

@tree=(
$base,
$bin_dir,
$cfg_dir1,
$cfg_dir,
$inf_dir,
$doc_dir0,
$doc_dir1,
$doc_dir,
$man_dir1,
$man_dir,
$cfg_dir2,
$cfg_dir3,
$Locale2,
$spLocale1,$spLocale,
$deLocale1,$deLocale,
'resultInf');

foreach $i (@tree)
  {
   if (!(-d $i))
     {
      mkdir($i,00755);
      print "$i\n";
     }
  }
print "done.\n\n";


print "Copying the exe: ";
$d=$bin_dir.'/infview'.$ExeExtension;
$o='../infview.exe';
if (!(-e $d) or (-M $d > -M $o))
  {
   system("cp -p $o .");
   system("zip -9u resultInf/infview-$version-Linux-debug.zip infview.exe") unless($iMode);
   system('strip infview.exe');
   $i=`which upx`;
   if (length($i))
     {
      system('upx -9 infview.exe') unless $iCompressExe==0;
     }
   system("mv infview.exe $d");
  }
print "done.\n\n";


print "Copying doc files: ";
chdir('../../doc');
$i=`which makeinfo`;
if (!length($i))
  {
   print "\n************* Attention!! *************\n\n";
   print "You must install the makeinfo package to generate the docs\n";
   print "Skipping the documentation, you can create it latter, now press ENTER\n";
   <STDIN>;
  }
else
  {
   if (system($Make.' txt info')==0)
     {
      CopyIfCpr('infeng.inf','../makes/linux/'.$inf_dir.'/infview.info');
      CopyIfCpr('infeng.txt','../makes/linux/'.$doc_dir.'/infview.txt');
      CopyIfCpr('infview.man','../makes/linux/'.$man_dir.'/infview.1');
     }
   else
     {
      print "\n************* Attention!! *************\n\n";
      print "Failed to generate the docs\n";
      print "Skipping the documentation, you can create it latter, now press ENTER\n";
      <STDIN>;
     }
  }
chdir('..');
CopyIf('copyrigh','makes/linux/'.$doc_dir.'/copyrigh');
CopyIf('copying.dj','makes/linux/'.$doc_dir.'/copying.dj');
CopyIf('copying.gpl','makes/linux/'.$doc_dir.'/copying.gpl');
CopyIf('copying.lgp','makes/linux/'.$doc_dir.'/copying.lgp');
CopyIf('copying.rh','makes/linux/'.$doc_dir.'/copying.rh');
chdir('makes/linux');
CopyIfRpl('../../distrib/distrib3.txt',$doc_dir.'/readme.1st');
CopyIfRpl('../../distrib/distrib3.txt',$cfg_dir.'/readme.1st');
CopyIf('../../internac/es.mo',$spLocale.'/setedit.mo');
CopyIf('../../internac/de.mo',$deLocale.'/setedit.mo');
print "done.\n\n";


print "Copying other files: ";
@fext=(
'INSTALL.LINUX',
'VCSA.SH',
'infREMOVE_UNNEEDED',
'infINSTALL.MAK'
);
# Here we say which files changes their name
%frep=(
'INSTALL.LINUX' => 'INSTALL',
'infREMOVE_UNNEEDED' => 'REMOVE_UNNEEDED',
'infINSTALL.MAK' => 'INSTALL.MAK'
);
foreach $i (@fext)
  {
   $r=$frep{$i};
   $r=$i if !$r;
   print $i.' ' if (CopyIfRpl('../../distrib/'.$i,$base.'/'.$r));
  }
print "done.\n\n";
chmod 0755, $base.'/INSTALL';

if ($iMode)
  {
   chdir($base);
   system('./INSTALL');
   chdir('..');
   print "End of installation\n";
  }
else
  {
   print "Compressing the files: ";
   system('tar cvf - '.$base.' | gzip -c > resultInf/infview-'.$version.'.bin.i386.elf.static.linux.tar.gz ');
   print "done.\n";
   
   CopyIfRpl('../../distrib/distrib3.txt','resultInf/readme.1st');
   
   chdir('setedit/makes/linux');
  }
0;


sub cat
{
 local $/;
 my $b;

 open(FIL,$_[0]) || return 0;
 $b=<FIL>;
 close(FIL);

 $b;
}


sub replace
{
 my $b=$_[1];

 open(FIL,">$_[0]") || return 0;
 print FIL ($b);
 close(FIL);
}


sub CopyIf
{
 my ($o,$d)=@_;

 if (!(-e $d) or (-M $d > -M $o))
   {
    system("cp -p $o $d");
    return 1;
   }
 0;
}

sub CopyIfCpr
{
 my ($o,$d)=@_;

 if (!(-e $d.'.gz') or (-M $d.'.gz' > -M $o))
   {
    system("rm -f $d.gz");
    system("cp -p $o $d");
    system("gzip -9 $d");
    return 1;
   }
 0;
}

sub CopyIfRpl
{
 my ($o,$d)=@_;
 my $a;

 if (!(-e $d) or (-M $d > -M $o))
   {
    $a=cat($o);
    $a =~ s/\@\@v\@\@/$version/g;
    $a =~ s/\@\@v1\@\@/$version1/g;
    $a =~ s/\@\@pref\@\@/$prefix/g;
    $a =~ s/\@\@pref_alt\@\@/$prefix_alt/g;
    $a =~ s/\@\@install_bin\@\@/$inst_bin/g;
    $a =~ s/\@\@install_data\@\@/$inst_data/g;
    $a =~ s/\@\@make\@\@/$Make/g;
    replace($d,$a);
    if (-x $o)
      {
       chmod(0755,$d);
      }
    return 1;
   }
 0;
}


