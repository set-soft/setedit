#!/usr/bin/perl
# Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
# see copyrigh file for details
#

# Configuration options:
$usePMode=0;

open(FIL,'../version.txt') || die;
$version=<FIL>;
chop($version);
close(FIL);
$version1=$version;
$version1=~ s/\.//g;
$dirResult='../makes/djgpp/result/';
$dirDist='../makes/djgpp/distrib/';
$dirManifest=$dirDist.'manifest';
$djdir=$ENV{'DJDIR'};

$bindist=$dirResult.'edi'.$version1.'b.zip';
$instdist=$dirResult.'edi'.$version1.'i.zip';
print "Creating the installation distribution ...\n";
if (!(-e $instdist) || (-M $bindist < -M $instdist) || (-M 'install.exe' < -M $instdist))
  { # Ok, we must redo it
   # .dat file
   $d='install.dat';
   if (!(-e $d) || (-M $bindist < -M $d))
     {
      open(FIL,"$dirManifest/edi$version1"."b.mft");
      @files=<FIL>;
      close(FIL);
      $list="";
      foreach $i (@files)
         {
          # Internationalization files have all the same name, avoid it
          if ($i=~/share\/locale\/(.*)\/LC_MESSAGES\/setedit.mo/)
            {
             system("cp $dirDist$i $1".'setedit.mo');
             $list.=$1.'setedit.mo ';
            }
          else
            {
             $list.=$dirDist.$i.' ';
            }
         }
      system("dat -a -s1 -c0 -k -t DATA $d $list $djdir/bin/cwsdpmi.exe $djdir/bin/cwsdpmi.doc $djdir/bin/emu387.dxe");
      # Cleanup temporal files
      foreach $i (@files)
         {
          if ($i=~/share\/locale\/(.*)\/LC_MESSAGES\/setedit.mo/)
            {
             system("rm $1".'setedit.mo');
            }
         }
      die 'Error creating .dat file' unless (-e $d);
     }
   # simple test
   $d='install.exe';
   die 'The install.exe is missing?!' unless (-e $d);
   system("upx $d");
   if ($usePMode)
     {
      trySystem("exe2coff $d");
      $stub="$djdir/bin/pmodstub.exe";
      $stub =~ s/\//\\/g;
      trySystem("copy /B $stub + install $d");
      unlink('install');
      $dpmi='';
     }
   else
     {
      trySystem("cp $djdir/bin/cwsdpmi.exe .");
      $dpmi='cwsdpmi.exe';
     }
   trySystem("exedat $d install.dat");
   trySystem("cp $djdir/bin/emu387.dxe .");
   trySystem("zip -9 $instdist $d emu387.dxe $dpmi");
   unlink('emu387.dxe');
   trySystem("mkdir es");
   trySystem("mkdir es\\LC_MESSAGES");
   trySystem("cp ../internac/es.mo es/LC_MESSAGES/install.mo");
   trySystem("mkdir de");
   trySystem("mkdir de\\LC_MESSAGES");
   trySystem("cp ../internac/de.mo de/LC_MESSAGES/install.mo");
   trySystem("zip -9rm $instdist es de");
  }
else
  {
   print "Is uptodate\n";
  }

sub trySystem
{
 print "Running $_[0]\n";
 system($_[0]) and die "Error running $_[0]\n";
}
