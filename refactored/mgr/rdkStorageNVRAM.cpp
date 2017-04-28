#include "rdkStorageNVRAM.h"
#include "rdkStorageMgrLogger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>


#define RDK_STMGR_NVRAM_EC_MAX      100000     /*<! SLC NAND theoretically are good for 100,000 cycles.*/
#define RDK_STMGR_NVRAM_EC_LIMIT     99000     /*<! Limiting 1000 cycles less to notify that its about to die */
#define RDK_STMGR_NVRAM_TMP_FILE    "/tmp/rStorageNVRAM.file"
#define RDK_STMGR_NVRAM_LOG_FILE    "/opt/logs/startup_stdout_log.txt"

const std::string RDK_STMGR_MANF_ID = "Manufacturer ID";
const std::string RDK_STMGR_CHIP_ID = "Chip ID";

rStorageNVRAM::rStorageNVRAM(std::string devicePath)
{
    m_pUDevNVRAM = udev_new();

    m_devicePath = devicePath;
    m_type = RDK_STMGR_DEVICE_TYPE_NVRAM;
    m_parentDevice = NULL;
    m_eraseCount = RDK_STMGR_NVRAM_EC_MAX;

    int l = strlen ("/dev/ubi");
    sprintf (m_deviceID, "%c", m_devicePath[l]);
    STMGRLOG_ERROR ("The m_deviceID is %s\n", m_deviceID);
}

/* Populates all the device data base */
eSTMGRReturns rStorageNVRAM::populateDeviceDetails()
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;

    m_status = RDK_STMGR_DEVICE_STATUS_OK;
    m_hasSMARTSupport = false;
    memset (&m_ifATAstandard, 0 , sizeof(m_ifATAstandard));

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
                        std::string deviceName = udev_device_get_sysname(pDevice);
                        const char *pDevicePartitionID = udev_device_get_devnode(pDevice);

                        STMGRLOG_INFO ("The Partition name is, %s\n", deviceName.c_str());
                        if (!pDevicePartitionID)
                        {
                            STMGRLOG_INFO ("Lets makeup a device path..\n");
                            deviceName = "/dev/" + deviceName;
                            STMGRLOG_INFO ("The Partition name is, %s\n", deviceName.c_str());
                            pDevicePartitionID = deviceName.c_str();
                        }

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


        /* Get the m_capacity */
        if (m_parentDevice)
        {
            struct udev_device *pMTDDevice = NULL;
            std::string mtdNumber = udev_device_get_sysattr_value(m_parentDevice, "mtd_num");
            STMGRLOG_INFO ("The mtdnumber is, %s\n", mtdNumber.c_str());
            std::string mtdDev = "mtd";
            mtdDev += mtdNumber;
            pMTDDevice = udev_device_new_from_subsystem_sysname(m_pUDevNVRAM, "mtd", mtdDev.c_str());
            if (pMTDDevice)
            {
                const char *pCapacity = udev_device_get_sysattr_value(pMTDDevice, "size");
                if (pCapacity)
                {
                    m_capacity = (unsigned long) (atol(pCapacity)/1024);
                }
                STMGRLOG_INFO ("The Capacity of this NVRAM device is, %lu\n", m_capacity);
            }
            else
            {
                STMGRLOG_ERROR ("Could not get the MTD Device info.. \n");
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
        tmpFile.open(RDK_STMGR_NVRAM_LOG_FILE);
    }

    if(!tmpFile)
    {
        STMGRLOG_ERROR("The log file also not present; Seems like we will not be able to provide the info..\n");

        memset (&m_firmwareVersion, 0, sizeof (m_firmwareVersion));
        memset (&m_manufacturer, 0, sizeof (m_firmwareVersion));
        memset (&m_model, 0, sizeof (m_firmwareVersion));
        memset (&m_serialNumber, 0, sizeof (m_firmwareVersion));
    }
    else
    {
        std::string line;
        std::size_t isFound = std::string::npos;
        std::string newStr;

        while(!tmpFile.eof())
        {
            std::getline(tmpFile, line);
            isFound = line.find(findStr);
            if (isFound != std::string::npos)
            {
                break;
            }
        }
        tmpFile.close();

        /* Check whether it is found something.. */
        if (isFound == std::string::npos)
        {
            STMGRLOG_ERROR ("The NAND Device is not found in the file that we monitor\n");
            return;
        }
        else
        {
            isFound += findStr.length();

            /* Write the retrived data into tmp file. */
            newStr = line.substr (isFound);
            STMGRLOG_INFO ("The line that we found is @%s@\n", newStr.c_str());
            ofstream writeFile;
            writeFile.open (RDK_STMGR_NVRAM_TMP_FILE);
            writeFile << newStr;
            writeFile.close();
        }

        parseManufData(newStr);
    }
    return;
}

