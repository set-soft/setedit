#!/usr/bin/perl
# Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#   This perl script reads the rhide.env file and replaces all the
# definitions of this file in the known .mak files. Only the needed
# definitions are patched, it means I don't replace a definition that is 100%
# equal to the new one. Additionally the .mak file is stored in disk only if
# it was changed.
#   The script prints a . for each search, a + for a one line replace and a
# * for a multiline replace.
#
@files=('amp3','easydiag','editor','extra','inffd','librhuti','libset',
        'sdgcline','settv','testeasy','install','infview');

# Get the current rhide.env
open(FIL,'rhide.env') || die 'Hey! where is rhide.env?';
@vars=<FIL>;
close(FIL);

PatchFile(1,0);

@files=('../mp3/libamp/libamp','../mp3/mpegsound/mpegsnd');

# Get the current rhide.env
open(FIL,'../mp3/libamp/rhide.env') || die 'Hey! where is ../mp3/libamp/rhide.env?';
@vars=<FIL>;
close(FIL);

PatchFile(0,1);

sub PatchFile
{
 my $TVsp=$_[0],$NoWall=$_[1];
 my $toRep;

 # Patch the environment variables
 foreach $i (@files)
   {
    $i.='.mak';
    print 'Processing '."$i: ";
    $r=&cat($i);
    $changes=0;
    foreach $search (@vars)
      {
       if ($NoWall)
         {
          $search=~s/-Wall//g;
          $search=~s/-Werror//g;
         }
       # Skip comments
       next if (substr($search,0,1) eq '#');
       # Get the name of the variable
       if ($search =~ /([\w_]*)(\s*)=(\s*)(.*)/)
         {
          $name=$1;
          $value=$4;
          #print "$name\n$value\n";
          print '.';
         }
       else
         {
          next; #?
         }
 
       if ($name eq "TVISION_INC" && $TVsp)
         { # That's very special because affects the dependencies
          # Search the original definition
          $repl="^$name"."=(.+)";
          if ($r =~ /$repl/m)
            {
             $toRep=$1;
            }
          else
            {# Robert screwed-up the projects in 1.5
             # and the gprexp tool makes things even worst.
             if ($r =~ /(\S+)\/compatlayer\.h/)
               {
                $toRep=$1;
               }
             else
               {# Not all have TV dependencies
                #die "Can't find original TVISION_INC, nor tv.h path! ($i)";
                $toRep=$value;
               }
            }
          if ($toRep ne $value)
            {
             # Ok, now replace any dependency
             $r =~ s/$toRep\/([\w\.\/]+)/$value\/$1/mg;
             # And the value
             $r =~ s/$repl/$name=$value/mg;
             print 'd';
            }
         }
       else
         {
          $repl="^$name"."=((.+\\\\\\n)+(.+))";
          if (($r =~ /$repl/m) && ($1 ne $value))
            { # Ok multiline
             #print "\nReplacing:\n>$repl<\n>$1<\n>$name=$value<\n";
             $r =~ s/$repl/$name=$value/mg;
             print '*';
             $changes++;
            }
          else
            { # Single line
             $repl="^$name"."=(.*)";
             #FIXME: Ivan: I had to change the below IF because for example
             #SUPPORT_INC could appear more than one time in the same .mak
             #and we need to change it multiple times, maybe the first time
             #it has the same value but the second time it has a different
             #one, and this only checks the first time it appears, so I just
             #avoided the check. Someone with good knowledge of Perl has to
             #fix this The Good Way TM.
             #if (($r =~ /$repl/m) && ($1 ne $value))
             if ($r =~ /$repl/m)
               {
                $r =~ s/$repl/$name=$value/mg;
                print '+';
                $changes++;
               }
             #print "\$1=$1\n";
            }
         }
      }
    if ($changes)
      {
       replace($i,$r);
       print ' updated ';
      }
    print "\n";
   }
}

sub cat
{
 local $/;
 my $b;

 open(FIL,$_[0]) || return 0;
 $b=<FIL>;
 close(FIL);

 $b;
}

sub replace
{
 my $b=\$_[1];

 open(FIL,">$_[0]") || return 0;
 print FIL ($$b);
 close(FIL);
}


