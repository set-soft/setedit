#!/usr/bin/perl
# Copyright (C) 2002 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#  Used to synchronize my CVS copy using synmail output.
#  This script was designed to use the mails sent by syncmail installed
# in the Source Forge CVS.
#  It takes a batch of mails and does the following tasks:
# * Extracts the ci message.
# * Applies the patches to the correct files, the patch isn't easy to
# apply directly because patch doesn't know in which directory is the
# file.
# * If the process succeeds modifies change.log and does a check-in of
# the patched files.
# * In case of errors the process is reverted removing the patched files
# and getting fresh copies from CVS. The ci message remains in
# /tmp.mensaje and the last patch processed in /tmp/parche.
#
# IMPORTANT! the script assumes the files to patch are exactly the same
# you'll get using a cvs update, if this isn't true the roll-back will
# fail.
#

require "miscperl.pl";

#$simulate=1;

$estado=0;
$LogMessage='';
$toPatch;
$toSkip=0;
$errors=0;
$applied=0;
$dirNewFile='';
$inNewFile=0;
$files='change.log ';
while ($a=<>)
  {
   if ($estado==1)
     {
      if ($a ne "\n")
        {
         $LogMessage.=$a;
         print FIL $a;
        }
      else
        {
         $estado=0;
         close(FIL);
         print "Mensaje:\n$LogMessage";
        }
     }
   elsif ($estado==2)
     {
      if ($a=~/^RCS file\: \/cvsroot\/([^\/]*)\/([^\/]*)\/(.*)\,v/)
        {
         $estado=3;
         $toPatch=$3;
         $toPatch=~s/Attic\///;
        }
      else
        {
         print $a;
        }
     }
   elsif ($estado==3)
     {
      if ($a=~/diff /)
        {
         $estado=4;
         open(FIL,">/tmp/parche") || die;
         print FIL $a;
        }
     }
   elsif ($estado==4)
     {
      if ($a=~/^(\*\*\*|\-\-\-) (.*)\t(.*)\t(.*)/)
        {
         print FIL "$1 $toPatch\t$3\t$4\n";
        }
      elsif ($a eq "***************\n")
        {
         print FIL $a;
         $estado=5;
        }
      else
        {
         print FIL $a;
        }
     }
   elsif ($estado==5)
     {
      if ($a=~/^(\*\*\*|\-\-\-) (\d*),(\d*) (\*\*\*\*|\-\-\-\-)\n/)
        {
         $toSkip=$3-$2+1;
         print FIL $a;
         $firstLine=1;
        }
      elsif ($a=~/^(\*\*\*|\-\-\-) (\d*) (\*\*\*\*|\-\-\-\-)\n/)
        {
         $toSkip=1;
         print FIL $a;
         $firstLine=1;
        }
      elsif (!$toSkip)
        {
         if (($a ne "***************\n") && ($a ne "\\ No newline at end of file\n"))
           {
            $estado=0;
            close(FIL);
            $ret=RunCommand("patch -p0 -i /tmp/parche");
            print "Patch result: $ret\n";
            if ($ret) { $errors++; }
            $applied++;
            $files.=$toPatch.' ';
           }
         else
           {
            print FIL $a;
           }
        }
      else
        {
         print FIL $a;
         $toSkip--;
         if ($firstLine)
           {
            $firstLine=0;
            if ($a eq "\n")
              {
               $estado=0;
               close(FIL);
               $ret=RunCommand("patch -p0 -i /tmp/parche");
               print "Patch result: $ret\n";
               if ($ret) { $errors++; }
               $applied++;
               $files.=$toPatch.' ';
              }
           }
        }
     }
   elsif ($a=~/^Index\: (.*)/)
     {
      if ($inNewFile)
        {
         close FIL;
         $inNewFile=0;
        }
      $estado=2 unless $1 eq 'change.log';
      print "$1\n";
     }
   elsif ($a=~/\-\-\- NEW FILE: (.*) \-\-\-/)
     {
      if ($inNewFile)
        {
         close FIL;
        }
      $dirNewFile='.' if !$dirNewFile;
      print "New file: $dirNewFile/$1\n";
      open(FIL,">$dirNewFile/$1") || die;
      $inNewFile=1;
      $newFiles.="$dirNewFile/$1 ";
     }
   elsif ($inNewFile)
     {
      print FIL $a;
     }
   elsif ($a=~/Update of \/cvsroot(\/[^\/]*)(\/[^\/]*\/)(.*$)/)
     {
      $dirNewFile=$3;
     }
   elsif (!$LogMessage && $a=~/^Log Message\:/)
     {
      $estado=1;
      open(FIL,">/tmp/mensaje") || die;
     }
  }
if (!$errors && $applied)
  {
   print "Successful process, doing a check-in\n";
   if ($newFiles)
     {
      RunCommand("cvs add $newFiles");
     }
   $a=cat('change.log');
   $a=~s/\$Log\: (.*),v \$/\$Log\: $1,v \$\./;
   replace('change.log',$a) unless $simulate;
   RunCommand("cvs ci -F /tmp/mensaje $files $newFiles");
   unlink('/tmp/mensaje','/tmp/parche');
  }
elsif ($applied)
  {
   print "Errors while patching, reverting patched files\n";
   RunCommand("rm $files");
   RunCommand("cvs update $files");
  }

sub RunCommand
{
 my ($command)=@_;

 if ($simulate)
   {
    print "Ejecutes: $command\n";
    return 0;
   }
 return system($command);
}

