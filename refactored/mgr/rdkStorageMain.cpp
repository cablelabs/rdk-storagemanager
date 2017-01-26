#include "rdkStorageMgr.h"
#include "rdkStorageMgrLogger.h"
#include "rdkStorageMain.h"
#include "rdkStorageBase.h"
#include "rdkStorageHardDisk.h"
#include "rdkStorageSDCard.h"
#include "rdkStorageNVRAM.h"
#include <unistd.h>

using namespace std;
void* healthMonitoringThreadfn  (void *pData);
rSTMgrMainClass* rSTMgrMainClass::g_instance = NULL;


rSTMgrMainClass* rSTMgrMainClass::getInstance()
{
    if (g_instance == NULL)
    {
        g_instance = new rSTMgrMainClass();
    }
    return g_instance;
}

rSTMgrMainClass::rSTMgrMainClass ()
{
    m_mainMutex = PTHREAD_MUTEX_INITIALIZER;
}

eSTMGRReturns rSTMgrMainClass::addNewMemoryDevice(std::string devicePath, eSTMGRDeviceType type)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_GENERIC_FAILURE;

    rStorageMedia *pMemoryObj = NULL;
    if (type == RDK_STMGR_DEVICE_TYPE_NVRAM)
    {
         pMemoryObj = new rStorageNVRAM (devicePath);
    }
    else if (type == RDK_STMGR_DEVICE_TYPE_HDD)
    {
        pMemoryObj = new rStorageHDDrive (devicePath);
    }
    else if (type == RDK_STMGR_DEVICE_TYPE_SDCARD)
    {
        pMemoryObj = new rStorageSDCard (devicePath);
    }
    else
    {
        STMGRLOG_ERROR ("Unsupprted Memory type.. \n");
    }

    if (pMemoryObj)
    {
        /* Protect the addition */
        pthread_mutex_lock(&m_mainMutex);

        m_storageDeviceObjects.insert ({devicePath, pMemoryObj});

        /* Protect the addition */
        pthread_mutex_unlock(&m_mainMutex);

        if (stmgrHealthMonitorTID != 0)
        {
            pthread_cancel (stmgrHealthMonitorTID);
        }
        /* Start the Polling thread which does the health Monitoring */
        pthread_create(&stmgrHealthMonitorTID, NULL, healthMonitoringThreadfn, NULL);

        rc = RDK_STMGR_RETURN_SUCCESS;
    }

    return rc;
}

eSTMGRReturns rSTMgrMainClass::deleteMemoryDevice(std::string key)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    /* Protect the addition */
    pthread_mutex_lock(&m_mainMutex);

    std::map <std::string, rStorageMedia*>  :: const_iterator eFound = m_storageDeviceObjects.find (key);

    if (eFound != m_storageDeviceObjects.end())
    {
        rStorageMedia *pTemp = eFound->second;

        /* Remove from MAP */
        m_storageDeviceObjects.erase(key);

        /* Delete it */
        delete pTemp;
        STMGRLOG_INFO ("Successfully found the entry and deleted.. \n");
    }
    else
        STMGRLOG_ERROR ("Failed to find the entry and nothing deleted.. \n");

    /* Protect the addition */
    pthread_mutex_unlock(&m_mainMutex);

    return rc;
}

rStorageMedia* rSTMgrMainClass::getMemoryDevice(std::string key)
{
    rStorageMedia *pTemp = NULL;
    /* Protect the addition */
    pthread_mutex_lock(&m_mainMutex);

    std::map <std::string, rStorageMedia*>  :: const_iterator eFound = m_storageDeviceObjects.find (key);

    if (eFound != m_storageDeviceObjects.end())
        pTemp = eFound->second;

    /* Protect the addition */
    pthread_mutex_unlock(&m_mainMutex);

    return pTemp;
}

void rSTMgrMainClass::notifyEvent(eSTMGREventMessage events)
{
    return;
}

void rSTMgrMainClass::enquireHealthInfo(void)
{
    pthread_mutex_lock(&m_mainMutex);

    rStorageMedia *pMemoryObj = NULL;
    for ( auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it )
    {
        pMemoryObj = it->second;
        pMemoryObj->doDeviceHealthQuery();
    }

    pthread_mutex_unlock(&m_mainMutex);
}

