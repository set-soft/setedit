#!/bin/sh
# Copyright (C) 2003 by Salvador E. Tropea (SET),
# see copyrigh file for details
#
# This file executes the steps needed to create the Turbo Vision and SETEdit
# RPM files. It doesn't check for errors and is really simple.
# It assumes both tarballs were uncompressed in the same directory and that
# you are running the script from ../
#
cd ../tvision
chmod +x redhat/create-rpms.sh
redhat/create-rpms.sh
cd ../setedit
chmod +x redhat/create-rpms.sh
redhat/create-rpms.sh

