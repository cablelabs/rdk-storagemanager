#include "rdkStorageBase.h"
#include "rdkStorageMgrLogger.h"
#include <string.h>
#include <stdlib.h>

using namespace std;

rStorageMedia::rStorageMedia()
{
    populateDeviceDetails();
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

eSTMGRReturns rStorageMedia::populateDeviceDetails()
{
    abort();
    return RDK_STMGR_RETURN_SUCCESS;
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
        m_maxTSBLengthConfigured = minutes;
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
    /* Loop thro' the partitions and if any partition supports TSB, take this value. */
    m_isTSBEnabled = isEnabled;
    return rc;
}

eSTMGRReturns rStorageMedia::setDVREnabled (bool isEnabled)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    /* Loop thro' the partitions and if any partition supports DVR, take this value. */
    m_isDVREnabled = isEnabled;
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


/* End of file */
