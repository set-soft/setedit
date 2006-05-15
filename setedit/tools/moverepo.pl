#!/usr/bin/perl
$newRoot=":ext:set\@setedit.cvs.sourceforge.net:/cvsroot/setedit\n";
$maxDe=5;
$pat='CVS/Root';
while ($maxDe--)
  {
   @a=(@a,glob($pat));
   $pat='*/'.$pat;
  }

print scalar(@a)." Root's found\nCross your fingers ...\n";
foreach $b (@a)
   {
    print "$b\n";
    open(FIL,">$b") || die "Can't create '$b' \n";
    print FIL ($newRoot);
    close(FIL);
   }
print "Ok, all changed\n";


