#!/usr/bin/perl
# Copyright (C) 2001 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#   This script adds a propper header to the .po files.
#
require "../miscperl.pl";

$header=$ARGV[0];
$input =$ARGV[1];
$output=$ARGV[2];

$h=cat($header);
die "Empty header!\n" unless $h;
$i=cat($input);
die "Empty input file!\n" unless $i;

# Remove all header
$i=~s/(\#((.*)\n)+\# End of header\n)//;

# Update header
GetVersion('../');
$h=~s/\@\@VERSION\@\@/$Version/;
# Use GMT time because in Perl is hard to know the time zone! (modules needed)
($seg,$min,$hora,$dmes,$mes,$ano,$dsem,$dano,$flg)=gmtime;
$Now=sprintf "%d-%02d-%02d %02d:%02d-0000",$ano+1900,$mes+1,$dmes,$hora,$min;
$h=~s/\@\@DATE\@\@/$Now/;

$h.=$i;

replace($output,$h);

1;