void rSTMgrMainClass::devHealthMonThreadEntryFunc(void)
{
    rStorageMedia *pMemoryObj = NULL;
    eSTMGRHealthInfo healthInfo;

    /* Get the periodic polling interval from config file; for now, its 10 mins */
    unsigned int timeout = 10 * 60;

    while (1)
    {
        /* Wait for timeout; */
        sleep (timeout);

        /* Populate the value */
        enquireHealthInfo();

        pthread_mutex_lock(&m_mainMutex);
        /* Do the verification of data n post events if needed; */
        for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
        {
            memset (&healthInfo, 0, sizeof(healthInfo));

            pMemoryObj = it->second;
            if (pMemoryObj->getHealth(&healthInfo) == RDK_STMGR_RETURN_SUCCESS)
            {
                /* Evaluate the possible failure and memory full condition and post event if needed */
            }
            else
            {
                STMGRLOG_INFO("Failed to get Health info for %s\n", it->first.c_str());
            }
        }
        pthread_mutex_unlock(&m_mainMutex);
    }

    return;
}

void* healthMonitoringThreadfn  (void *pData)
{
    pData = pData; 

    rSTMgrMainClass::getInstance()->devHealthMonThreadEntryFunc();
    return NULL;
}

eSTMGRReturns rSTMgrMainClass::getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs)
{
    rStorageMedia *pMemoryObj = NULL;
    eSTMGRDeviceIDs deviceIDs;
    int i = 0;
    memset (&deviceIDs, 0, sizeof(deviceIDs));

    if (!pDeviceIDs)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        pMemoryObj->getDeviceId(deviceIDs.m_deviceIDs[i++]);
        pDeviceIDs->m_numOfDevices = i;
    }
    pthread_mutex_unlock(&m_mainMutex);

    memcpy(pDeviceIDs, &deviceIDs, sizeof(deviceIDs)); 
    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rSTMgrMainClass::getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo)
{
    bool isFound = false;
    rStorageMedia *pMemoryObj = NULL;
    if ((!pDeviceID) || (!pDeviceInfo))
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        memset (pDeviceInfo, 0, sizeof(eSTMGRDeviceInfo));

        pMemoryObj = it->second;
        pMemoryObj->getDeviceInfo(pDeviceInfo);
        if ( 0 == strncmp (pDeviceInfo->m_deviceID, pDeviceID, RDK_STMGR_MAX_STRING_LENGTH))
        {
            STMGRLOG_INFO ("Found the matching Device..\n");
            isFound = true;
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isFound)
    {
        STMGRLOG_ERROR ("Given deviceID is not present\n");
        return RDK_STMGR_RETURN_GENERIC_FAILURE;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList)
{
    int i = 0;
    rStorageMedia *pMemoryObj = NULL;

    if (!pDeviceInfoList)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (pDeviceInfoList, 0, sizeof(eSTMGRDeviceInfoList));

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        pMemoryObj->getDeviceInfo(&pDeviceInfoList->m_devices[i++]);
        pDeviceInfoList->m_numOfDevices = i;
    }
    pthread_mutex_unlock(&m_mainMutex);
    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    rStorageMedia *pMemoryObj = NULL;
    char deviceID[RDK_STMGR_MAX_STRING_LENGTH] = "";

    if ((!pDeviceID) || (!pPartitionId) || (!pPartitionInfo))
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        memset (&deviceID, 0, sizeof(deviceID));

        pMemoryObj = it->second;
        pMemoryObj->getDeviceId(deviceID);
        if ( 0 == strncmp (deviceID, pDeviceID, RDK_STMGR_MAX_STRING_LENGTH))
        {
            STMGRLOG_INFO ("Found the matching Device..\n");
            /* Get the partition info */
            if (RDK_STMGR_RETURN_SUCCESS != pMemoryObj->getPartitionInfo(pPartitionId, pPartitionInfo))
            {
                STMGRLOG_ERROR ("Given partitionID is not present\n");

                pthread_mutex_unlock(&m_mainMutex);
                return RDK_STMGR_RETURN_GENERIC_FAILURE;
            }
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pTSBStatus)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pTSBStatus, 0, sizeof(eSTMGRTSBStatus));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getTSBStatus(pTSBStatus);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports TSB, we return that TSB is disabled; not failed\n");
        *pTSBStatus = RDK_STMGR_TSB_STATUS_DISABLED;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::setTSBMaxMinutes (unsigned int minutes)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            if (RDK_STMGR_RETURN_SUCCESS != pMemoryObj->setTSBMaxMinutes(minutes))
            {
                STMGRLOG_ERROR ("Set TSB Max mins failed\n");
            }
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getTSBMaxMinutes (unsigned int *pMinutes)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pMinutes)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pMinutes, 0, sizeof(unsigned int));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getTSBMaxMinutes(pMinutes);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports TSB, we return that TSB is disabled; not failed\n");
        *pMinutes = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getTSBCapacityMinutes(unsigned int *pMinutes)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pMinutes)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pMinutes, 0, sizeof(unsigned int));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getTSBCapacityMinutes(pMinutes);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports TSB, we return that TSB is disabled; not failed\n");
        *pMinutes = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getTSBCapacity(unsigned long *pCapacityInKB)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pCapacityInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pCapacityInKB, 0, sizeof(unsigned long));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getTSBCapacity(pCapacityInKB);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports TSB, we return that TSB is disabled; not failed\n");
        *pCapacityInKB = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getTSBFreeSpace(unsigned long *pFreeSpaceInKB)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pFreeSpaceInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pFreeSpaceInKB, 0, sizeof(unsigned long));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getTSBFreeSpace(pFreeSpaceInKB);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports TSB, we return that TSB is disabled; not failed\n");
        *pFreeSpaceInKB = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getDVRCapacity(unsigned long *pCapacityInKB)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pCapacityInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pCapacityInKB, 0, sizeof(unsigned long));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isDVRSupported();

        /* We just take it from the first DVR Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getDVRCapacity(pCapacityInKB);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports DVR, we return that DVR is disabled; not failed\n");
        *pCapacityInKB = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

eSTMGRReturns rSTMgrMainClass::getDVRFreeSpace(unsigned long *pFreeSpaceInKB)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;

    if (!pFreeSpaceInKB)
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    memset (&pFreeSpaceInKB, 0, sizeof(unsigned long));
    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isDVRSupported();

        /* We just take it from the first DVR Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            pMemoryObj->getDVRFreeSpace(pFreeSpaceInKB);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isSupported)
    {
        STMGRLOG_ERROR ("As NO memory device found that supports DVR, we return that DVR is disabled; not failed\n");
        *pFreeSpaceInKB = 0;
    }

    return RDK_STMGR_RETURN_SUCCESS; 
}

bool rSTMgrMainClass::isTSBEnabled(void)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;
    bool tsbEnabled = false;

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        /* We just take it from the first TSB Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            tsbEnabled = pMemoryObj->isTSBEnabled();
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    return tsbEnabled;
}

bool rSTMgrMainClass::isDVREnabled(void)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;
    bool dvrEnabled = false;

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isDVRSupported();

        /* We just take it from the first DVR Supported Memory device as of now.. we will workout extending the logic/API for device ID based */
        if (isSupported)
        {
            dvrEnabled = pMemoryObj->isDVREnabled();
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    return dvrEnabled;
}

