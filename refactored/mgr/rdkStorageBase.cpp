#include "rdkStorageBase.h"
#include "rdkStorageMgrLogger.h"
#include <string.h>
#include <stdlib.h>

using namespace std;

rStorageMedia::rStorageMedia()
    : m_devicePath("")
    , m_type(RDK_STMGR_DEVICE_TYPE_MAX)
    , m_capacity (0)
    , m_status(RDK_STMGR_DEVICE_STATUS_UNKNOWN)
    , m_hasSMARTSupport(false)
    , m_tsbStatus(RDK_STMGR_TSB_STATUS_UNKNOWN)
    , m_maxTSBCapacityinMinutes(0)
    , m_maxTSBLengthConfigured(0)
    , m_maxTSBCapacityinKB(0)
    , m_freeTSBSpaceLeftinKB(0)
    , m_maxDVRCapacityinKB(0)
    , m_freeDVRSpaceLeftinKB(0)
    , m_isTSBEnabled(false)
    , m_isDVREnabled(false)
    , m_isTSBSupported(false)
    , m_isDVRSupported(false)
{
    memset (&m_manufacturer, 0, sizeof(m_manufacturer));
    memset (&m_model, 0, sizeof(m_model));
    memset (&m_serialNumber, 0, sizeof(m_serialNumber));
    memset (&m_firmwareVersion, 0, sizeof(m_firmwareVersion));
    memset (&m_ifATAstandard, 0, sizeof(m_ifATAstandard));
    memset (&m_hwVersion, 0, sizeof(m_hwVersion));
}

rStorageMedia::~rStorageMedia()
{
    /* Delete the partition class */
    for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
    {
        rStoragePartition* pTemp = it->second;
        delete pTemp;
    }

    /* Empty the partition class */
    m_partitionInfo.clear();
}

