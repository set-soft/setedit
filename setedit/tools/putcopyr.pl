sub SaveIt;
sub FindCopy;

$Copy1="Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),";
$Copy2="see copyrigh file for details";
$CopyCpp='/* '.$Copy1."\n   ".$Copy2." */\n";
$CopySHL='# '.$Copy1."\n# ".$Copy2."\n#\n";
$CopyPMC='; '.$Copy1."\n; ".$Copy2."\n;\n";
$CopyTX ='@c '.$Copy1."\n\@c ".$Copy2."\n\n";
$CopyBAT='@Rem '.$Copy1."\n\@Rem ".$Copy2."\n@\n";

print 'Copyright stuffer by SET'."\n\n";

open(FI,"../makes/lista") || die "Can't open listagcc";
@prgs=<FI>;
close(FI);

chdir('..');
foreach $prg (@prgs)
{
 chop $prg;
 length($prg) || next;
 $a=substr($prg,0,1);
 ($a ne '-') || next;
 if (($a eq '*') or ($a eq '+'))
   {
    $prg=substr($prg,1,length($prg)-1);
   }
 $a=&cat($prg);
 $ret=FindCopy($a);
 if (!$ret)
   {
    print "No mine: $prg\n";
   }
 elsif ($ret eq 'Update')
   {
    $a =~ s/1999/1999,2000/;
    print "Updated year: $prg ";
    SaveIt($a,'');
   }
 elsif ($ret eq 'Save')
   {
    if ($prg =~ /\.(([ch]\w?)|(000)|(s))$/)
      {
       SaveIt($a,$CopyCpp);
      }
    elsif ($prg =~ /\.((shl)|(pl)|(smn))$/)
      {
       SaveIt($a,$CopySHL);
      }
    elsif ($prg =~ /\.tx$/)
      {
       SaveIt($a,$CopyTX);
      }
    elsif ($prg =~ /\.((pmc)|(slp))$/)
      {
       SaveIt($a,$CopyPMC);
      }
    elsif ($prg =~ /\.bat$/)
      {
       SaveIt($a,$CopyBAT);
      }
    else
      {
       print "Skipped: $prg\n";
      }
   }
}
chdir('tools');

sub SaveIt
{
 open(FI,'>'.$prg);
 print FI ($_[1]);
 print FI ($_[0]);
 close(FI);
 print "* Altered: $prg\n";
}

sub FindCopy
{
 my $str=\$_[0]; # by reference
 my $b;

 if ($$str =~ /\(C\) tomislav/)
   {
    return 0;
   }
 if ($$str =~ /Ove Kaaven/)
   {
    return 0;
   }
 if ($$str =~ /Andrew Richards/)
   {
    return 0;
   }
 if ($$str =~ /[Cc]opyright(.*)$/gm)
   {
    if ($1 =~ /Salvador/)
      {
       if ($$str =~ /2000/)
         {
          return 1;
         }
       return 'Update';
      }
    return 0;
   }
 'Save';
}

sub cat
{
 my ($file,$ret)=@_;
 local $/;

 open(MY_FILE,$file) || die "Can't open '$file'";
 $ret=<MY_FILE>;
 close(MY_FILE);
 $ret;
}

