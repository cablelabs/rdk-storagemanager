##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
m4_define([stmgr_major_version], [2])
m4_define([stmgr_minor_version], [0])
m4_define([stmgr_micro_version], [0])
m4_define([stmgr_version], [stmgr_major_version.stmgr_minor_version.stmgr_micro_version])

AC_INIT([StorageManager], [stmgr_version], [rajnish_dey2@cable.comcast.com])
AC_CONFIG_SRCDIR([src/rpc/src/storageMgr.cpp])
#AC_CONFIG_SRCDIR([src/udev_test.c])

STMGR_MAJOR_VERSION=stmgr_major_version
STMGR_MINOR_VERSION=stmgr_minor_version
STMGR_MICRO_VERSION=stmgr_micro_version

AC_SUBST(STMGR_MAJOR_VERSION)
AC_SUBST(STMGR_MINOR_VERSION)
AC_SUBST(STMGR_MICRO_VERSION)


AC_CONFIG_MACRO_DIR([cfg])
AC_CONFIG_AUX_DIR([cfg])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 1.9])
AC_CONFIG_HEADERS([cfg/config.h])

LT_INIT

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
[AM_DEFAULT_VERBOSITY=1
AC_SUBST(AM_DEFAULT_VERBOSITY)])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
DISKCHECK=" "
SYSTEMDCHECK=" "
DEEPSLEEP=" "
SMARTSUPPORT=" "
SDCSUPPORT=" "

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdlib.h unistd.h])
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.24.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0 >= 2.24.0])

AC_ARG_ENABLE([diskcheck],
	AS_HELP_STRING([--enable-diskcheck],[This will use disk_check script from '/lib/rdk.']),
	[
	  case "${enableval}" in
  	   yes) DISKCHECK=true ;;
           no) DISKCHECK=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-diskcheck ]) ;;
           esac
           ],
	 [echo " diskcheck is enable"])
AM_CONDITIONAL([DISKCHECK], [test x$DISKCHECK = xtrue])

AC_ARG_ENABLE([systemdcheck],
	AS_HELP_STRING([--enable-systemdcheck],[This will use the systemd services.]),
	[
	  case "${enableval}" in
  	   yes) SYSTEMDCHECK=true ;;
           no) SYSTEMDCHECK=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-systemdcheck ]) ;;
           esac
           ],
	 [echo " systemdcheck is enable"])
AM_CONDITIONAL([SYSTEMDCHECK], [test x$SYSTEMDCHECK = xtrue])


AC_ARG_ENABLE([deepsleep],
        AS_HELP_STRING([--enable-deepsleep],[This will enable DEEPSLEEP functionality. ]),
        [
          case "${enableval}" in
           yes) DEEPSLEEP=true ;;
           no) DEEPSLEEP=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-deepsleep ]) ;;
           esac
           ],
         [echo " deepsleep is enable"])
AM_CONDITIONAL([DEEPSLEEP], [test x$DEEPSLEEP = xtrue])

AC_ARG_ENABLE([smartsupport],
        AS_HELP_STRING([--enable-smartsupport],[This will enable Hard Disk Smartmontools supported functionality. ]),
        [
          case "${enableval}" in
           yes) SMARTSUPPORT=true ;;
           no) SMARTSUPPORT=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-smartsupport ]) ;;
           esac
           ],
         [echo " SMART MON Tool is enable"])
AM_CONDITIONAL([SMARTSUPPORT], [test x$SMARTSUPPORT = xtrue])

AC_ARG_ENABLE([sdc],
        AS_HELP_STRING([--enable-sdc],[This will enable SD Card supported functionality. ]),
        [
          case "${enableval}" in
           yes) SDCSUPPORT=true ;;
           no) SDCSUPPORT=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-sdc ]) ;;
           esac
           ],
         [echo " SDCard storage api support is enable"])
AM_CONDITIONAL([ENABLE_SDC_SUPPORT_IN_V2_CODE], [test x$SDCSUPPORT = xtrue])

AC_CHECK_LIB(gthread-2.0, g_thread_init)

ENABLE_STMGR_V2_CODE=""
AC_ARG_ENABLE([refactored-mgr],
        AS_HELP_STRING([--enable-refactored-mgr],[This will enable refactored-src compilation instead of legacy src/includes. ]),
        [
          case "${enableval}" in
           yes) ENABLE_STMGR_V2_CODE=true ;;
            no) ENABLE_STMGR_V2_CODE=false echo[latest mgr code is disabled & Legacy code is used];;
             *) AC_MSG_ERROR([bad value ${enableval} for --enable-refactored-mgr ]) ;;
          esac
        ],
        [ echo "Used Legacy Code n disabled Refactored Code"])

AM_CONDITIONAL(USE_REFACTORED, test "x$ENABLE_STMGR_V2_CODE" = xtrue)

AC_ARG_ENABLE([hdd], AS_HELP_STRING([--enable-hdd], [This will enable refactored-src with hdd-support; default is enabled]), ,[enable_hdd=yes])
AM_CONDITIONAL(ENABLE_HDD_SUPPORT_IN_V2_CODE, test "x$enable_hdd" != "xno")


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

if test "x$ENABLE_STMGR_V2_CODE" = "xtrue"; then
    AC_CONFIG_FILES([storagemanager.pc])
fi

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/utils/Makefile])
AC_CONFIG_FILES([src/rpc/Makefile])
AC_CONFIG_FILES([test/Makefile])

AC_CONFIG_FILES([refactored/Makefile])
AC_CONFIG_FILES([refactored/rpc/Makefile])
AC_CONFIG_FILES([refactored/mgr/Makefile])
AC_CONFIG_FILES([refactored/main/Makefile])
AC_CONFIG_FILES([refactored/test/Makefile])
AC_OUTPUT
