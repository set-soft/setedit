#!/bin/sh
# Copyright (c) 2002 by Iván Baldo
#
# This script should be used to install the binaries from
# the SETEdit binary distribution on your system if you
# want to select default global options for the editor.
#
# FIXME: it uses my (Iván) indentation style to code it faster, but if this
# file is accepted and Salvador want's to change the indentation, I will
# do the work to reindent it upon request.

# Default options:
CentralDesktopFile=1
TabsForIndent=1
CreateBackUps=2

# Configuration file:
cfgFile=$PREFIX/share/setedit/install.log

echo "SETEdit UNIX installation script"
echo "================================"
echo

# $1 is the question description.
# $2 is the number of options.
# $3 is the default option.
# Returns the option number selected in the variable pointed by the $4 parameter.
function ask()
{
	local desc=$1
	local numopt=$2
	local defopt=$3
	while true; do
		echo "$desc"
		echo
		read -p "Choose an option [$defopt]: " selopt
		if [ -z "$selopt" ]; then
			echo "Using default: $defopt."
			selopt=$defopt
		fi
		if ((selopt < 1)) || ((selopt > numopt)); then
			echo "Please enter a value between 1 and $numopt inclusive."
		else
			eval $4=$selopt
			return
		fi
	done
}

qsep="

--------------------------------------------------------------------------------"

function do_questions()
{
	ask "
The editor stores configuration options in files called desktop files. These
files also store information about what files are opened and the size,
position, etc. of the windows. You can have only one global file for this or
one in each directory you use the editor. Which option do you prefer?
1) A desktop file in each directory.
2) One central desktop file." 2 $CentralDesktopFile CentralDesktopFile
	echo "$qsep"
	ask "
The editor is set by default to indent text using spaces. To configure the
editor to use tabs more than one option must be selected. What do you want to
use for indentation?
1) Spaces.
2) Tabs." 2 $TabsForIndent TabsForIndent
	echo "$qsep"
	ask "
Each time the editor stores a modified file to disk a backup file can be
created in case you want to revert the changes. This is specially useful when
you alredy exited the editor and hence undo option isn't available. Do you
want to create backup files?
1) No.
2) Yes, create backup files." 2 $CreateBackUps CreateBackUps
}

# Returns true if confirmed, false if we should ask again.
function confirm_questions()
{
	local str="
You selected:

"
	[ "$CentralDesktopFile" = "1" ] && str="${str}A] Save one desktop file in each directory.
"
	[ "$CentralDesktopFile" = "2" ] && str="${str}A] Save only one central desktop file.
"

	[ "$TabsForIndent" = "1" ] && str="${str}B] Use spaces for indentation.
"
	[ "$TabsForIndent" = "2" ] && str="${str}B] Use tabs for indentation.
"

	[ "$CreateBackUps" = "1" ] && str="${str}C] Do not create backup files.
"
	[ "$CreateBackUps" = "2" ] && str="${str}C] Create backup files.
"
	str="${str}
1) This is correct go on and install.
2) I changed my mind, let me change the options."
	ask "$str" 2 1 ready

	return $((ready - 1))
}

while true; do
	do_questions
	echo "$qsep"
	if confirm_questions; then
		break
	else
		echo "$qsep"
	fi
done

# Save the configuration file, first of all, decrease by one the values of the options, since
# thats the internal format used:
CentralDesktopFile=$((CentralDesktopFile - 1))
TabsForIndent=$((TabsForIndent - 1))
CreateBackUps=$((CreateBackUps - 1))

echo "#
# Default Installation Options file, created during installation.
# These options will be used when no desktop file can be loaded.
#
[Install]
CentralDesktopFile=$CentralDesktopFile
TabsForIndent=$TabsForIndent
CreateBackUps=$CreateBackUps
" > $cfgFile


