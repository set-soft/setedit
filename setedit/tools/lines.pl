#!/usr/bin/perl

require "../miscperl.pl";
open(FIL,"<../makes/lista") || die;
$c='';
while ($a=<FIL>)
  {
   $b=substr($a,0,1);
   chomp $a;
   $a=substr($a,1) if ($b eq '+' || $b eq '-' || $b eq '*');
   if ($a=~/\.c$/ || $a=~/\.cc$/ || $a=~/\.h$/ || $a=~/\.pl$/)
     {
      if (!($a=~/^gettext/ || $a=~/^libbzip2/ || $a=~/^libpcre/ ||
            $a=~/^libz/ || $a=~/^mp3\/libamp/ || $a=~/^mp3\/mpegsound/))
        {
         #print "$a\n";
         $c.="../$a ";
        }
     }
  }
system("wc $c");
