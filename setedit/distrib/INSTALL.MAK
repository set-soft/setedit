# Copyright (C) 1998-2002 by Salvador E. Tropea
# Copyright (C) 1996-1998 Robert H”hne, see COPYING.RH for details
# This file is part of RHIDE. 

.SUFFIXES:

setgid=
setuid=
SETGID=
SETUID=
SETMODE=0

ifneq ($(setgid),)
SETGID=-g $(setgid)
SETMODE=2
endif

ifneq ($(setuid),)
SETUID=-o $(setuid)
SETMODE=4
endif

EXEC_MODE=-m $(SETMODE)755
DATA_MODE=-m 0644


INSTALL_BIN=install $(EXEC_MODE) $(SETUID) $(SETGID)
INSTALL_DATA=install $(DATA_MODE)
INSTALL_DIR=install -d -m 0755

prefix=@@pref@@
bin_dir=bin
data_dir=share
info_dir=info
doc_dir=doc
man_dir=man
lib_dir=lib

files:=$(shell find $(bin_dir) $(data_dir) $(info_dir) $(doc_dir) $(man_dir) $(lib_dir)\
	! -type d -print)
ALL_FILES=$(addprefix $(prefix)/,$(files))

$(prefix)/$(bin_dir)/%: $(bin_dir)/%
	$(INSTALL_DIR) $(dir $@)
	@@install_bin@@

$(prefix)/$(bin_dir)/e:
	# IVAN: Create a RELATIVE symlink:
	cd $(prefix)/bin; ln -s setedit e

$(prefix)/share/setedit/install.log:
	export PREFIX=$(prefix); ./ask_config.sh

$(prefix)/%: %
	$(INSTALL_DIR) $(dir $@)
	@@install_data@@

all install: $(ALL_FILES) $(prefix)/$(bin_dir)/e
