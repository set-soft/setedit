# Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
# Configuration variables

# Convert text files to UNIX format
#$convertUNIX=1;
#$dontCopyBinaries=1;

open(FIL,"listagcc");

$orig=<FIL>; chop($orig);
$diff=<FIL>; chop($diff);
$dest=<FIL>; chop($dest);

print "Original: $orig\nDestino:  $dest\nDiffs en: $diff\n";

$cant=0;

# Clean the destination directories, is there any Perl for it?
print "\nDeleting all the files in $dest ...\n";
system("rm -r $dest*");
print "Deleting all the files in $diff ...\n";
system("rm -r $diff*");


while (!eof(FIL))
#while (!$a)
  {
   #$a=1;
   $file=<FIL>; chop($file);
   if (substr($file,0,1) eq "-")
     {
      $binary=1;
      $file=substr($file,1,length($file)-1);
     }
   else
     {
      $binary=0;
     }
   print $file;
   # die if the file doesn't exist in current version, buggy list
   if (!(-e $file))
     {
      die("$file doesn't exist check listagcc");
     }
   # check if that's new
   if (!(-e "$orig$file"))
     {
      # if that's new unconditionally copy it
      print " is new ";
      # is there any Perl for it?
      system("cp -pP $file $diff");
      print "-> copied ";
     }
   else
     {
      $t1=-M "$file";
      $t2=-M "$orig$file";
      if ($binary)
        {
         if ($t1!=$t2)
           {
            print " binary modified ";
            system("cp -pP $file $diff");
            print "-> copied ";
           }
         else
           {
            print " unchanged ";
           }
        }
      else
        {
         if ($t1!=$t2 || &SizeOfFile("$orig$file")!=&SizeOfFile("$file"))
           {
            print " modified ",$t1!=$t2 ? "[T] " : "[S] ";
            system("diff -u $orig$file $file >> $diff"."rest.dif");
            print "-> diff ";
           }
         else
           {
            print " unchanged ";
           }
        }
     }
   if (!$dontCopyBinaries || !$binary)
     {
      system("cp -pP $file $dest");
      ($binary || !$convertUNIX) ||
         system("dtou $dest$file");
      print "[copied]";
     }
   print "\n";
  }

close(FIL);

#
# This routine meassures the file length removing the CR+LF characters
#
sub SizeOfFile
{
 my($F1,@f,$l,$line);

 open F1,$_[0];
 #binmode(F1); used to test under DOS
 @f=<F1>;
 $l=0;
 foreach $line (@f)
    {
     chop($line);
     (substr($line,length($line)-1,1) ne "\r") ||
       chop($line);
     $l+=length($line);
    }
 #print "largo: $l ";
 return $l;
}
