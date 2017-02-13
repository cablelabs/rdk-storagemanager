bin_PROGRAMS = rdkStorageTest
rdkStorageTest_SOURCES = rdkStorageTest.cpp
rdkStorageTest_LDADD     = $(top_builddir)/refactored/rpc/librdkstmgr.la

AM_CPPFLAGS = -Wall -Werror -I${top_srcdir}/refactored/include -I=${includedir}/rdk/iarmbus
AM_LDFLAGS = -ldbus-1  -lIARMBus

#This is hack to keep the builds not breaking during the integration of new refactored code.
include_HEADERS = ${top_srcdir}/src/rpc/include/storageMgr.h
