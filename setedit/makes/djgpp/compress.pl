#!/usr/bin/perl
# Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
open(FIL,'../../version.txt') || die;
$version=<FIL>;
chop($version);
close(FIL);
$version1=$version;
$version1=~ s/\.//g;

$r=cat('../../include/ced_inte.h');
if ($r!~/$version/)
  {
   die "Error! inconsistent version in ced_inte.h\n";
  }

$r=cat('../../include/vername.h');
$revision=$1 if $r=~/VERSION_REV\s+(\d+)/;
$relname=$1 if $r=~/VERSION_NAME\s+\"([^\"]+)\"/;

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
$Strip=1;
$OnlySources=0;
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
   elsif ($i eq '--no-strip')
     {
      $Strip=0;
     }
   elsif ($i eq '--only-source')
     {
      $OnlySources=1;
     }
   elsif ($i eq '--use-bzip2')
     {
      $UseBzip2=1;
     }
   else
     {
      print "Unknown command line option: $i\n";
      print "Usage: compres.pl [--prefix path] [--install] [--no-compress]\n\n";
     }
  }

@files=('../../distrib/distrib1.txt','../../distrib/distrib2.txt');


# Update the makefiles if needed
if (!$iMode)
  {
   print "Creating makefile: ";
   #system('cp -p ../djgppenv.env ../rhide.env');
   chdir('..');
   system('make makes');
   chdir('djgpp');
   print "done.\n\n";

   print "Creating Borland C++ makefile: ";
   chdir('../..');
   system('perl confignt.pl');
   chdir('makes/djgpp');
   print "done.\n\n";
  }

if ($OnlySources)
  {
   GenerateSourceDistro();
   exit 0;
  }

# Create the distribution tree if needed
print "Creating directories: \n";

