#!/usr/bin/perl
# Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
open(FIL,'../../version.txt') || return 0;
$version=<FIL>;
chop($version);
close(FIL);
$version1=$version;
$version1=~ s/\.//g;

# Adjust Debian version
$r=cat('../../debian/changelog');
if ($r!~/$version/)
  {
   chdir('../../debian');
   @ENV{'DEBEMAIL'}='salvador@inti.gov.ar';
   @ENV{'DEBFULLNAME'}='Salvador E. Tropea (SET)';
   system("dch -v$version"."-0.1 \"New version automatically modified by version update scripts\"");
   chdir('../makes/linux');
  }

$r=cat('../../include/ced_inte.h');
if ($r!~/$version/)
  {
   die "Error! inconsistent version in ced_inte.h\n";
  }
$r=0;

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
$Strip=1;
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
   elsif ($i eq '--no-strip')
     {
      $Strip=0;
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

@files=('../../distrib/distrib1.txt','../../distrib/distrib2.txt');

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
$base="setedit-$version";
$bin_dir=$base.'/bin';
$cfg_dir1=$base.'/share';
$cfg_dir=$cfg_dir1.'/setedit';
$cfg_dir2=$cfg_dir.'/eterm';
$cfg_dir3=$cfg_dir2.'/Setedit';

$Locale2  =$cfg_dir1.'/locale';
$spLocale1=$Locale2.'/es';
$spLocale =$spLocale1.'/LC_MESSAGES';
$deLocale1=$Locale2.'/de';
$deLocale =$deLocale1.'/LC_MESSAGES';

$baseFHS=$base;
$baseFHS.='/share' if $useFHS;
$doc_dir1=$baseFHS.'/doc';
$doc_dir1=$base.'/share/doc' if ($os=~/FreeBSD/ && !$useFHS);
$doc_dir=$doc_dir1.'/setedit';
$man_dir1=$baseFHS.'/man';
$man_dir=$man_dir1.'/man1';
$inf_dir=$baseFHS.'/info';

@tree=(
$base,
$bin_dir,
$cfg_dir1,
$cfg_dir,
$cfg_dir2,
$cfg_dir3,
$inf_dir,
$doc_dir1,
$doc_dir,
$man_dir1,
$man_dir,
$Locale2,
$spLocale1,$spLocale,
$deLocale1,$deLocale,
'result');

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
$d=$bin_dir.'/setedit';
$o='../editor.exe';
if (!(-e $d) or (-M $d > -M $o))
  {
   system("cp -p $o .");
   if ($Strip)
     {
      system("zip -9u result/setedit-$version-Linux-debug.zip editor.exe") unless($iMode);
      system('strip editor.exe');
      $i=`which upx`;
      if (length($i))
        {
         system('upx -9 editor.exe') unless $iCompressExe==0;
        }
     }
   system("mv editor.exe $d");
  }
print "done.\n\n";


print "Copying share files: ";
@cfgs=('../../cfgfiles/*.pmc','../../cfgfiles/*.shl','../../cfgfiles/*.tip',
       '../../fonts/*.sft','../../sdg/txhgen-i.*','../../sdg/*.frt',
       '../../cfgfiles/menubind.smn','../../cfgfiles/redmond.smn',
       '../../cfgfiles/*.slp','../../cfgfiles/*.cle');
foreach $i (@cfgs)
  {
   @a=glob($i);
   foreach $o (@a)
     {
      $o =~ /.*\/(.*)/;
      $d = $cfg_dir.'/'.$1;
      print $1.' ' if (CopyIf($o,$d));
     }
  }
print 'eterm/readme.txt ' if (CopyIf('../../cfgfiles/eterm/readme.txt',$cfg_dir.'/eterm/readme.txt'));
print 'eterm/xterm-eterm-tv ' if (CopyIf('../../cfgfiles/eterm/xterm-eterm-tv',$cfg_dir.'/eterm/xterm-eterm-tv'));
print 'eterm/Setedit/MAIN ' if (CopyIf('../../cfgfiles/eterm/Setedit/MAIN',$cfg_dir.'/eterm/Setedit/MAIN'));
print 'eterm/Setedit/Setedit.menu ' if (CopyIf('../../cfgfiles/eterm/Setedit/Setedit.menu',$cfg_dir.'/eterm/Setedit/Setedit.menu'));
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
      CopyIfCpr('editor.inf','../makes/linux/'.$inf_dir.'/setedit.info');
      CopyIfCpr('sdg.inf','../makes/linux/'.$inf_dir.'/sdg.info');
      CopyIfCpr('infeng.inf','../makes/linux/'.$inf_dir.'/infview.info');
      CopyIfCpr('editor.txt','../makes/linux/'.$doc_dir.'/setedit.txt');
      CopyIfCpr('sdg.txt','../makes/linux/'.$doc_dir.'/sdg.txt');
      CopyIfCpr('infeng.txt','../makes/linux/'.$doc_dir.'/infview.txt');
      CopyIfCpr('setedit.man','../makes/linux/'.$man_dir.'/setedit.1');
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
CopyIfCpr('change.log','makes/linux/'.$doc_dir.'/change.log');
chdir('makes/linux');
CopyIfRpl('../../distrib/distrib1.txt',$doc_dir.'/readme.1st');
CopyIfRpl('../../distrib/distrib1.txt',$cfg_dir.'/readme.1st');
CopyIf('../../distrib/linux.faq',$doc_dir.'/faq.txt');
CopyIf('../../internac/es.mo',$spLocale.'/setedit.mo');
CopyIf('../../internac/de.mo',$deLocale.'/setedit.mo');
print "done.\n\n";


print "Copying other files: ";
@fext=(
'INSTALL.LINUX',
'INSTALL.MAK',
'REMOVE_UNNEEDED',
'VCSA.SH',
'default.map',
'linux.faq',
'ask_config.sh');
# Here we say which files changes their name
%frep=(
'INSTALL.LINUX' => 'INSTALL');
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
   system('tar cvf - '.$base.' | gzip -c > result/setedit-'.$version.'.bin.i386.elf.static.linux.tar.gz');
   print "done.\n";
   
   CopyIfRpl('../../distrib/distrib1.txt','result/readme.1st');
   CopyIfRpl('../../distrib/distrib2.txt','result/announce.txt');
   
   print "\n\nCreating source distribution\n";
   
   chdir('../../..');
   
   open(FIL,'setedit/makes/lista');
   @files=<FIL>;
   close(FIL);
   $r='';
   foreach $i (@files)
     {
      chop($i);
      # Exclude libamp
      #next if ($i =~ /libamp\//);
      $a=substr($i,0,1);
      if (($a eq '-') or ($a eq '+') or ($a eq '*'))
        {
         $i=substr($i,1);
        }
      $r.=' setedit/'.$i;
     }
   open(FIL,'setedit/makes/listaxtr');
   @files=<FIL>;
   close(FIL);
   foreach $i (@files)
     {
      chop($i);
      if (length($i))
        {
         $r.=' '.join(' ',glob('setedit/'.$i));
        }
     }
   
   # Generate the tar file
   $srcdist="setedit-$version.tar.gz";
   unlink($srcdist);
   system("tar cvf - $r | gzip -c > setedit/makes/linux/result/$srcdist");
   #replace('pp',$r);
   
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
    replace($d,$a);
    if (-x $o)
      {
       chmod(0755,$d);
      }
    return 1;
   }
 0;
}
