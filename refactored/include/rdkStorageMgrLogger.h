/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
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