if ($iMode)
  {
   # Installation directories
   $bindir    =$prefix.'/bin';
   $contrib1  =$prefix.'/contrib';
   $contrib   =$contrib1.'/setedit.bin';
   $contribX  =$contrib.'/examples';
   $contribTg =$contrib.'/tag_imgs';
   $info      =$prefix.'/info';
   $share1    =$prefix.'/share';
   $share     =$share1.'/setedit';
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
   $contribX,
   $contribTg,
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
   $distPrefix=$prefix.'/distrib';
   $bindir    =$distPrefix.'/bin';
   $contrib1  =$distPrefix.'/contrib';
   $contrib   =$contrib1.'/setedit.bin';
   $contribX  =$contrib.'/examples';
   $contribTg =$contrib.'/tag_imgs';
   $info      =$distPrefix.'/info';
   $share1    =$distPrefix.'/share';
   $share     =$share1.'/setedit';
   $manifest  =$distPrefix.'/manifest';
   $Locale2   =$share1.'/locale';
   $spLocale1 =$Locale2.'/es';
   $spLocale  =$spLocale1.'/LC_MESSAGES';
   $deLocale1 =$Locale2.'/de';
   $deLocale  =$deLocale1.'/LC_MESSAGES';
   $result    =$prefix.'/result';
   
   @tree=(
   $distPrefix,
   $bindir,
   $contrib1,
   $contrib,
   $contribX,
   $contribTg,
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
$d=$bindir.'/setedit.exe';
$o='../editor.exe';
$binManifest.=$d."\n";
if (!(-e $d) or (-M $d > -M $o))
  {
   system("cp -p $o .");
   if ($Strip)
     {
      system("zip -9u $result/setedit-$version-DOS-debug.zip editor.exe") unless($iMode);
      system('strip editor.exe');
      # I could use --best but is slooooowwwwwwwww and doesn't compress much in
      # the case of the editor.
      system('upx -9 editor.exe') unless $iCompressExe==0;
     }
   system("mv editor.exe $d");
  }
print "done.\n\n";


print "Copying the batch: ";
$d=$bindir.'/e.bat';
$o='../../distrib/arranca.bat';
CopyIf($o,$d,$binManifest);
print "done.\n\n";


print "Copying share files: ";
@cfgs=('../../cfgfiles/*.pmc','../../cfgfiles/*.shl','../../cfgfiles/*.tip',
       '../../fonts/*.sft','../../sdg/txhgen-i.*','../../sdg/*.frt',
       '../../cfgfiles/*.smn','../../cfgfiles/simple.dst',
       '../../cfgfiles/*.slp','../../scrnsave/plasma1/ps1.exe',
       '../../scrnsave/plasma2/ps2.exe','../../scrnsave/plasma3/ps3.exe',
       '../../scrnsave/plasma4/ps4.exe','../../scrnsave/extrscsv.txt',
       '../../cfgfiles/*.cle');
foreach $i (@cfgs)
  {
   @a=glob($i);
   foreach $o (@a)
     {
      $o =~ /.*\/(.*)/;
      $d = $share.'/'.$1;
      print $1.' ' if (CopyIf($o,$d,$binManifest));
     }
  }
print "done.\n\n";


print "Copying doc files: ";
chdir('../../doc');
if (system('make txt info man')==0)
  {
   CopyIf('editor.inf',$info.'/setedit.inf',$binManifest);
   CopyIf('sdg.inf',$info.'/sdg.inf',$binManifest);
   CopyIf('infeng.inf',$info.'/infview.inf',$binManifest);
   CopyIf('setedit.1',$info.'/setedit.1',$binManifest);
   CopyIf('editor.txt',$contrib.'/setedit.txt',$binManifest);
   CopyIf('sdg.txt',$contrib.'/sdg.txt',$binManifest);
   CopyIf('infeng.txt',$contrib.'/infview.txt',$binManifest);
  }
else
  {
   print "\n************* Attention!! *************\n\n";
   print "Failed to generate the docs ... Skipping the documentation,\n";
   print "you can create it latter, now press ENTER or ^C to stop\n";
   <STDIN>;
  }
chdir('../makes/djgpp');
CopyIfRpl('../../distrib/distrib1.txt',$contrib.'/readme.1st',$binManifest);
print "done.\n\n";


print "Copying other files: ";
print 'example.zip '  if (CopyIf('../../distrib/example.zip',$contrib.'/example.zip',$binManifest));
print 'kexpand.zip '  if (CopyIf('../../distrib/kextend.zip',$contrib.'/kextend.zip',$binManifest));
print 'calltpc7.zip ' if (CopyIf('../../distrib/calltpc7.zip',$contrib.'/calltpc7.zip',$binManifest));
print 'change.log '   if (CopyIf('../../change.log',$contrib.'/change.log',$binManifest));
print 'copyrigh '     if (CopyIf('../../copyrigh',$contrib.'/copyrigh',$binManifest));
print 'copying.dj '   if (CopyIf('../../copying.dj',$contrib.'/copying.dj',$binManifest));
print 'copying.gpl '  if (CopyIf('../../copying.gpl',$contrib.'/copying.gpl',$binManifest));
print 'copying.lgp '  if (CopyIf('../../copying.lgp',$contrib.'/copying.lgp',$binManifest));
print 'copying.rh '   if (CopyIf('../../copying.rh',$contrib.'/copying.rh',$binManifest));
print 'windos.faq '   if (CopyIf('../../windos.faq',$contrib.'/readme.faq',$binManifest));
print 'setedit.pif '  if (CopyIf('../../distrib/setedit.pif',$bindir.'/setedit.pif',$binManifest));
print 'Spanish messages ' if (CopyIf('../../internac/es.mo',$spLocale.'/setedit.mo',$binManifest));
print 'German messages '  if (CopyIf('../../internac/de.mo',$deLocale.'/setedit.mo',$binManifest));
$a= CopyIf('../../distrib/examples/tvrc',$contribX.'/tvrc',$binManifest);
$a|=CopyIf('../../distrib/examples/examp1.epr',$contribX.'/examp1.epr',$binManifest);
$a|=CopyIf('../../distrib/examples/examp1.dst',$contribX.'/examp1.dst',$binManifest);
$a|=CopyIf('../../distrib/examples/test1.cc',$contribX.'/test1.cc',$binManifest);
print 'Examples '      if ($a);
$a= CopyIf('../../www-site/tags.html',$contrib.'/tags.html',$binManifest);
$a|=CopyIf('../../www-site/my_file1.html',$contrib.'/my_file1.html',$binManifest);
$a|=CopyIf('../../www-site/my_file2.html',$contrib.'/my_file2.html',$binManifest);
$a|=CopyIf('../../www-site/my_file3.html',$contrib.'/my_file3.html',$binManifest);
$a|=CopyIf('../../www-site/examples.css',$contrib.'/examples.css',$binManifest);
print 'Tags tutorial ' if ($a);
@a=glob('../../www-site/tag_imgs/*.png');
foreach $o (@a)
  {
   $o =~ /.*\/(.*)/;
   $d = $contribTg.'/'.$1;
   CopyIf($o,$d,$binManifest);
  }
print "done.\n\n";


print "Generating manifest and version files: \n";
$d="$manifest/edi$version1"."b.ver";
unlink glob("$manifest/edi*b.ver") unless $iMode;
open(FIL,">$d") || die "Can't create version file";
print FIL ("edi".$version1."b SET's editor v$version for DJGPP");
close(FIL);
$binManifest.="$d\n";
$d="$share/version.txt";
open(FIL,">$d") || die "Can't create version.txt";
print FIL ("$version\n$version1\n");
close(FIL);
$binManifest.="$d\n";
$d="$manifest/edi$version1"."b.mft";
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
   unlink glob("$manifest/edi*b.mft");
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
   system("zip -9ru $result/edi".$version1."b.zip *");
   chdir($i);
   print "done.\n\n";
   
   
   CopyIfRpl('../../distrib/distrib1.txt',"$result/readme.1st");
   CopyIfRpl('../../distrib/distrib2.txt',"$result/announce.txt");

   GenerateSourceDistro();

   #############################
   # Change.log in HTML format #
   #############################
   
   $a=cat('../../change.log');
   # Line separators
   $a =~ s/\n(-)+\n/<hr>/mg;
   # Lines between text are paragraphs
   $a =~ s/\n\n/<p>\n/mg;
   $a =~ s/\n\.\n/<p>\n/mg;
   # Convert * to list items
   $a =~ s/\n\*/<br>\n<li>/mg;
   # If any survives
   $a =~ s/\n-/<br>-/mg;
   $a =~ s/-\n/-<br>/mg;
   # Beautyful ;-)
   $a =~ s/Revision/<b>Revision<\/b>/mg;
   $a =~ s/\$\n/\$<br>/mg;
   
   open(FIL,">$result/change.html") || die 'Can not create changelog';
   print FIL ('<HTML><Title>change.log for setedit</Title><Body>');
   print FIL ($a);
   print FIL ('<p>Converted to HTML by a simple Perl script &copy; by SET</Body></HTML>');
   close(FIL);
   
   ToHTML("$result/readme.1st","$result/readme.html",'readme.1st');
   ToHTML("$result/announce.txt","$result/announce.html",'announce');
  }
0;

sub ToHTML
{
 my $a;

 $a=cat($_[0]);
 $a=~s/</&lt;/g;
 $a=~s/>/&gt;/g;
 open(FIL,">$_[1]") || die "Can't create readme $_[1]";
 print FIL ("<HTML><Title>$_[2] for setedit</Title><Body><Pre>");
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
    $a =~ s/\@\@relname\@\@/$relname/g;
    $a =~ s/\@\@rev\@\@/$revision/g;
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

sub GenerateSourceDistro
{
 print "\n\nCreating source distribution\n";
 $srcmft="manifest/edi$version1".'s.mft';
 $srcver="manifest/edi$version1".'s.ver';
    
 chdir('../../../..');
 
 open(FIL,'contrib/setedit/makes/lista');
 @files=<FIL>;
 close(FIL);
 $r='';
 foreach $i (@files)
   {
    $a=substr($i,0,1);
    if (($a eq '-') or ($a eq '+') or ($a eq '*'))
      {
       $i=substr($i,1);
      }
    $r.='contrib/setedit/'.$i;
   }
 open(FIL,'contrib/setedit/makes/listaxtr');
 @files=<FIL>;
 close(FIL);
 foreach $i (@files)
   {
    chop($i);
    $r.=join("\r",glob('contrib/setedit/'.$i))."\r";
   }
 $r.="$srcmft\n$srcver\n";
 replace($srcmft,$r);
 replace($srcver,"edi".$version1."s SET's editor v$version for DJGPP");
 
 
 # Generate the zip files
 $srcdist="edi$version1".'s.zip';
 unlink($srcdist);
 system("zip -9u $result/$srcdist \@$srcmft");
 
 chdir('contrib/setedit/makes/djgpp');
}

