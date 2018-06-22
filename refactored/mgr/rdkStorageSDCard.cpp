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
#include "rdkStorageSDCard.h"
#include "rdkStorageMgrLogger.h"
#ifdef YOCTO_BUILD
extern "C" {
#include "secure_wrapper.h"
}
#endif

#define MMC_BLOCK_MAJOR  179
#define MMC_RSP_SPI_S1  (1 << 7)
#define MMC_RSP_SPI_R1  (MMC_RSP_SPI_S1)
#define MMC_CMD_ADTC  (1 << 5)
#define MMC_RSP_PRESENT  (1 << 0)
#define MMC_RSP_CRC  (1 << 2)
#define MMC_RSP_OPCODE  (1 << 4)
#define MMC_RSP_R1  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define SUBSYSTEM_FILTER_MMC "mmc"


const short FRAME_RATE_MBPS = 18;
const short DEFAULT_TSB_MAX_MINUTE=25;

const char* SM_MOUNT_PATH_ENV  = "SD_CARD_MOUNT_PATH";
const char* SM_TSB_PART_ENV    = "SD_CARD_TSB_PART";
const char* SM_DISK_VALID_FILE = "mountStatus.txt";
const char* SM_DISK_CHECK      = "/lib/rdk/disk_checkV2";

rStorageSDCard::rStorageSDCard(std::string devicePath)
{
    m_type = RDK_STMGR_DEVICE_TYPE_SDCARD;

    m_hasSMARTSupport = false;
    memset(m_deviceID, 0, sizeof(m_deviceID));

    memset (&m_manufacturer, 0 , sizeof(m_manufacturer));
    memset (&m_model, 0 , sizeof(m_model));
    memset (&m_serialNumber, 0 , sizeof(m_serialNumber));
    memset (&m_firmwareVersion, 0 , sizeof(m_firmwareVersion));
    memset (&m_ifATAstandard, 0 , sizeof(m_ifATAstandard));
    m_hasSMARTSupport = false;

    m_pUDevSDC = udev_new();
    m_isCMD56Enable = false;
    m_isMounted = false;
    m_tsbHM = NO_TSB_SUPPORT;
    m_isDVREnabled = false;
    m_isDVRSupported = false;

    m_devicePath = devicePath;
    STMGRLOG_ERROR ("[%s:%d] Device Path : %s\n", __FUNCTION__, __LINE__, devicePath.c_str());

    // get tsb mount path from environment
    char *pPath;
    pPath = getenv(SM_MOUNT_PATH_ENV);
    if (pPath!=NULL) {
        m_SMMountPath = pPath;
    }

    // get tsb predesignated partition
    pPath = getenv(SM_TSB_PART_ENV);
    if (pPath!=NULL) {
        m_devMount = pPath;
    }
    if (m_SMMountPath.empty()) {
        STMGRLOG_ERROR ("[%s:%d] TSB mount Path from device.properties is empty\n", __FUNCTION__, __LINE__);
    }
    STMGRLOG_INFO ("[%s:%d] TSB mount Path from device.properties is : %s\n", __FUNCTION__, __LINE__, m_SMMountPath.c_str());

    if (m_devMount.empty()) {
        STMGRLOG_ERROR ("[%s:%d] TSB device partition from device.properties is empty\n", __FUNCTION__, __LINE__);
    }
    STMGRLOG_INFO ("[%s:%d] TSB device partition from device.properties is : %s\n", __FUNCTION__, __LINE__, m_devMount.c_str());

    m_SMDiskValid = m_SMMountPath + "/" + SM_DISK_VALID_FILE;
    STMGRLOG_INFO ("[%s:%d] calculated full handshake file path is : %s\n", __FUNCTION__, __LINE__, m_SMDiskValid.c_str());
}

rStorageSDCard::~rStorageSDCard()
{
    doUmountSDC();
}

