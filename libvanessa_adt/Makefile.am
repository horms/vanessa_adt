######################################################################
# Makefile.am                                            February 2000
# Horms                                             horms@verge.net.au
#
# vanessa_adt
# Library of Abstract Data Types
# Copyright (C) 1999-2003  Horms
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
#
######################################################################

lib_LTLIBRARIES = libvanessa_adt.la

include_HEADERS = vanessa_adt.h

libvanessa_adt_la_SOURCES = \
vanessa_adt.h \
dynamic_array.c \
element_ops.c \
queue.c \
key_value.c \
config_file.c \
list.c \
hash.c

libvanessa_adt_la_LDFLAGS    = -version-info 0:3:0

libvanessa_adt_la_LIBADD     = -lvanessa_logger