eSTMGRReturns rStorageMedia::getDeviceId(char* pDeviceID)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (pDeviceID)
    {
        memcpy (pDeviceID, &m_deviceID, (RDK_STMGR_MAX_STRING_LENGTH - 1));
    }
    else
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getDeviceInfo(eSTMGRDeviceInfo* pDeviceInfo)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (pDeviceInfo)
    {
        /* Clear the incoming */
        memset (pDeviceInfo, 0, sizeof (eSTMGRDeviceInfo));

        /* Copy over */
        memcpy (&pDeviceInfo->m_deviceID, m_deviceID, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        pDeviceInfo->m_type = m_type;
        pDeviceInfo->m_capacity = m_capacity;
        pDeviceInfo->m_status = m_status;
        pDeviceInfo->m_hasSMARTSupport = m_hasSMARTSupport;
        strncpy (pDeviceInfo->m_manufacturer, m_manufacturer, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        strncpy (pDeviceInfo->m_model, m_model, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        strncpy (pDeviceInfo->m_serialNumber, m_serialNumber, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        strncpy (pDeviceInfo->m_firmwareVersion, m_firmwareVersion, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        strncpy (pDeviceInfo->m_ifATAstandard, m_ifATAstandard, (RDK_STMGR_MAX_STRING_LENGTH - 1));
        strncpy (pDeviceInfo->m_hwVersion, m_hwVersion, (RDK_STMGR_MAX_STRING_LENGTH - 1));

        int length = m_partitionInfo.size();
        int loopCnt = 0;
        for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
        {
            loopCnt += 1;
            rStoragePartition* pTemp = it->second;
            strncat (pDeviceInfo->m_partitions, pTemp->m_partitionId, (RDK_STMGR_MAX_STRING_LENGTH - 1));

            if (loopCnt != length)
                strcat (pDeviceInfo->m_partitions, ";");
        }
    }
    else
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getPartitionInfo (char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (pPartitionId)
    {
        /* Clear the incoming */
        memset (pPartitionInfo, 0, sizeof (eSTMGRPartitionInfo));

        std::string key = pPartitionId;
        rStoragePartition *pTemp = NULL;
        std::map <std::string, rStoragePartition*> :: const_iterator eFound = m_partitionInfo.find (key);
        if (eFound == m_partitionInfo.end())
        {
            STMGRLOG_ERROR ("Partition ID is not present %s\n", __FUNCTION__);
            rc = RDK_STMGR_RETURN_INVALID_INPUT;
        }
        else
        {
            pTemp = eFound->second;

            /* Copy over */
            strncpy (pPartitionInfo->m_partitionId, pTemp->m_partitionId, (RDK_STMGR_MAX_STRING_LENGTH - 1));
            strncpy (pPartitionInfo->m_format, pTemp->m_format, (RDK_STMGR_MAX_STRING_LENGTH - 1));
            pPartitionInfo->m_capacity = pTemp->m_capacityinKB;
            pPartitionInfo->m_freeSpace = pTemp->m_freeSpaceinKB;
            pPartitionInfo->m_status = pTemp->m_status;
            pPartitionInfo->m_isDVRSupported = pTemp->m_isDVRSupported;
            pPartitionInfo->m_isTSBSupported = pTemp->m_isTSBSupported;
            STMGRLOG_INFO ("copied all the data at %s\n", __FUNCTION__);
        }
    }
    else
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;

    if (pTSBStatus)
    {
        *pTSBStatus = m_tsbStatus;
    }
    else
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::setTSBMaxMinutes (unsigned int minutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (minutes >= m_maxTSBCapacityinMinutes)
    {
        STMGRLOG_ERROR ("Invalid input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        if (m_isTSBSupported)
            m_maxTSBLengthConfigured = minutes;
        else
        {
            STMGRLOG_WARN ("TSB Support is not present in the memory deivice\n");
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getTSBMaxMinutes (unsigned int *pMinutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pMinutes)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pMinutes = m_maxTSBLengthConfigured;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getTSBCapacityMinutes(unsigned int *pMinutes)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pMinutes)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pMinutes = m_maxTSBCapacityinMinutes;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getTSBCapacity(unsigned long *pCapacityInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pCapacityInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pCapacityInKB = m_maxTSBCapacityinKB;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getTSBFreeSpace(unsigned long *pFreeSpaceInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pFreeSpaceInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pFreeSpaceInKB = m_freeTSBSpaceLeftinKB;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getDVRCapacity(unsigned long *pCapacityInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pCapacityInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pCapacityInKB = m_maxDVRCapacityinKB;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::getDVRFreeSpace(unsigned long *pFreeSpaceInKB)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (!pFreeSpaceInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else
    {
        *pFreeSpaceInKB = m_freeDVRSpaceLeftinKB;
    }
    return rc;
}

eSTMGRReturns rStorageMedia::setTSBEnabled (bool isEnabled)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    rStoragePartition* pTemp = NULL;

    /* The populateDeviceDetails must ensure that the m_isTSBSupported is updated */
    if (!m_isTSBSupported)
    {
        bool isFound = false;
        for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
        {
            pTemp = it->second;
            if (pTemp->m_isTSBSupported)
            {
                isFound = true;
                break;
            }
        }

        if ((isFound) && (pTemp != NULL))
        {
            STMGRLOG_ERROR ("Found a partition (partitonID = %s) in this memory (deviceID = %s) that supports TSB.\n", pTemp->m_partitionId, m_deviceID);
            m_isTSBEnabled = isEnabled;
        }
        else
        {
            STMGRLOG_ERROR ("Not found any partition of the memory that supports TSB\n");
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Found that this memory (deviceID = %s) is supporting TSB.\n", m_deviceID);
        m_isTSBEnabled = isEnabled;
    }

    return rc;
}

eSTMGRReturns rStorageMedia::setDVREnabled (bool isEnabled)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    rStoragePartition* pTemp = NULL;

    /* The populateDeviceDetails must ensure that the m_isDVRSupported is updated */
    if (!m_isDVRSupported)
    {
        bool isFound = false;
        for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
        {
            pTemp = it->second;
            if (pTemp->m_isDVRSupported)
            {
                isFound = true;
                break;
            }
        }

        if ((isFound) && (pTemp != NULL))
        {
            STMGRLOG_ERROR ("Found a partition (partitonID = %s) in this memory (deviceID = %s) that supports DVR.\n", pTemp->m_partitionId, m_deviceID);
            m_isDVREnabled = isEnabled;
        }
        else
        {
            STMGRLOG_ERROR ("Not found any partition of the memory that supports DVR\n");
            rc = RDK_STMGR_RETURN_GENERIC_FAILURE;
        }
    }
    else
    {
        STMGRLOG_ERROR ("Found that this memory (deviceID = %s) is supporting DVR.\n", m_deviceID);
        m_isDVREnabled = isEnabled;
    }
    return rc;
}

bool rStorageMedia::isTSBEnabled()
{
    return m_isTSBEnabled;
}

bool rStorageMedia::isDVREnabled()
{
    return m_isDVREnabled;
}

bool rStorageMedia::isTSBSupported()
{
    return m_isTSBSupported;
}
bool rStorageMedia::isDVRSupported()
{
    return m_isDVRSupported;
}

eSTMGRReturns rStorageMedia::getHealth (eSTMGRHealthInfo* pHealthInfo)
{
    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rStorageMedia::getTSBPartitionMountPath(char* pMountPath)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;

    if (!pMountPath)
    {
        STMGRLOG_ERROR ("NULL Pointer input at %s\n", __FUNCTION__);
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }
    else if (m_isTSBEnabled)
    {
        for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
        {
            rStoragePartition* pTemp = it->second;
            if (pTemp->m_isTSBSupported)
            {
                strcpy (pMountPath, pTemp->m_mountPath);
                break;
            }
        }
    }
    else
    {
        STMGRLOG_ERROR ("Found that this storage device (deviceID = %s) is not supporting TSB\n", m_deviceID);
    }

    return rc;
}

eSTMGRReturns rStorageMedia::registerEventCallback(fnSTMGR_EventCallback eventCallback)
{
    STMGRLOG_INFO("ENTRY of %s\n", __FUNCTION__);

    if (eventCallback != NULL)
    {
        m_eventCallback = eventCallback;
        return RDK_STMGR_RETURN_SUCCESS;
    }
    else
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }
}

void rStorageMedia::notifyEvent(eSTMGREventMessage events)
{
    if (m_eventCallback)
        m_eventCallback(events);

    return;
}

/* End of file */
