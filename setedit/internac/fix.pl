#!/usr/bin/perl
# Copyright (C) 2001 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#   This script adds a propper header to the .po files.
#
require "../miscperl.pl";
use POSIX qw(strftime);

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
# Perl needs modules for it
#$Now=`date '+%Y-%m-%d %H:%m%z'`;
$Now=strftime "%Y-%m-%d %H:%M%z", localtime;
$h=~s/\@\@DATE\@\@/$Now/;

$h.=$i;

replace($output,$h);

1;
