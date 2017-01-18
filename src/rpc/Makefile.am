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
#

SUBDIRS =
DIST_SUBDIRS =

AM_CXXFLAGS = $(GLIB_CFLAGS) $(GTHREAD_CFLAGS) -I$(top_srcdir)/src/rpc/include -I$(top_srcdir)/src/utils/include

AM_LDFLAGS = $(GLIB_LDFLAGS) $(GLIB_LIBS) $(G_THREAD_LIBS) -ludev -ldirect -ldbus-1 -lIARMBus -lpthread -lrdkloggers
AM_CPPFLAGS =  $(GLIB_CFLAGS) $(GTHREAD_CFLAGS)

if DISKCHECK
AM_CXXFLAGS += -DUSE_DISK_CHECK
endif

if DEEPSLEEP
AM_CXXFLAGS += -DENABLE_DEEP_SLEEP
endif

if SMARTSUPPORT
AM_CXXFLAGS += -DENABLE_SMARTMONTOOL_SUPPORT
endif

if SDCSUPPORT
AM_CXXFLAGS += -DENABLE_SDCARD_SUPPORT
endif

# the list of header files that belong to the library (to be installed later)
include_HEADERS =    $(top_srcdir)/src/rpc/include/storageMgr.h

bin_PROGRAMS = storageMgrMain
storageMgrMain_SOURCES=$(top_srcdir)/src/rpc/src/storageMgr.cpp $(top_srcdir)/src/rpc/src/storageMgrMain.cpp
storageMgrMain_LDADD = $(top_builddir)/src/utils/libstmgrutils.la
