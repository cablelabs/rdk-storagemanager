#ifndef __RDK_STMGR_IARM_INTERFACE__
#define __RDK_STMGR_IARM_INTERFACE__

#include "libIBus.h"
#include "libIARM.h"

#define IARM_BUS_STMGR_NAME     "rSTMgrBus"


typedef enum _stmgr_iarm_events {
    RDK_STMGR_IARM_EVENT_STATUS_CHANGED = 0,
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
