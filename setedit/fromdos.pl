#!/usr/bin/perl
# Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
# see copyrigh file for details
#

open(FI,"makes/lista") || die "Can't open lista";

while ($a=<FI>)
  {
   chop($a);

   if (substr($a,0,1) eq "-")
     {
      $bin=1;
      $exe=0;
      $a=substr($a,1);
     }
   else
     {
      $bin=0;
      if (substr($a,0,1) eq "+")
        {
         $exe=1;
         $a=substr($a,1);
        }
      else
        {
         $exe=0;
         if (substr($a,0,1) eq "*")
           {
            $a=substr($a,1);
           }
        }
     }
   
   if (!$bin and length($a))
     {
      &ToUNIX($a) || die "Failed to convert $a";
     }
   if ($exe)
     {
      system("chmod +x $a");
     }
  }
system('cp makes/linux.env makes/rhide.env');

sub ToUNIX
{
 local $/;
 my $b;
 my $a=$_[0];

 print "Processing $a\n";
 open(FIL,$a) || return 0;
 $b=<FIL>;
 $b =~ s/\r\n/\n/g;
 close(FIL);

 open(FIL,">$a") || return 0;
 print FIL ($b);
 close(FIL);
}
