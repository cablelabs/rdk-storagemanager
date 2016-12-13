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
AM_LDFLAGS = $(GLIB_LDFLAGS) $(GLIB_LIBS) $(G_THREAD_LIBS) -ludev -ldirect  -ldbus-1 -lIARMBus -lpthread -lrdkloggers
#AM_CPPFLAGS =  $(GLIB_CFLAGS) $(GTHREAD_CFLAGS)

if DISKCHECK
AM_CXXFLAGS += -DUSE_DISK_CHECK
endif

if SYSTEMDCHECK
AM_CXXFLAGS += -DUSE_SYSTEMD_SERVICE
endif

if DEEPSLEEP
AM_CXXFLAGS += -DENABLE_DEEP_SLEEP
endif

noinst_LTLIBRARIES = libstmgrutils.la
libstmgrutils_la_SOURCES = 	$(top_srcdir)/src/utils/src/udevUtils.cpp $(top_srcdir)/src/utils/src/sdCardReader.cpp
