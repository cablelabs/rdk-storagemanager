#include "rdkStorageMgr.h"
#include "rdkStorageMgrTypes.h"
#include "rdkStorageMgrLogger.h"
#include "rdkStorageMgr_iarm_private.h"

/* For file access */
#include <sys/types.h>
#include <unistd.h>

/* statics */
static bool gIsSTMGRInited = false;
static fnSTMGR_EventCallback g_eventCallbackFunction = NULL;

/* Consts */
const char* STMGR_IARM_DEBUG_ACTUAL_PATH    = "/etc/debug.ini";
const char* STMGR_IARM_DEBUG_OVERRIDE_PATH  = "/opt/debug.ini";

/* local functions */
static void _stmgr_deviceCallback(const char *owner, IARM_EventId_t eventId, void *pData, size_t len)
{
    if (NULL == pData)
    {
        STMGRLOG_ERROR ("%s : Input is invalid\n", __FUNCTION__);
    }
    else if (g_eventCallbackFunction)
    {
        eSTMGREventMessage *pReceivedEvent = (eSTMGREventMessage*)pData;
        eSTMGREventMessage newEvent;

        if ((RDK_STMGR_IARM_EVENT_STATUS_CHANGED == eventId)   ||
            (RDK_STMGR_IARM_EVENT_TSB_ERROR == eventId)        ||
            (RDK_STMGR_IARM_EVENT_HEALTH_WARNING == eventId)   ||
            (RDK_STMGR_IARM_EVENT_DEVICE_FAILURE == eventId))
        {
            memcpy (&newEvent, pReceivedEvent, sizeof(eSTMGREventMessage));
            g_eventCallbackFunction (newEvent);
        }
        else
        {
            STMGRLOG_ERROR ("%s : Event is invalid\n", __FUNCTION__);
        }
    }
    else
        STMGRLOG_ERROR ("%s : No Event Callback registered\n", __FUNCTION__);

    return;
}

/* Wrapper to do the force init for LOGGING sake */
bool _stmgr_isSTMGRInited()
{
    if (!gIsSTMGRInited)
    {
        STMGRLOG_WARN ("%s : Not done Init yet; So doing it now to keep the APIs going\n", __FUNCTION__);
        rdkStorage_init();
    }

    return true;
}

/* Public functions */
void rdkStorage_init (void)
{
    const char* pDebugConfig = NULL;

    if (!gIsSTMGRInited)
    {
        gIsSTMGRInited = true;

        /* Init the logger */
        if( access(STMGR_IARM_DEBUG_OVERRIDE_PATH, F_OK) != -1 )
            pDebugConfig = STMGR_IARM_DEBUG_OVERRIDE_PATH;
        else
            pDebugConfig = STMGR_IARM_DEBUG_ACTUAL_PATH;

        rdk_logger_init(pDebugConfig);
        
        IARM_Bus_RegisterEventHandler(IARM_BUS_STMGR_NAME, RDK_STMGR_IARM_EVENT_STATUS_CHANGED, _stmgr_deviceCallback);
        IARM_Bus_RegisterEventHandler(IARM_BUS_STMGR_NAME, RDK_STMGR_IARM_EVENT_TSB_ERROR,      _stmgr_deviceCallback);
        IARM_Bus_RegisterEventHandler(IARM_BUS_STMGR_NAME, RDK_STMGR_IARM_EVENT_HEALTH_WARNING, _stmgr_deviceCallback);
        IARM_Bus_RegisterEventHandler(IARM_BUS_STMGR_NAME, RDK_STMGR_IARM_EVENT_DEVICE_FAILURE, _stmgr_deviceCallback);

        STMGRLOG_INFO("IARM Interface Inited Successfully\n");
    }
    else
        STMGRLOG_INFO("IARM Interface already Inited Successfully\n");

    return;
}

/* Get DeviceIDs*/
eSTMGRReturns rdkStorage_getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRDeviceIDs deviceIDs;
    memset (&deviceIDs, 0, sizeof(deviceIDs));

    if (!pDeviceIDs)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetDeviceIDs", (void *)&deviceIDs, sizeof(deviceIDs));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pDeviceIDs, &deviceIDs, sizeof(deviceIDs));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    return rc;
}

