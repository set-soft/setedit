#!/usr/bin/perl
# Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#   This script scans all the C++ files in the source tree looking for files
# with internationalization strings and generates a list called po_list
#
require "../miscperl.pl";

# Open the list of files
open(FIL,'../makes/lista');
@files=<FIL>;
close(FIL);
$po_list='';
foreach $i (@files)
  {
   chop($i);
   $a=substr($i,0,1);
   if (($a eq '-') or ($a eq '+') or ($a eq '*'))
     {
      $i=substr($i,1);
     }
   $name='../'.$i;
   if ($name=~/\.cc$/)
     {
      $r=cat($name);
      if ($r=~/\W_{1,2}\(/)
        {
         $po_list.="$name\n";
        }
     }
  }
replace('po_list',$po_list);