std::string & ltrim_string (std::string & s)
{
    s.erase (s.begin (), std::find_if (s.begin (), s.end (), std::not1 (std::ptr_fun < int, int >(std::isspace))));
    return s;
}

std::string & rtrim_string (std::string & s)
{
    s.erase (std::find_if (s.rbegin (), s.rend (), std::not1 (std::ptr_fun < int, int >(std::isspace))).base (), s.end ());
    return s;
}

std::string & trim_string (std::string & s)
{
    return ltrim_string (rtrim_string (s));
}

std::vector < std::string > &split_string (const std::string & s, char delim, std::vector < std::string > &elems)
{
    std::stringstream ss (s);
    std::string item;
    while (std::getline (ss, item, delim))
    {
        elems.push_back (item);
    }
    return elems;
}

void rStorageNVRAM::parseManufData(std::string &parseStr)
{
    std::string nandHeader = parseStr;
    std::string manufactureStr;
    std::string chipIdStr;
    size_t position = 0;

    STMGRLOG_DEBUG ("The received string for parsing is %s\n", nandHeader.c_str());

    size_t manufactureIndex = nandHeader.find(RDK_STMGR_CHIP_ID);
    if (manufactureIndex != std::string::npos)
    {
        manufactureStr = nandHeader.substr(0, manufactureIndex);
        chipIdStr = nandHeader.substr(manufactureIndex);

        /* Parse Manufacturer ID */
        position = manufactureStr.find_first_of (':');
        if (position != std::string::npos)
        {
            std::string serialStr = manufactureStr.substr((position + 1));
            position = serialStr.find_first_of (',');
            if (position != std::string::npos)
                serialStr[position] = ' ';
            serialStr = trim_string (serialStr);
            strcpy (m_serialNumber, serialStr.c_str());
            STMGRLOG_INFO ("The serial number that we found is %s\n", serialStr.c_str());
        }

        /* Parse the Chip ID */
        position = chipIdStr.find_first_of (':');
        if (position != std::string::npos)
        {
            std::string tmpStr = chipIdStr.substr((position + 1));
            tmpStr = trim_string (tmpStr);

            std::vector<std::string> splitStr;
            split_string (tmpStr, ' ', splitStr);
            if (splitStr.size() == 3)
            {
                tmpStr = splitStr.at(0);
                strcpy (m_firmwareVersion, tmpStr.c_str());
                STMGRLOG_INFO ("The firmware version that we found is %s\n", tmpStr.c_str());

                std::string manufStr = splitStr.at(1);
                if (manufStr[0] == '(')
                {
                    manufStr[0] = ' ';
                }
                manufStr = trim_string(manufStr);
                STMGRLOG_INFO ("The Manufacturer name that we found is %s\n", manufStr.c_str());
                strcpy (m_manufacturer, manufStr.c_str());

                std::string modelStr = splitStr.at(2);
                size_t len = modelStr.length() - 1;
                if (modelStr[len] == ')')
                {
                    modelStr[len] = ' ';
                }
                modelStr = trim_string(modelStr);
                STMGRLOG_INFO ("The Model that we found is %s\n", modelStr.c_str());
                strcpy(m_model, modelStr.c_str());
            }
            else
            {
                STMGRLOG_ERROR ("The parsed string does nt hv manufacture name n model\n");
                strcpy (m_firmwareVersion, tmpStr.c_str());
                STMGRLOG_INFO ("The firmware version that we found is %s\n", tmpStr.c_str());
            }
        }
    }
    else
    {
        STMGRLOG_ERROR ("The received string does not have manufacture ID (%s)\n", nandHeader.c_str());
    }

    return;
}

/* End of File */