/* Get DeviceInfo */
eSTMGRReturns rdkStorage_getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo )
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRDeviceInfo_iarm_t deviceInfo;
    memset (&deviceInfo, 0, sizeof(deviceInfo));


    if ((!pDeviceID) || (!pDeviceInfo))
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        memcpy(&deviceInfo.m_deviceID, pDeviceID, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetDeviceInfo", (void *)&deviceInfo, sizeof(deviceInfo));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pDeviceInfo, &deviceInfo.m_deviceInfo, sizeof(eSTMGRDeviceInfo));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get DeviceInfoList */
eSTMGRReturns rdkStorage_getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRDeviceInfoList deviceInfoList;
    memset (&deviceInfoList, 0, sizeof(deviceInfoList));


    if (!pDeviceInfoList)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetDeviceInfoList", (void *)&deviceInfoList, sizeof(deviceInfoList));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pDeviceInfoList, &deviceInfoList, sizeof(eSTMGRDeviceInfoList));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}


/* Get PartitionInfo */
eSTMGRReturns rdkStorage_getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRPartitionInfo_iarm_t partitionInfo;
    memset (&partitionInfo, 0, sizeof(partitionInfo));

    if ((!pDeviceID) || (!pPartitionId) || (!pPartitionInfo))
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        memcpy(&partitionInfo.m_deviceID, pDeviceID, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        memcpy(&partitionInfo.m_partitionID, pPartitionId, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetPartitionInfo", (void *)&partitionInfo, sizeof(partitionInfo));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pPartitionInfo, &partitionInfo.m_partitionInfo, sizeof(eSTMGRPartitionInfo));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }

    return rc;
}

/* Get TSBStatus */
eSTMGRReturns rdkStorage_getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRTSBStatus status;

    if (!pTSBStatus)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBStatus", (void *)&status, sizeof(status));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pTSBStatus = status;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Set TSBMaxMinutes */
eSTMGRReturns rdkStorage_setTSBMaxMinutes (unsigned int minutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "SetTSBMaxMinutes", (void *)&minutes, sizeof(minutes));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get TSBMaxMinutes */
eSTMGRReturns rdkStorage_getTSBMaxMinutes (unsigned int *pMinutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned int minutes;

    if (!pMinutes)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBMaxMinutes", (void *)&minutes, sizeof(minutes));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pMinutes = minutes;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get TSBCapacityMinutes */
eSTMGRReturns rdkStorage_getTSBCapacityMinutes(unsigned int *pMinutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned int minutes;

    if (!pMinutes)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBCapacityInMinutes", (void *)&minutes, sizeof(minutes));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pMinutes = minutes;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get TSBCapacity*/
eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long *pCapacityInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned long capacity;

    if (!pCapacityInKB)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBCapacityInKB", (void *)&capacity, sizeof(capacity));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pCapacityInKB = capacity;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get TSBFreeSpace*/
eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long *pFreeSpaceInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned long freespace;

    if (!pFreeSpaceInKB)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBFreeSpaceInKB", (void *)&freespace, sizeof(freespace));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pFreeSpaceInKB = freespace;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get DVRCapacity */
eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long *pCapacityInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned long capacity;

    if (!pCapacityInKB)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetDVRCapacityInKB", (void *)&capacity, sizeof(capacity));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pCapacityInKB = capacity;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get DVRFreeSpace*/
eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long *pFreeSpaceInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    unsigned long freespace;

    if (!pFreeSpaceInKB)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetDVRFreeSpaceInKB", (void *)&freespace, sizeof(freespace));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            *pFreeSpaceInKB = freespace;
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

/* Get isTSBEnabled*/
bool rdkStorage_isTSBEnabled()
{
    bool rc = false;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetIsTSBEnabled", (void *)&rc, sizeof(rc));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
    }

    return rc;
}

/* Set TSBEnabled */
eSTMGRReturns rdkStorage_setTSBEnabled (bool isEnabled)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "SetIsTSBEnabled", (void *)&isEnabled, sizeof(isEnabled));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    return rc;
}

/* Get isDVREnabled*/
bool rdkStorage_isDVREnabled()
{
    bool rc = false;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetIsDVREnabled", (void *)&rc, sizeof(rc));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
    }

    return rc;
}

/* Set DVREnabled */
eSTMGRReturns rdkStorage_setDVREnabled (bool isEnabled)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "SetIsDVREnabled", (void *)&isEnabled, sizeof(isEnabled));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    return rc;
}

/* Get Health */
eSTMGRReturns rdkStorage_getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    eSTMGRHealthInfo_iarm_t healthInfo;
    memset (&healthInfo, 0, sizeof(healthInfo));

    if ((!pDeviceID) || (!pHealthInfo))
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        memcpy(&healthInfo.m_deviceID, pDeviceID, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetHealthInfo", (void *)&healthInfo, sizeof(healthInfo));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pHealthInfo, &healthInfo.m_healthInfo, sizeof(eSTMGRHealthInfo));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

eSTMGRReturns rdkStorage_getTSBPartitionMountPath (char* pMountPath)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    char mountPath[RDK_STMGR_MAX_STRING_LENGTH] = "";
    memset (&mountPath, 0, sizeof(mountPath));

    if (pMountPath)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "GetTSBPartitionMountPath", (void *)&mountPath, sizeof(mountPath));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            /* Copy over the data */
            memcpy (pMountPath, &mountPath, sizeof(mountPath));
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;

    }
    return rc;
}

void rdkStorage_notifyMGRAboutFailure (eSTMGRErrorEvent failEvent)
{
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;
    if (_stmgr_isSTMGRInited())
    {
        retCode = IARM_Bus_Call(IARM_BUS_STMGR_NAME, "NotifyMGRAboutFailure", (void *)&failEvent, sizeof(failEvent));
        if (IARM_RESULT_SUCCESS == retCode)
        {
            STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
        }
        else
        {
            STMGRLOG_ERROR ("%s The IARM communication failed with rc = %d\n", __FUNCTION__, retCode);
        }
    }
    else
    {
        STMGRLOG_ERROR ("Init is not done yet or failed init.\n");

    }
    return;
}

eSTMGRReturns rdkStorage_RegisterEventCallback(fnSTMGR_EventCallback eventCallback)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!eventCallback)
    {
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
        STMGRLOG_ERROR ("%s : Input is invalid\n", __FUNCTION__);
    }
    else
    {
        g_eventCallbackFunction = eventCallback;
        STMGRLOG_INFO ("%s : Successfully finished \n", __FUNCTION__);
    }
    return rc;
}

