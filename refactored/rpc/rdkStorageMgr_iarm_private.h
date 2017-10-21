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
#ifndef __RDK_STMGR_IARM_INTERFACE__
#define __RDK_STMGR_IARM_INTERFACE__

#include "libIBus.h"
#include "libIARM.h"

#define IARM_BUS_STMGR_NAME     "rSTMgrBus"


typedef enum _stmgr_iarm_events {
    RDK_STMGR_IARM_EVENT_STATUS_CHANGED = 100,
    RDK_STMGR_IARM_EVENT_TSB_ERROR,
    RDK_STMGR_IARM_EVENT_HEALTH_WARNING,
    RDK_STMGR_IARM_EVENT_DEVICE_FAILURE,
    RDK_STMGR_IARM_EVENT_MAX
} eSTMGREvents_iarm_t;


typedef struct _stmgr_iarm_deviceinfo {
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRDeviceInfo m_deviceInfo;
} eSTMGRDeviceInfo_iarm_t;

typedef struct _stmgr_iarm_partitioninfo {
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    char m_partitionID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRPartitionInfo m_partitionInfo;
} eSTMGRPartitionInfo_iarm_t;

typedef struct _stmgr_iarm_healthinfo {
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRHealthInfo m_healthInfo;
} eSTMGRHealthInfo_iarm_t;
#endif  /* __RDK_STMGR_IARM_INTERFACE__ */