eSTMGRReturns rSTMgrMainClass::setTSBEnabled (bool isEnabled)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;
    bool isFound = false;

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isTSBSupported();

        if (isSupported)
        {
            isFound = true;
            pMemoryObj->setTSBEnabled(isEnabled);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isFound)
    {
        STMGRLOG_ERROR ("No device is found TSB supporting\n");
        return RDK_STMGR_RETURN_GENERIC_FAILURE;
    }

    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rSTMgrMainClass::setDVREnabled (bool isEnabled)
{
    rStorageMedia *pMemoryObj = NULL;
    bool isSupported = false;
    bool isFound = false;

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        isSupported = pMemoryObj->isDVRSupported();
        if (isSupported)
        {
            isFound = true;
            pMemoryObj->setDVREnabled(isEnabled);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    if (!isFound)
    {
        STMGRLOG_ERROR ("No device is found DVR supporting\n");
        return RDK_STMGR_RETURN_GENERIC_FAILURE;
    }

    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rSTMgrMainClass::getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo)
{
    rStorageMedia *pMemoryObj = NULL;
    char deviceID[RDK_STMGR_MAX_STRING_LENGTH] = "";
    if ((!pDeviceID) || (!pHealthInfo))
    {
        STMGRLOG_ERROR ("NULL Pointer input\n");
        return RDK_STMGR_RETURN_INVALID_INPUT;
    }

    pthread_mutex_lock(&m_mainMutex);
    for (auto it = m_storageDeviceObjects.begin(); it != m_storageDeviceObjects.end(); ++it)
    {
        pMemoryObj = it->second;
        pMemoryObj->getDeviceId(deviceID);
        if ( 0 == strncmp (deviceID, pDeviceID, RDK_STMGR_MAX_STRING_LENGTH))
        {
            STMGRLOG_INFO ("Found the matching Device..\n");
            pMemoryObj->getHealth(pHealthInfo);
            break;
        }
    }
    pthread_mutex_unlock(&m_mainMutex);

    return RDK_STMGR_RETURN_SUCCESS; 
}

/* End of the File */
