#ifndef __RDK_STORAGE_MGR_LOGGER_H__
#define __RDK_STORAGE_MGR_LOGGER_H__

#include "rdk_debug.h"
#include "stdlib.h"
#include "string.h"

#if 1

#define STMGRLOG_ERROR(format...)       RDK_LOG(RDK_LOG_ERROR,  "LOG.RDK.STORAGEMGR", format)
#define STMGRLOG_WARN(format...)        RDK_LOG(RDK_LOG_WARN,   "LOG.RDK.STORAGEMGR", format)
#define STMGRLOG_INFO(format...)        RDK_LOG(RDK_LOG_INFO,   "LOG.RDK.STORAGEMGR", format)
#define STMGRLOG_DEBUG(format...)       RDK_LOG(RDK_LOG_DEBUG,  "LOG.RDK.STORAGEMGR", format)
#define STMGRLOG_TRACE(format...)       RDK_LOG(RDK_LOG_TRACE1, "LOG.RDK.STORAGEMGR", format)

#else

#define STMGRLOG_ERROR(format...)       printf (format)
#define STMGRLOG_WARN(format...)        printf (format)
#define STMGRLOG_INFO(format...)        printf (format)
#define STMGRLOG_DEBUG(format...)       printf (format)
#define STMGRLOG_TRACE(format...)       printf (format)

#endif

#endif /* __RDK_STORAGE_MGR_LOGGER_H__ */
