open(FIL,'tvision.po') || die;

print "// This is a fake file containing the strings that should be
// internationalized and are defined in Turbo Vision (outside
// setedit).

";

while ($a=<FIL>)
  {
   if ($a=~/msgid \"(.*)\"/)
     {
      print "__(\"$1\")\n";
     }
  }
