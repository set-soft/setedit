#!/usr/bin/perl
# Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
open(FIL,'../../infview/version.txt') || return 0;
$version=<FIL>;
chop($version);
close(FIL);
$version1=$version;
$version1=~ s/\.//g;

$r=cat('../../infview/infmain.cc');
if ($r!~/$version/)
  {
   die "Error! inconsistent version in infmain.cc\n";
  }

$binManifest='';

# Default prefix
$pwd=Pwd();
$prefix=$pwd;
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
$nextisprefix=0;
$iMode=0;  # Installation mode
$iCompressExe=1;
foreach $i (@ARGV)
  {
   if ($nextisprefix)
     {
      $prefix=$i;
      $nextisprefix=0;
     }
   elsif ($i eq '--prefix')
     {
      $nextisprefix=1;
     }
   elsif ($i eq '--install')
     {
      $iMode=1;
     }
   elsif ($i eq '--no-compress')
     {
      $iCompressExe=0;
     }
   else
     {
      print "Unknown command line option: $i\n";
      print "Usage: compres.pl [--prefix path] [--install] [--no-compress]\n\n";
     }
  }

# Update the makefiles if needed
if (!$iMode)
  {
   print "Creating makefile: ";
   #system('cp -p ../djgppenv.env ../rhide.env');
   chdir('..');
   system('make makes');
   chdir('djgpp');
   print "done.\n\n";
  }


# Create the distribution tree if needed
print "Creating directories: \n";

if ($iMode)
  {
   # Installation directories
   $bindir    =$prefix.'/bin';
   $contrib1  =$prefix.'/contrib';
   $contrib   =$contrib1.'/infview';
   $info      =$prefix.'/info';
   $share1    =$prefix.'/share';
   $share     =$share1.'/infview';
   $manifest  =$prefix.'/manifest';
   $Locale2   =$share1.'/locale';
   $spLocale1 =$Locale2.'/es';
   $spLocale  =$spLocale1.'/LC_MESSAGES';
   $deLocale1 =$Locale2.'/de';
   $deLocale  =$deLocale1.'/LC_MESSAGES';

   @tree=(
   $bindir,
   $contrib1,
   $contrib,
   $info,
   $share1,
   $share,
   $manifest,
   $Locale2,
   $spLocale1,
   $spLocale,
   $deLocale1,
   $deLocale);
  }
else
  {
   # Distribution directories
   $distPrefix=$prefix.'/distinf';
   $bindir    =$distPrefix.'/bin';
   $contrib1  =$distPrefix.'/contrib';
   $contrib   =$contrib1.'/infview';
   $info      =$distPrefix.'/info';
   $share1    =$distPrefix.'/share';
   $share     =$share1.'/infview';
   $manifest  =$distPrefix.'/manifest';
   $Locale2   =$share1.'/locale';
   $spLocale1 =$Locale2.'/es';
   $spLocale  =$spLocale1.'/LC_MESSAGES';
   $deLocale1 =$Locale2.'/de';
   $deLocale  =$deLocale1.'/LC_MESSAGES';
   $result    =$prefix.'/resulinf';
   
   @tree=(
   $distPrefix,
   $bindir,
   $contrib1,
   $contrib,
   $info,
   $share1,
   $share,
   $manifest,
   $Locale2,
   $spLocale1,
   $spLocale,
   $deLocale1,
   $deLocale,
   $result);
  }

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
$d=$bindir.'/infview.exe';
$o='../infview.exe';
$binManifest.=$d."\n";
if (!(-e $d) or (-M $d > -M $o))
  {
   system("cp -p $o .");
   system("zip -9u $result/infview-$version-DOS-debug.zip infview.exe") unless($iMode);
   # I could use --best but is slooooowwwwwwwww and doesn't compress much in
   # the case of the editor.
   system('upx -9 infview.exe') unless $iCompressExe==0;
   system("mv infview.exe $d");
  }
print "done.\n\n";


