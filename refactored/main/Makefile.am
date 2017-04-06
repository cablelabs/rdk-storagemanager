SUBDIRS =
AM_CPPFLAGS = -Wall -Werror -std=c++0x  -g -fPIC -D_REENTRANT -Wall -I${top_srcdir}/refactored/src/mgr -I${top_srcdir}/refactored/src/rpc -I${top_srcdir}/refactored/include -I=${includedir}/rdk/iarmbus
AM_LDFLAGS = -ldbus-1  -lIARMBus -lrdkloggers -ludev

if DEEPSLEEP
AM_CXXFLAGS = -DENABLE_DEEP_SLEEP
endif

bin_PROGRAMS = storageMgrMain
storageMgrMain_SOURCES  = main.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorage.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageBase.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageMain.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageNVRAM.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageHardDisk.cpp ${top_srcdir}/refactored/mgr/smartmonUtiles.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageSDCard.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/rpc/rdkStorageMgr_iarm_internal.cpp
