# Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
open(FI,"makes/lista") || die "Can't open lista";

while ($a=<FI>)
  {
   chop($a);

   $b=substr($a,0,1);
   if ($b eq "*")
     {
      $conv=1;
      $a=substr($a,1);
     }
   else
     {
      $conv=0;
      if (($b eq "-") or ($b eq "+"))
        {
         $a=substr($a,1);
        }
     }
   
   if ($conv and length($a))
     {
      &ToDOS($a) || die "Failed to convert $a";
     }
  }
system('cp makes/djgppenv.env makes/rhide.env');


sub ToDOS
{
 local $/;
 my $b;
 my $a=$_[0];

 print "Processing $a\n";
 open(FIL,$a) || return 0;
 $b=<FIL>;
 $b =~ s/[!\r]\n/\r\n/g;
 close(FIL);

 open(FIL,">$a") || return 0;
 print FIL ($b);
 close(FIL);
}
