#!/usr/bin/perl
#
# Copyright (c) 2002 by Salvador E. Tropea
#
# This script should be used to install the binaries from
# the SETEdit binary distribution on your system if you
# have Perl interpreter installed and want to select
# default global options for the editor.
#

@Value=(0,0,1);
@Questions=("
The editor stores configuration options in files called desktop files. These
files also stores information about what files are opened and the size,
position, etc. of the windows. You can have only one global file for this or
one in each directory you use the editor. Which option do you prefer?
1) A desktop file in each directory.
2) One central desktop file.","
The editor is set by default to indent text using spaces. To configure the
editor to use tabs more than one option must be selected. What do you want to
use for indentation.
1) Spaces.
2) Tabs.","
Each time the editor stores a modified file to disk a backup file can be
created in case you want to revert the changes. This is specially useful when
you alredy exited the editor and hence undo option isn't available. Do you
want to create backup files?
1) No.
2) Yes, create backup files.");
@Opts=("CentralDesktopFile","TabsForIndent","CreateBackUps");

print "SETEdit UNIX installation script\n\n";

$OK=1;
do
  {
   MakeQuestions();
   print "You selected:\n";
   print "A] Save only one central desktop file.\n" if $Value[0];
   print "A] Save one desktop file in each directory.\n" unless $Value[0];
   print "B] Use tabs for indentation.\n" if $Value[1];
   print "B] Use spaces for indentation.\n" unless $Value[1];
   print "C] Create backup files.\n" if $Value[2];
   print "C] Do not create backup files.\n" unless $Value[2];
   print "\n1) This is correct go on and install\n";
   print "2) I changed my mind let me change the options\n";
   $OK=1-Ask1or2('Choose an option',0);
   print "\n=======================================================================\n" if (!$OK);
  }
while (!$OK);

open(FIL,'>share/setedit/install.log') || die;
$i=0;
print FIL "#
# Default Installation Options file, created during installation.
# These options will be used when no desktop file can be loaded.
#
[Install]
";
foreach $o (@Opts)
  {
   print FIL "$o=$Value[$i]\n";
   $i++;
  }
close(FIL);

sub Ask1or2
{
 my ($text,$default)=@_;
 my $val;

 while (1)
   {
    $val=$default+1;
    print "\n$text [$val]: ";
    $b=<STDIN>;
    chop $b;
    if ($b eq '')
      {
       print "Using default: $val\n";
       return $default;
      }
    if (($b ne '1') && ($b ne '2') && ($b ne ''))
      {
       print "Please just enter 1 or 2\n";
      }
    return $b eq '1' ? 0 : 1;
   }
 return 0;
}

sub MakeQuestions
{
 my $i=0;
 my $b;

 foreach $q (@Questions)
   {
    print "$q\n";
    $Value[$i]=Ask1or2('Choose an option',$Value[$i]);
    $i++;
    print "\n------------------------------------------------------------------------------\n\n";
   }
}