/* Populates all the device data base */
eSTMGRReturns rStorageSDCard::populateDeviceDetails()
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    STMGRLOG_INFO("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    if (!m_pUDevSDC)
    {
        STMGRLOG_ERROR ("NVRAM Storage Class: Could not able to create udev instance of its own.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    else
    {
//        m_status = RDK_STMGR_DEVICE_STATUS_OK;
        struct udev_device *pDevice;
        struct udev_enumerate *pEnumerate = NULL;
        struct udev_list_entry *pDeviceList = NULL;
        struct udev_list_entry *pDeviceListEntry = NULL;
        short partionNum  = 0;

        pEnumerate = udev_enumerate_new(m_pUDevSDC);
        udev_enumerate_add_match_subsystem(pEnumerate, SUBSYSTEM_FILTER_MMC);
        udev_enumerate_add_match_subsystem(pEnumerate, "block");

        if (0 != udev_enumerate_scan_devices(pEnumerate))
        {
            STMGRLOG_ERROR("[%s]Failed to scan the devices.\n", __FUNCTION__);
        }

        pDeviceList = udev_enumerate_get_list_entry(pEnumerate);

        udev_list_entry_foreach(pDeviceListEntry, pDeviceList)
        {
            const char* pSysPath = NULL;
            const char* pSysyAttrType = NULL;
            const char* pDevNode = NULL;
            const char* pDevType = NULL;

            pSysPath = udev_list_entry_get_name(pDeviceListEntry);
            pDevice = udev_device_new_from_syspath(m_pUDevSDC, pSysPath);


            STMGRLOG_DEBUG("[%s:%d] SysPath : %s  \n", __FUNCTION__, __LINE__, pSysPath);

            if (pDevice)
            {
                pSysyAttrType = udev_device_get_sysattr_value(pDevice, "type");
                if (pSysyAttrType && (strcasecmp (pSysyAttrType, "SD") == 0))
                {
                    STMGRLOG_DEBUG("[%s:%d]Read SDC Attributes. \n", __FUNCTION__, __LINE__);
                    readSdcSysAttributes(pDevice);
                }

                pDevNode = udev_device_get_devnode(pDevice);
                pDevType = udev_device_get_devtype(pDevice);

                if (pDevNode && (strncasecmp (pDevNode, m_devicePath.c_str(), m_devicePath.length()) == 0))
                {
                    if((0 == strcasecmp(pDevType, "disk") && (0 == strcasecmp (pDevNode, m_devicePath.c_str())))) {
                        const char *pCapacity = udev_device_get_sysattr_value(pDevice, "size");
                        if(pCapacity) {
                            m_capacity = atoll(pCapacity) * 512;
                        }
                        const char* ro = udev_device_get_sysattr_value(pDevice, "ro");
                        STMGRLOG_INFO("[%s] The SD Card \'READ ONLY\' attribute is %s.\n", __FUNCTION__, ro);
                        if(ro)
                        {
                            int irw = atoi(ro);
                            m_status = (irw == 0)?RDK_STMGR_DEVICE_STATUS_OK:RDK_STMGR_DEVICE_STATUS_READ_ONLY;
                            STMGRLOG_INFO("[%s] The SD Card is %s and status is %d.\n", __FUNCTION__, ((irw)?"READ-ONLY":"READ-WRITE"),m_status);
                        }

                    }
                    /*Check for partition */
                    else if(0 == strcasecmp(pDevType, "partition")) {
                        partionNum = atol(udev_device_get_sysattr_value(pDevice, "partition"));

                        /* Create Partition class n update it */
                        rStoragePartition *pPartitionPtr = new rStoragePartition;
                        if (pPartitionPtr && partionNum)
                        {
                            sprintf (pPartitionPtr->m_partitionId, "%s", pDevNode);
                            // if not alrady provided by environment variable try to use the first partiton as tsb
                            if (m_devMount.empty() && (partionNum == 1))
                            {
                                m_devMount = pDevNode;
                            }
                            const char *pCapacity = udev_device_get_sysattr_value(pDevice, "size");

                            if (pCapacity)
                            {
                                pPartitionPtr->m_capacity = atoll(pCapacity) * 512;
                                STMGRLOG_INFO("[%s] Partition capacity is : [%llu]\n", __FUNCTION__, pPartitionPtr->m_capacity);
                            }
                            const char* ro = udev_device_get_sysattr_value(pDevice, "ro");
                            if(ro)
                            {
                                pPartitionPtr->m_status = (atoi(ro) == 0)?RDK_STMGR_DEVICE_STATUS_OK:RDK_STMGR_DEVICE_STATUS_READ_ONLY;
                                sprintf(pPartitionPtr->m_mountPath, "%s", m_SMMountPath.c_str());

                            }

                            /* Set default */
                            pPartitionPtr->m_freeSpace = 0;

                            /* @TODO: Update m_isTSBSupported after successful mounting */
                            pPartitionPtr->m_isTSBSupported = false;
                            pPartitionPtr->m_isDVRSupported = false;
                            m_partitionInfo.insert({pDevNode, pPartitionPtr});
                        }
                        else
                        {
                            STMGRLOG_ERROR ("Could not find the device partition id..\n");
                            delete pPartitionPtr;
                        }
                    }
                    else {
                        STMGRLOG_ERROR ("This SD Card detected, but not partitioned.\n");
                    }
                }
            }
        }


        /* Check for TSB Enabled and CMD56 capable  */
        if(isSDTSBSupported())
        {
            if(partionNum && doMountSDC())
            {
                STMGRLOG_INFO("[%s] SDcard Mount Successfully.\n", __FUNCTION__);
                get_SdcPropertiesStatvfs();
                /* Since the card is mounted by script, check whether u can read/write */
                {
                    bool isOk = false;
                    int fileD = open(m_SMDiskValid.c_str(), O_RDONLY);
                    if (fileD > 0)
                    {
                        ssize_t length;
                        char buffer[1024] = "";
                        length = read(fileD, buffer, 1000);
                        if (length)
                        {
                            if (strstr(buffer, "SUCCESS"))
                            {
                                /* TODO: Check the timing information but now lets just test it now */
                                STMGRLOG_WARN("[%s] The HANDSHAKE file Content is Proper.. So Enable TSB\n", __FUNCTION__);
                                isOk = true;
                            }
                            else
                                STMGRLOG_ERROR ("[%s] The HANDSHAKE file Content is missing. Mount must have been hung. So disabled TSB\n", __FUNCTION__);
                        }
                        else
                        {
                            STMGRLOG_ERROR ("[%s] The HANDSHAKE file Content is missing. Mount must have been hung. So disabled TSB\n", __FUNCTION__);
                        }
                        close(fileD);
                    }
                    else
                    {
                        STMGRLOG_ERROR ("[%s] The HANDSHAKE file is missing. Mount must have been Failed. So disabled TSB\n", __FUNCTION__);
                    }

                    /* Set the TSB Status */
                    m_isTSBSupported = isOk;
                    if ((RDK_STMGR_TSB_STATUS_OK == m_tsbStatus) && (m_isTSBSupported))
                        m_isTSBEnabled = true;
                }
            }
            else {
                STMGRLOG_ERROR ("[%s]Failed to mount, so disabled tsb.\n", __FUNCTION__);
               if(!partionNum)
                   STMGRLOG_ERROR ("This SD Card detected, but not partitioned.\n");
                m_tsbStatus = RDK_STMGR_TSB_STATUS_READ_ONLY;
            }
        }
        else
        {
            STMGRLOG_ERROR ("[%s]TSB NOT_QUALIFIED, so disabled tsb.\n", __FUNCTION__);
            m_status = RDK_STMGR_DEVICE_STATUS_NOT_QUALIFIED;
            m_tsbStatus = RDK_STMGR_TSB_STATUS_NOT_QUALIFIED;

            /*Notify Event for Disqualified tsb card */
            eSTMGREventMessage events;
            memset (&events, 0, sizeof(events));

            events.m_eventType = RDK_STMGR_EVENT_TSB_ERROR;
            events.m_deviceType = m_type;
            events.m_deviceStatus = m_status;
            sprintf(events.m_description, "%s", "Incompatible SD Card.");
            notifyEvent(events);
        }
    }

    STMGRLOG_INFO("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return rc;

}

/* Queries the Device for the health info */
eSTMGRReturns rStorageSDCard::doDeviceHealthQuery(void)
{
    return RDK_STMGR_RETURN_SUCCESS;
}


eSTMGRReturns rStorageSDCard::getHealth(eSTMGRHealthInfo* pHealthInfo)
{
    if(m_isCMD56Enable)
    {
        eSTMGRSdHSInfo hs;

        STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
        if(check_HealthStatus(&hs))
        {
            strcpy(pHealthInfo->m_deviceID, m_deviceID);
            pHealthInfo->m_deviceType = RDK_STMGR_DEVICE_TYPE_SDCARD;

            if(hs.used) {
                pHealthInfo->m_isOperational = true;
                pHealthInfo->m_isHealthy = true;
                sprintf(pHealthInfo->m_diagnosticsList.m_diagnostics[0].m_name, "used");
                sprintf(pHealthInfo->m_diagnosticsList.m_diagnostics[0].m_value, "%d", hs.used);
                pHealthInfo->m_diagnosticsList.m_numOfAttributes = 1;
            }
            else
            {
                pHealthInfo->m_isOperational = false;
                pHealthInfo->m_isHealthy = false;
            }
        }

    }
    return RDK_STMGR_RETURN_SUCCESS;
}

bool rStorageSDCard::check_HealthStatus(eSTMGRSdHSInfo *sdcHS)
{
    bool status = false;
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    uint8_t health_status_buffer[512];
    int fd = -1;
    int ret = -1;

    fd = open(m_devicePath.c_str(), O_RDONLY);

    if (fd < 0) {
        STMGRLOG_ERROR("[%s:%d] Failed opening file \'%s\', returns with error no : %d, i.e., \'%s\'] \n",
                       __FUNCTION__, __LINE__, m_devicePath.c_str(), errno, strerror(errno));
        return false;
    }

    struct mmc_ioc_cmd idata;

    memset(&idata, 0, sizeof(idata));
    memset(health_status_buffer, 0, sizeof(__u8) * 512);

    idata.write_flag = 0;
    idata.opcode = 56;
    idata.arg = 0x110005FF;
    idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
    idata.blksz = 512;
    idata.blocks = 1;
    idata.data_timeout_ns = 1000000000;
    idata.cmd_timeout_ms = 100;

    mmc_ioc_cmd_set_data(idata, health_status_buffer);

    ret = ioctl(fd, MMC_IOC_CMD, &idata);

    STMGRLOG_INFO("[%s:%d] mmc_ioc_cmd_set_data() for opcode (CMD56) returns (%d).\n", __FUNCTION__, __LINE__, ret);

    if (ret != 0) {
        perror("ioctl");
        STMGRLOG_ERROR("[%s:%d] Failed: ioctl call returns: %d, error no : %d, i.e., \'%s\'] \n", __FUNCTION__, __LINE__, ret, errno, strerror(errno));

        memset(&idata, 0, sizeof(idata));
        idata.write_flag = 0;
        idata.opcode = 12;
        idata.arg = 0x00000000;
        idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
        ret = ioctl(fd, MMC_IOC_CMD, &idata);
        if (ret) {
            /* Set the CMD56 status, so don't check 'LifeElapsed' for none supported CMD56 cmd SDcard*/
            m_isCMD56Enable = false;
            STMGRLOG_ERROR("[%s:%d]ioctl stop transmission. isCMD56Enable(%s) \n", __FUNCTION__, __LINE__, (char*)(m_isCMD56Enable?"YES":"FALSE"));
        }
    }
    else {
        eSTMGRSdHSInfo *hs = NULL;
        hs = (eSTMGRSdHSInfo *) health_status_buffer;
        sdcHS->tsb_id = hs->tsb_id;
        if(hs->tsb_id == TSB_ID0 || (hs->tsb_id == TSB_ID1))
        {
            if(hs->man_date[0] != '\0')
                memcpy(sdcHS->man_date,hs->man_date,strlen(hs->man_date));
            sdcHS->used = hs->used;
            sdcHS->used_user_area = hs->used_user_area;
            sdcHS->used_spare_block = hs->used_spare_block;

            if(hs->customer_name[0] != '\0')
                memcpy(sdcHS->customer_name, hs->customer_name, strlen(hs->customer_name));
        }
        status = true;
        m_isCMD56Enable = true;
        STMGRLOG_DEBUG("id=%x decimal id=%d date=%.6s used=%d  used_user_area=%d used_spare_block=%d customer_name=%.32s\n",
                       hs->tsb_id, hs->tsb_id, hs->man_date, hs->used, hs->used_user_area, hs->used_spare_block, hs->customer_name);
    }
    close(fd);
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return status;
}


bool rStorageSDCard::isSDTSBSupported()
{
    uint16_t tsbValue = 0;
    bool isTSBSupported = false;
    eSTMGRSdHSInfo hs;

    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    /*Check for TSB*/
    if(check_HealthStatus(&hs))
    {
        tsbValue = hs.tsb_id;

        STMGRLOG_DEBUG("[%s:%d] tsbValue: %d \n", __FUNCTION__, __LINE__, tsbValue);

        if ((tsbValue == TSB_ID0 || (tsbValue == TSB_ID1)) )
        {
            /* TSB Supports with Health Monitoring*/
            m_tsbHM = TSB_SUPPORT_WITH_HEALTH_MONITORING;
            STMGRLOG_INFO("[%s:%d] This SDCard (%s) have \"TSB_SUPPORT_WITH_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__, m_deviceID);
            isTSBSupported = true;
        }
        else if(0 == strncasecmp(m_deviceID, EXCEPTION_CID, strlen(EXCEPTION_CID)))
        {
            m_tsbHM = TSB_SUPPORT_WITHOUT_HEALTH_MONITORING;
            STMGRLOG_INFO("[%s:%d] This SDCard (%s) have \"TSB_SUPPORT_WITHOUT_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__, m_deviceID);
            isTSBSupported = true;
        }
        else
        {
            m_tsbHM = NO_TSB_SUPPORT;
            STMGRLOG_WARN("[%s:%d]  This SDCard (%s) is Incompatible, so doesn't support tsb. \n", __FUNCTION__, __LINE__, m_deviceID);
        }
    }
    else {
        if(0 == strncasecmp(m_deviceID, EXCEPTION_CID, strlen(EXCEPTION_CID)))
        {
            m_tsbHM = TSB_SUPPORT_WITHOUT_HEALTH_MONITORING;
            STMGRLOG_INFO("[%s:%d] This SDCard (%s) have \"TSB_SUPPORT_WITHOUT_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__, m_deviceID);
            isTSBSupported = true;
        }
        else
        {
            STMGRLOG_WARN("[%s:%d]  This SDCard (%s) is Incompatible and doesn't support tsb. \n", __FUNCTION__, __LINE__, m_deviceID );
            m_tsbHM = NO_TSB_SUPPORT;
            isTSBSupported = false;
        }
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return isTSBSupported;
}



/* Execute_Mount_Script */
bool rStorageSDCard::doMountSDC()
{
    int ret = 0, status = 0 ;
    char mountbuff[200] = {'\0'};

    pthread_mutex_lock(&m_mountLock);

    if(m_isMounted == true) {
        STMGRLOG_INFO("[%s:%d] Already Mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        return true;
    }

    /* Now mount : disk_check mount /dev/mmcpblkp01 /media/tsb*/
    sprintf(mountbuff, "%s %s %s %s %s", "sh", SM_DISK_CHECK, "mount", m_devMount.c_str(), m_SMMountPath.c_str());

#ifdef YOCTO_BUILD
    ret = v_secure_system(mountbuff);
#else
    ret = system(mountbuff);
#endif
    status = WEXITSTATUS(ret);
     
    STMGRLOG_INFO("[%s:%d] Executed : \'%s\', return as [%d]\n", __FUNCTION__, __LINE__, mountbuff, ret);

    if(!status)
    {
        STMGRLOG_INFO("[%s:%d] Successfully mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        m_isMounted = true;
        return true;
    }
    else
    {
        STMGRLOG_ERROR("[%s:%d] Fail to mount.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        return false;
    }
   
  
    
}


/* Execute_Umount_Script */
bool rStorageSDCard::doUmountSDC()
{
    int ret = 0, status = 0;
    char umountbuff[200] = {'\0'};

    if(m_isMounted == false) {
        STMGRLOG_INFO("[%s:%d] Already Umounted.\n", __FUNCTION__, __LINE__);
        return false; // TBD: return true?
    }

    pthread_mutex_lock(&m_mountLock);

    /* Now mount : disk_check umount /media/tsb*/
    sprintf(umountbuff, "%s %s %s %s %s", "sh", SM_DISK_CHECK, "umount", m_devMount.c_str(), m_SMMountPath.c_str());
#ifdef YOCTO_BUILD
    ret = v_secure_system(umountbuff);
#else
    ret = system(umountbuff);
#endif
    status = WEXITSTATUS(ret);

    STMGRLOG_INFO("[%s:%d] Executed : \'%s\', return as [%d]\n", __FUNCTION__, __LINE__, umountbuff, ret);

    if(!status)
    {
        STMGRLOG_INFO("[%s:%d] Successfully umounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        m_isMounted = false;
        return true;
    }
    else
    {
        STMGRLOG_ERROR("[%s:%d] Fail to umount.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        return false;
    }
}

bool rStorageSDCard::readSdcSysAttributes(struct udev_device *device)
{
    bool ret = false;

    if(!device) {
        return ret;
    }
    else {
        ret = true;
    }

    STMGRLOG_DEBUG("[%s:%d] Device PATH = \'%s\' with subsystem (%s).\n",
                   __FUNCTION__, __LINE__, udev_device_get_devpath(device),
                   udev_device_get_subsystem(device));

    struct udev_list_entry *list_entry = NULL;
    const char *sysAttrName = NULL;
    const char *sysAttrVal = NULL;

    /* Iterate system attributes and update the SDCard system properties */
    udev_list_entry_foreach(list_entry, udev_device_get_sysattr_list_entry(device)) {
        sysAttrName = udev_list_entry_get_name(list_entry),
        sysAttrVal = udev_device_get_sysattr_value(device, sysAttrName);

        STMGRLOG_DEBUG("[%s()] [%s : %s]\n", __FUNCTION__, sysAttrName, sysAttrVal);

        if(0 == strcasecmp(sysAttrName, "cid")) {
            sprintf(m_deviceID, "%s", sysAttrVal);
        }
        if(0 == strcasecmp(sysAttrName, "scr")) {
            m_scr = sysAttrVal;
        }
        if(0 == strcasecmp(sysAttrName, "date")) {
            m_date = sysAttrVal;
        }
        if(0 == strcasecmp(sysAttrName, "name")) {
            sprintf(m_model,"%s", sysAttrVal);
        }
        if(0 == strcasecmp(sysAttrName, "fwrev")) {
            sprintf(m_firmwareVersion, "%s", sysAttrVal);
        }
        if(0 == strcasecmp(sysAttrName, "hwrev")) {
            /*LotID */
            sprintf(m_hwVersion, "%s", sysAttrVal);
        }
        if(0 == strcasecmp(sysAttrName, "oemid")) {
            m_oemid = sysAttrVal;
        }
        if(0 == strcasecmp(sysAttrName, "manfid")) {
            sprintf(m_manufacturer, "%s", sysAttrVal);
        }
        if(0 == strcasecmp(sysAttrName, "serial")) {
            sprintf(m_serialNumber,"%s", sysAttrVal);
        }
    }
    return ret;
}


bool rStorageSDCard::get_SdcPropertiesStatvfs()
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    bool ret = false;

    const char *file = "/etc/mtab";
    struct mntent *fs = NULL;
    FILE *fp = setmntent(file, "r");

    if (fp == NULL) {
        STMGRLOG_ERROR("[%s:%d] %s: could not open: %s\n", __FUNCTION__, __LINE__, file, strerror(errno));
        return ret;
    }


    for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
    {
        rStoragePartition* pObj = it->second;
        if(pObj)
        {

            while ((fs = getmntent(fp)) != NULL)
            {
                if (fs->mnt_fsname[0] != '/')	/* skip nonreal filesystems */
                    continue;

                if(0 == strncasecmp(fs->mnt_fsname, pObj->m_partitionId, strlen((const char *)pObj->m_partitionId)))
                {
                    struct statvfs vfs;
                    if (statvfs(fs->mnt_dir, & vfs) != 0) {
                        STMGRLOG_ERROR("[%s:%d] %s: statvfs failed: %s\n", __FUNCTION__, __LINE__, fs->mnt_dir, strerror(errno));
                        return ret;
                    }

                    unsigned long long capacity  = (long long) vfs.f_blocks * (long long) vfs.f_frsize;
                    unsigned long long freeSpace = (long long) vfs.f_bavail * (long long) vfs.f_frsize;
                    STMGRLOG_INFO ("Update the capacity n freespace as per STATVFS\n");

                    pObj->m_capacity = capacity;
                    /*Actual values */
                    pObj->m_freeSpace = freeSpace;
                    sprintf(pObj->m_format, fs->mnt_type);
                    sprintf(pObj->m_mountPath, fs->mnt_dir);

                    pObj->m_status = RDK_STMGR_DEVICE_STATUS_OK;
                    pObj->m_isTSBSupported = true;
                    /* Update the base class variable as well for TSB Support*/
                    m_isTSBSupported = true;

                    pObj->m_isDVRSupported = false;

                    unsigned long long frameRate = (DEFULT_DATARATE_PER_SEC * 60 * 1024 * 1024)/8;

                    unsigned short actualTsbMaxMin = pObj->m_capacity/frameRate;

                    if(actualTsbMaxMin)
                    {
                        /* Need to check whether tsbMaxMin is less than that supported by the platform */
                        if((actualTsbMaxMin >= DEFULT_TSB_MAX_MINUTE) || ((actualTsbMaxMin < DEFULT_TSB_MAX_MINUTE) && (actualTsbMaxMin >= DEFULT_TSB_MIN_MINUTE)))
                        {
                            m_maxTSBCapacityinMinutes = DEFULT_TSB_MIN_MINUTE;
                        }
                        else if(actualTsbMaxMin < DEFULT_TSB_MIN_MINUTE)
                        {
                            m_maxTSBCapacityinMinutes = actualTsbMaxMin;
                        }

                        /* Set the TSB Length to m_maxTSBCapacityinMinutes && Let the Service Manager Decide how much TSB is wanted */
                        m_maxTSBLengthConfigured = m_maxTSBCapacityinMinutes;
                    }

                    if((pObj->m_capacity - pObj->m_freeSpace)<= 0) {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_DISK_FULL;
                    }
                    else
                    {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_OK;
                    }

                    /* Update the BaseClass members */
                    m_maxTSBCapacity = pObj->m_capacity;
                    m_freeTSBSpaceLeft = pObj->m_freeSpace;

                    STMGRLOG_INFO("[%s:%d] Partition Details: \n", __FUNCTION__, __LINE__);
                    STMGRLOG_INFO("===========================================\n");
                    STMGRLOG_INFO("[%s, mounted on %s: of type: %s option: %s\n",
                                   fs->mnt_dir, fs->mnt_fsname, fs->mnt_type, fs->mnt_opts);
                    STMGRLOG_INFO("\tm_partitionId: %s\n",  pObj->m_partitionId);
                    STMGRLOG_INFO("\tm_format: %s\n",  pObj->m_format);
                    STMGRLOG_INFO("\tm_mountPath: %s\n",  pObj->m_mountPath);
                    STMGRLOG_INFO("\tm_capacity: %llu\n",  pObj->m_capacity);
                    STMGRLOG_INFO("\tm_freeSpace: %llu\n",  pObj->m_freeSpace);
                    STMGRLOG_INFO("\tm_maxTSBCapacityinMinutes: %d\n",  m_maxTSBCapacityinMinutes);
                    STMGRLOG_INFO("\tm_status: %d\n",  pObj->m_status);
                    STMGRLOG_INFO("\tm_isTSBSupported: %d\n",  pObj->m_isTSBSupported);
                    STMGRLOG_INFO("\tm_isDVRSupported: %d\n",  pObj->m_isDVRSupported);

                    STMGRLOG_DEBUG("\tf_bsize: %ld\n",  (long) vfs.f_bsize);
                    STMGRLOG_DEBUG("\tf_frsize: %ld\n", (long) vfs.f_frsize);
                    STMGRLOG_DEBUG("\tf_blocks: %lu\n", (unsigned long) vfs.f_blocks);
                    STMGRLOG_DEBUG("\tf_bfree: %lu\n",  (unsigned long) vfs.f_bfree);
                    STMGRLOG_DEBUG("\tf_bavail: %lu\n", (unsigned long) vfs.f_bavail);
                    STMGRLOG_DEBUG("\tf_files: %lu\n",  (unsigned long) vfs.f_files);
                    STMGRLOG_DEBUG("\tf_ffree: %lu\n",  (unsigned long) vfs.f_ffree);
                    STMGRLOG_DEBUG("\tf_favail: %lu\n", (unsigned long) vfs.f_favail);
                    STMGRLOG_DEBUG("\tf_fsid: %#lx\n",  (unsigned long) vfs.f_fsid);
                    STMGRLOG_DEBUG("\tf_flag: %lu\n", (unsigned long) vfs.f_flag);
                    STMGRLOG_DEBUG("===========================================\n");
                    ret = true;
                    break;
                }
            }

        }
    }

    endmntent(fp);
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return ret;
}
