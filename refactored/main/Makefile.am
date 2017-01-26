SUBDIRS =
AM_CPPFLAGS = -std=c++0x  -g -fPIC -D_REENTRANT -Wall -I${top_srcdir}/refactored/src/mgr -I${top_srcdir}/refactored/src/rpc -I${top_srcdir}/refactored/include -I=${includedir}/rdk/iarmbus
AM_LDFLAGS = -ldbus-1  -lIARMBus -lrdkloggers -ludev

bin_PROGRAMS = storageMgrMain
storageMgrMain_SOURCES  = main.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorage.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageBase.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/mgr/rdkStorageMain.cpp
storageMgrMain_SOURCES += ${top_srcdir}/refactored/rpc/rdkStorageMgr_iarm_internal.cpp
