#
# If not stated otherwise in this file or this component's Licenses.txt file the
# following copyright and licenses apply:
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
#
if USE_REFACTORED
AM_CPPFLAGS = -Wall -Werror -I${top_srcdir}/refactored/src/main -I${top_srcdir}/refactored/include -I${includedir}/rdk/iarmbus

lib_LTLIBRARIES = librdkstmgr.la
librdkstmgr_la_SOURCES = rdkStorageMgr_iarm_external.cpp
librdkstmgr_la_includedir = $(includedir)
librdkstmgr_la_include_HEADERS  = ${top_srcdir}/refactored/include/rdkStorageMgr.h
librdkstmgr_la_include_HEADERS += ${top_srcdir}/refactored/include/rdkStorageMgrTypes.h
librdkstmgr_la_LDFLAGS  = $(GLIB_LDFLAGS) $(GLIB_LIBS) $(G_THREAD_LIBS) -ldbus-1  -lIARMBus -lrdkloggers
librdkstmgr_la_LDFLAGS += -version-info @STMGR_MAJOR_VERSION@:@STMGR_MINOR_VERSION@:@STMGR_MICRO_VERSION@
endif
