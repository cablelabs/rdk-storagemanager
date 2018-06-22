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
SUBDIRS =
AM_CPPFLAGS = -Wall -Werror -std=c++0x  -g -fPIC -D_REENTRANT -Wall -I${top_srcdir}/refactored/src/mgr -I${top_srcdir}/refactored/src/rpc -I${top_srcdir}/refactored/include -I${includedir}/rdk/iarmbus $(YOCTO_FLAG)
if IS_YOCTO_ENABLED
AM_LDFLAGS = -ldbus-1  -lIARMBus -lrdkloggers -ludev -lsecure_wrapper
else
AM_LDFLAGS = -ldbus-1  -lIARMBus -lrdkloggers -ludev
endif


bin_PROGRAMS = storageMgrMain
storageMgrMain_SOURCES  = main.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorage.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageBase.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageMain.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/rpc/rdkStorageMgr_iarm_internal.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageNVRAM.cpp

if ENABLE_HDD_SUPPORT_IN_V2_CODE
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageHardDisk.cpp ${top_srcdir}/refactored/mgr/smartmonUtiles.cpp
AM_CPPFLAGS += -DENABLE_HDD_SUPPORT_IN_V2_CODE
endif

if ENABLE_SDC_SUPPORT_IN_V2_CODE
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageSDCard.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageeMMC.cpp
AM_CPPFLAGS += -DENABLE_SDC_SUPPORT_IN_V2_CODE
endif
