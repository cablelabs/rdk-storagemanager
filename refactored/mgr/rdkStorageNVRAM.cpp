#include "rdkStorageNVRAM.h"
#include "rdkStorageMgrLogger.h"
#include <fstream>

#define RDK_STMGR_NVRAM_EC_MAX      100000     /*<! SLC NAND theoretically are good for 100,000 cycles.*/
#define RDK_STMGR_NVRAM_EC_LIMIT     99000     /*<! Limiting 1000 cycles less to notify that its about to die */
#define RDK_STMGR_NVRAM_TMP_FILE    "/tmp/rStorageNVRAM.file"
#define RDK_STMGR_NVRAM_LOG_FILE    "/tmp/rStorageNVRAM.file"

rStorageNVRAM::rStorageNVRAM(std::string devicePath)
{
    m_pUDevNVRAM = udev_new();

    m_devicePath = devicePath;
    m_type = RDK_STMGR_DEVICE_TYPE_NVRAM;
    m_parentDevice = NULL;
    m_eraseCount = RDK_STMGR_NVRAM_EC_MAX;

    int l = strlen ("/dev/ubi");
    int id = m_devicePath.at(l);
    sprintf (m_deviceID, "%d", id);
}

/* Populates all the device data base */
eSTMGRReturns rStorageNVRAM::populateDeviceDetails()
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;

    /* Get the Manufacturer detials */
    {
        readChipDetails();
    }

    if (!m_pUDevNVRAM)
    {
        STMGRLOG_ERROR ("NVRAM Storage Class: Could not able to create udev instance of its own.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    else
    {
        struct udev_device *pDevice;
        struct udev_enumerate *pEnumerate = NULL;
        struct udev_list_entry *pDeviceList = NULL;
        struct udev_list_entry *pDeviceListEntry = NULL;

        pEnumerate = udev_enumerate_new(m_pUDevNVRAM);
        udev_enumerate_add_match_subsystem(pEnumerate, "ubi");
        if (0 != udev_enumerate_scan_devices(pEnumerate))
        {
            STMGRLOG_ERROR("Failed to scan the devices \n");
        }

        pDeviceList = udev_enumerate_get_list_entry(pEnumerate);
        udev_list_entry_foreach(pDeviceListEntry, pDeviceList)
        {
            const char* pSysPath = NULL;
            const char* pDeviceType = NULL;

            STMGRLOG_ERROR("Its a loop of scanned devices \n");
            pSysPath = udev_list_entry_get_name(pDeviceListEntry);
            pDevice = udev_device_new_from_syspath(m_pUDevNVRAM, pSysPath);
            if (pDevice)
            {
                pDeviceType = udev_device_get_sysattr_value(pDevice, "type");
                if (pDeviceType && (strcmp (pDeviceType, "dynamic") == 0))
                {
                    STMGRLOG_DEBUG ("Type of Device: %s\n", pDeviceType);
                    if (!m_parentDevice)
                    {
                        m_parentDevice = udev_device_get_parent (pDevice);
                        readEraseCount();
                    }
                    /* Create Partition class n update it */
                    rStoragePartition *pPartitionPtr = new rStoragePartition;
                    if (pPartitionPtr)
                    {
                        const char *pDevicePartitionID = udev_device_get_devnode(pDevice);

                        if (pDevicePartitionID)
                        {
                            STMGRLOG_INFO ("The Partition Id is, %s\n", pDevicePartitionID);
                            sprintf (pPartitionPtr->m_partitionId, "%s", pDevicePartitionID);
                            sprintf(pPartitionPtr->m_format, "ubi");

                            const char *pCapacity = udev_device_get_sysattr_value(pDevice, "data_bytes");

                            if (pCapacity)
                            {
                                pPartitionPtr->m_capacityinKB = (unsigned long) (atol(pCapacity)/1024);
                            }
                            /* FIXME: Read the freespace from statvfs !?! */
                            pPartitionPtr->m_freeSpaceinKB = pPartitionPtr->m_capacityinKB;
                            pPartitionPtr->m_status = RDK_STMGR_DEVICE_STATUS_OK;
                            pPartitionPtr->m_isTSBSupported = false;
                            pPartitionPtr->m_isDVRSupported = false;
                            m_partitionInfo.insert({pDevicePartitionID, pPartitionPtr});
                        }
                        else
                        {
                            STMGRLOG_ERROR ("Could not find the device partition id..\n");
                            delete pPartitionPtr;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/* Queries the Device for the health info */
eSTMGRReturns rStorageNVRAM::doDeviceHealthQuery(void)
{
    readEraseCount();

    /* Update m_healthInfo; */
    memset (&m_healthInfo, 0 ,sizeof(m_healthInfo));

    if ((m_eraseCount > 0) && (m_eraseCount <= RDK_STMGR_NVRAM_EC_LIMIT))
    {
        m_healthInfo.m_isHealthy = true;
        m_healthInfo.m_isOperational = true;
    }
    else if ((m_eraseCount >= RDK_STMGR_NVRAM_EC_LIMIT) && (m_eraseCount <= RDK_STMGR_NVRAM_EC_MAX))
    {
        m_healthInfo.m_isHealthy = false;
        m_healthInfo.m_isOperational = true;
    }
    else
    {
        m_healthInfo.m_isHealthy = false;
        m_healthInfo.m_isOperational = false;
    }
    strncpy(m_healthInfo.m_deviceID, m_deviceID, RDK_STMGR_MAX_STRING_LENGTH);
    m_healthInfo.m_deviceType = RDK_STMGR_DEVICE_TYPE_NVRAM;
 
    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rStorageNVRAM::getHealth (eSTMGRHealthInfo* pHealthInfo)
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    if (pHealthInfo)
    {
        memset(pHealthInfo, 0, sizeof(eSTMGRHealthInfo));

        /* Do the query */
        doDeviceHealthQuery();
        /* Updated it & return */
        memcpy (pHealthInfo, &m_healthInfo, sizeof(m_healthInfo));
    }
    else
    {
        STMGRLOG_ERROR("NULL Pointer input\n");
        rc = RDK_STMGR_RETURN_INVALID_INPUT;
    }

    return rc;
}

void rStorageNVRAM::readEraseCount()
{
    const char* pEraseCount = NULL;
    if (m_parentDevice)
    {
        pEraseCount = udev_device_get_sysattr_value(m_parentDevice, "max_ec");
        if (pEraseCount)
        {
            m_eraseCount = atoi(pEraseCount);
        }
    }
}

void rStorageNVRAM::readChipDetails()
{
    std::string findStr = "NAND device:";
    ifstream tmpFile;

    tmpFile.open(RDK_STMGR_NVRAM_TMP_FILE);
    if(!tmpFile)
    {
        STMGRLOG_ERROR("The temp file is not present; Seems like first time boot..\n");
    }
    else
    {
        tmpFile.open(RDK_STMGR_NVRAM_LOG_FILE);
    }

    if(!tmpFile)
    {
        STMGRLOG_ERROR("The log file also not present; Seems like we will not be able to provide the info..\n");
    }
    else
    {
        //char line[1024] = {'\0'};
        //char *isFound = NULL;
        std::string line;
        std::size_t isFound = std::string::npos;

        while(!tmpFile.eof())
        {
            std::getline(tmpFile, line);
            isFound = line.find(findStr);
            if (isFound != std::string::npos)
            {
                isFound += findStr.length();
                break;
            }
        }
        tmpFile.close();

        /* Check whether it is found something.. */
        if (isFound == std::string::npos)
        {
            return;
        }
        else
        {
            /* TODO here */
            std::string newStr = line.substr (isFound);
            STMGRLOG_INFO ("The line that we found is @%s@\n", newStr.c_str());
        }

    }
#if 0
    //FIXME: Update the followings
    char m_manufacturer[RDK_STMGR_MAX_STRING_LENGTH];
    char m_model[RDK_STMGR_MAX_STRING_LENGTH];
#endif
}
