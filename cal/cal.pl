###[txh]####################################################################
#
# Cascade Associations Language => CAL
# Copyright (c) 2002 by Salvador Eduardo Tropea (SET)
#
# Module: CAL
# Comment:
# This should be included by any cal script.
#
####################################################################[txi]###

@SkinPriority=('Common','Base');
$NameHTMLFile='unknown.html';

sub Insert
{
 my ($rut,$hash)=@_;
 my ($a,$module,$ret);

 foreach $a (@SkinPriority)
   {
    $module=$a.'/'.$rut.'.pl';
    if (-e $module)
      {
       require $module;
       eval("\$ret=$a$rut(\$hash)");
       print "Executing: $a$rut\n";
       return 1 unless $ret;
      }
   }
 die "Unknown module requested: $rut\n";
}

sub CreateHTML
{
 my ($file)=@_;

 $NameHTMLFile=$file if $file;
 open(FilOut,'>'.$NameHTMLFile) || die "Can't create $NameHTMLFile\n";
}
1;
