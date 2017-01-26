if USE_REFACTORED
AM_CPPFLAGS = -Wall -Werror -I${top_srcdir}/refactored/src/main -I${top_srcdir}/refactored/include -I=${includedir}/rdk/iarmbus
AM_LDFLAGS = $(GLIB_LDFLAGS) $(GLIB_LIBS) $(G_THREAD_LIBS) -ldbus-1  -lIARMBus -lrdkloggers

lib_LTLIBRARIES = librdkstmgr.la
librdkstmgr_la_SOURCES = rdkStorageMgr_iarm_external.cpp
librdkstmgr_la_includedir = $(includedir)
librdkstmgr_la_include_HEADERS  = ${top_srcdir}/refactored/include/rdkStorageMgr.h
librdkstmgr_la_include_HEADERS += ${top_srcdir}/refactored/include/rdkStorageMgrTypes.h
endif
