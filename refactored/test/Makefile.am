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
bin_PROGRAMS = rdkStorageTest
rdkStorageTest_SOURCES = rdkStorageTest.cpp
rdkStorageTest_LDADD     = $(top_builddir)/refactored/rpc/librdkstmgr.la

AM_CPPFLAGS = -Wall -Werror -I${top_srcdir}/refactored/include -I${includedir}/rdk/iarmbus
AM_LDFLAGS = -ldbus-1  -lIARMBus

#This is hack to keep the builds not breaking during the integration of new refactored code.
#include_HEADERS = ${top_srcdir}/src/rpc/include/storageMgr.h
