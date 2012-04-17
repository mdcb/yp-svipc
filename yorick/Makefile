#
#  Copyright (C) 2011-2012  Matthieu Bec.
#
#  This file is part of yp-svipc.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# these values filled in by    yorick -batch make.i
Y_MAKEDIR=/usr//lib/yorick
Y_EXE=/usr//lib/yorick/bin/yorick
Y_EXE_PKGS=
Y_EXE_HOME=/usr//lib/yorick
Y_EXE_SITE=/usr//lib/yorick

# ----------------------------------------------------- optimization flags

# options for make command line, e.g.-   make COPT=-g TGT=exe
COPT=$(COPT_DEFAULT)
TGT=$(DEFAULT_TGT)

# ------------------------------------------------ macros for this package

PKG_NAME=svipc
PKG_I=svipc.i

OBJS=yorick_svipc.o common_svipc_misc.o common_svipc_shm.o common_svipc_sem.o common_svipc_msq.o


# change to give the executable a name other than yorick
PKG_EXENAME=yorick

# PKG_DEPLIBS=-Lsomedir -lsomelib   for dependencies of this package
PKG_DEPLIBS=
# set compiler (or rarely loader) flags specific to this package
PKG_CFLAGS=-I ../common -Wall

#freeBSD/Darwin timedop compat hack
ifneq ($(shell uname),Linux)
PKG_CFLAGS+=-DSVIPC_HACKS
endif

PKG_LDFLAGS=

# list of additional package names you want in PKG_EXENAME
# (typically Y_EXE_PKGS should be first here)
EXTRA_PKGS=$(Y_EXE_PKGS)

# list of additional files for clean
PKG_CLEAN=

# autoload file for this package, if any
PKG_I_START=
# non-pkg.i include files for this package, if any
PKG_I_EXTRA=

# -------------------------------- standard targets and rules (in Makepkg)

# set macros Makepkg uses in target and dependency names
# DLL_TARGETS, LIB_TARGETS, EXE_TARGETS
# are any additional targets (defined below) prerequisite to
# the plugin library, archive library, and executable, respectively
PKG_I_DEPS=$(PKG_I)
Y_DISTMAKE=distmake

include $(Y_MAKEDIR)/Make.cfg
include $(Y_MAKEDIR)/Makepkg
include $(Y_MAKEDIR)/Make$(TGT)

# override macros Makepkg sets for rules and other macros
# Y_HOME and Y_SITE in Make.cfg may not be correct (e.g.- relocatable)
Y_HOME=$(Y_EXE_HOME)
Y_SITE=$(Y_EXE_SITE)

# reduce chance of yorick-1.5 corrupting this Makefile
MAKE_TEMPLATE = protect-against-1.5

# ------------------------------------- targets and rules for this package

common_svipc_misc.o: ../common/svipc_misc.c ../common/svipc_misc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c ../common/svipc_misc.c
common_svipc_shm.o: ../common/svipc_shm.c ../common/svipc_shm.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c ../common/svipc_shm.c
common_svipc_sem.o: ../common/svipc_sem.c ../common/svipc_sem.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c ../common/svipc_sem.c
common_svipc_msq.o: ../common/svipc_msq.c ../common/svipc_msq.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c ../common/svipc_msq.c

# -------------------------------------------------------- end of Makefile