print "Copying doc files: ";
chdir('../../doc');
if (system('make txt info man')==0)
  {
   CopyIf('infeng.inf',$info.'/infview.inf',$binManifest);
   CopyIf('infeng.txt',$contrib.'/infview.txt',$binManifest);
   CopyIf('infview.1',$info.'/infview.1',$binManifest);
  }
else
  {
   print "\n************* Attention!! *************\n\n";
   print "Failed to generate the docs ... Skipping the documentation,\n";
   print "you can create it latter, now press ENTER or ^C to stop\n";
   <STDIN>;
  }
chdir('../makes/djgpp');
CopyIfRpl('../../distrib/distrib3.txt',$contrib.'/readme.1st',$binManifest);
print "done.\n\n";


print "Copying other files: ";
print 'copyrigh'     if (CopyIf('../../copyrigh',$contrib.'/copyrigh',$binManifest));
print 'copying.dj'   if (CopyIf('../../copying.dj',$contrib.'/copying.dj',$binManifest));
print 'copying.gpl'  if (CopyIf('../../copying.gpl',$contrib.'/copying.gpl',$binManifest));
print 'copying.lgp'  if (CopyIf('../../copying.lgp',$contrib.'/copying.lgp',$binManifest));
print 'copying.rh'   if (CopyIf('../../copying.rh',$contrib.'/copying.rh',$binManifest));
print 'Spanish messages' if (CopyIf('../../internac/es.mo',$spLocale.'/setedit.mo',$binManifest));
print 'German messages'  if (CopyIf('../../internac/de.mo',$deLocale.'/setedit.mo',$binManifest));
print "done.\n\n";


print "Generating manifest and version files: \n";
$d="$manifest/inf$version1"."b.ver";
open(FIL,">$d") || die "Can't create version file";
print FIL ("inf".$version1."b Info Viewer v$version for DJGPP");
close(FIL);
$binManifest.="$d\n";
$d="$share/version.txt";
open(FIL,">$d") || die "Can't create version.txt";
print FIL ("$version\n$version1\n");
close(FIL);
$binManifest.="$d\n";
$d="$manifest/inf$version1"."b.mft";
$binManifest.="$d\n";
if ($iMode)
  {
   $prefix.='/';
   $binManifest =~ s/$prefix//g;
  }
else
  {
   $i="$distPrefix/";
   $binManifest =~ s/$i//g;
  }
open(FIL,">$d");
print FIL ($binManifest);
close(FIL);
print "done.\n\n";


if ($iMode)
  {
   print "End of installation\n";
  }
else
  {
   print "Compressing the files: ";
   $i=Pwd();
   chdir($distPrefix);
   system("zip -9ru $result/inf".$version1."b.zip *");
   chdir($i);
   print "done.\n\n";
   
   CopyIfRpl('../../distrib/distrib3.txt',"$result/readme.1st");
   
   chdir('contrib/setedit/makes/djgpp');
   
   ToHTML("$result/readme.1st","$result/readmeinf.html",'readme.1st');
  }

0;

sub ToHTML
{
 my $a;

 $a=cat($_[0]);
 open(FIL,">$_[1]") || die "Can't create readme $_[1]";
 print FIL ("<HTML><Title>$_[2] for InfView</Title><Body><Pre>");
 print FIL ($a);
 print FIL ('</Pre></Body></HTML>');
 close(FIL);
}

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
 my $list=\$_[2];

 $$list.=$d."\n";
 if (!(-e $d) or (-M $d > -M $o))
   {
    system("cp -p $o $d");
    return 1;
   }
 0;
}


sub CopyIfRpl
{
 my ($o,$d)=@_;
 my $a;
 my $list=\$_[2];

 $$list.=$d."\n";
 if (!(-e $d) or (-M $d > -M $o))
   {
    $a=cat($o);
    $a =~ s/\@\@v\@\@/$version/g;
    $a =~ s/\@\@v1\@\@/$version1/g;
    replace($d,$a);
    return 1;
   }
 0;
}

sub Pwd
{
 my $pwd;
 $pwd=`pwd`;
 $pwd=`sh pwd` unless $pwd;
 chop($pwd);
 return $pwd;
}

