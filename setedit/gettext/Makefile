# Makefile for directory with message catalog handling in GNU NLS Utilities.
# Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.
# Simplified by Salvador E. Tropea
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

SHELL = /bin/sh

srcdir = .
top_srcdir = .
top_builddir = ..
VPATH = .

libdir = /usr/lib
datadir = /usr/share
localedir = $(datadir)/locale
aliaspath = $(localedir)
lo = o

AR = ar
CC = gcc

DEFS = -DLOCALEDIR=\"$(localedir)\" -DLOCALE_ALIAS_PATH=\"$(aliaspath)\" \
-DLIBDIR=\"$(libdir)\" -DHAVE_CONFIG_H=1
CPPFLAGS =
CFLAGS = -O2 -Wall -Werror -gstabs+3 -pipe
LDFLAGS =

COMPILE = $(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(XCFLAGS)

COMHDRS = gettext.h gettextP.h hash-string.h
HEADERS = $(COMHDRS) libgnuintl.h libgettext.h loadinfo.h
COMSRCS = bindtextdom.c dcgettext.c dgettext.c gettext.c \
finddomain.c loadmsgcat.c localealias.c textdomain.c l10nflist.c \
explodename.c dcigettext.c dcngettext.c dngettext.c ngettext.c plural.y \
localcharset.c
SOURCES = $(COMSRCS) intl-compat.c
OBJECTS = bindtextdom.$lo dcgettext.$lo dgettext.$lo gettext.$lo \
finddomain.$lo loadmsgcat.$lo localealias.$lo textdomain.$lo l10nflist.$lo \
explodename.$lo dcigettext.$lo dcngettext.$lo dngettext.$lo ngettext.$lo \
plural.$lo localcharset.$lo intl-compat.$lo

.SUFFIXES:
.SUFFIXES: .c .y .o
.c.o:
	$(COMPILE) $<

.y.c:
	$(YACC) $(YFLAGS) --output $@ $<
	rm -f $*.h

INCLUDES = -I.. -I. -I$(top_srcdir)/intl

all: libintl.a libintl.h

libintl.a libgnuintl.a: $(OBJECTS)
	rm -f $@
	$(AR) cru $@ $(OBJECTS)

libintl.h: libgnuintl.h
	cp $(srcdir)/libgnuintl.h libintl.h

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
