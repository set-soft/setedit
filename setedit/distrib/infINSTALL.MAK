# Copyright (C) 1998,1999,2000 by Salvador E. Tropea
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

files:=$(shell find $(bin_dir) $(data_dir) $(info_dir) $(doc_dir) $(man_dir)\
	! -type d -print)
ALL_FILES=$(addprefix $(prefix)/,$(files))

$(prefix)/$(bin_dir)/%: $(bin_dir)/%
	$(INSTALL_DIR) $(dir $@)
	@@install_bin@@

$(prefix)/%: %
	$(INSTALL_DIR) $(dir $@)
	@@install_data@@

all install: $(ALL_FILES)
