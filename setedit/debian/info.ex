# Unfortunately this seems to not work and I don't know why... so done it
# manually, but if someone knows why this method doesn't work, then tell me
# please.

# This is a configuration files for installing a .info menu
# The Description to be placed into the directory
DESCR="This is a user friendly text editor for programmers."

# The section this info file should be placed in (Regexp) followed by
# the new section name to be created if the Regexp does not match
# (Optional. If not given the .info will be appended to the directory)
SECTION_MATCH="Text Editors"
SECTION_NAME="Text Editors"

# The file referred to from the Info directory
# FIXME: I think this info file name could collide with possibly one provided by update-alternatives for editors! But I am not sure since update-alternatives doesn't have good documentation and I don't understand the purpose of the -slave flag!
FILE=setedit.info

# Optional. The files to be copied to /usr/info
#FILES=*.info
