#!/usr/bin/perl
# Copyright (C) 2003 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
#  This script extracts the revision number from the change.log and puts the
# value in the include/vername.h file.
#

require "miscperl.pl";

# The time stamp is checked by the Makefile
#if (-M 'change.log' < -M 'include/vername.h')

print "Updating revision number\n";
$vername=cat('include/vername.h');
$changelog=cat('change.log');
$changelog=~/Revision \d\.(\d*) /;
$revision=$1;
die "Can't determine the revision number\n" unless $revision;
# Source Forge CVS was created around revision 1.515
$revision+=514 unless $changelog=~/Revision 1\.926  2003\/06\/17 22\:09\:57  set/;
$vername=~s/VERSION_REV\s+\d+/VERSION_REV  $revision/;
replace('include/vername.h',$vername);

