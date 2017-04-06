#include "rdkStorageSDCard.h"
#include "rdkStorageMgrLogger.h"


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

#ifdef ENABLE_DEEP_SLEEP
const char* SM_MOUNT_PATH = "/media/tsb";
const char* SM_DISK_CHECK = "/lib/rdk/disk_checkV2";
#else
const char* SM_MOUNT_PATH = "/tmp/data";
const char* SM_DISK_CHECK = "/lib/rdk/disk_check";
#endif

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
    STMGRLOG_ERROR ("[%s:%d Device Path : %s\n", __FILE__, __FUNCTION__, devicePath.c_str());
};

rStorageSDCard::~rStorageSDCard()
{
    doUmountSDC();
}

/* Populates all the device data base */
eSTMGRReturns rStorageSDCard::populateDeviceDetails()
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
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
                            m_capacity = (unsigned long) (((atol(pCapacity))*512)/1024);
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
                        short partionNum = atol(udev_device_get_sysattr_value(pDevice, "partition"));

                        /* Create Partition class n update it */
                        rStoragePartition *pPartitionPtr = new rStoragePartition;
                        if (pPartitionPtr && partionNum)
                        {
                            sprintf (pPartitionPtr->m_partitionId, "%s", pDevNode);
                            m_devMount = pDevNode;
                            const char *pCapacity = udev_device_get_sysattr_value(pDevice, "size");

                            if (pCapacity)
                            {
                                pPartitionPtr->m_capacityinKB = (unsigned long) ((atol(pCapacity))*512)/1024;
                            }
                            const char* ro = udev_device_get_sysattr_value(pDevice, "ro");
                            if(ro)
                            {
                                pPartitionPtr->m_status = (atoi(ro) == 0)?RDK_STMGR_DEVICE_STATUS_OK:RDK_STMGR_DEVICE_STATUS_READ_ONLY;

                            }

                            /* Set default */
                            pPartitionPtr->m_freeSpaceinKB = 0;

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
                }
            }
        }


        /* Check for TSB Enabled and CMD56 capable  */
        if(isSDTSBSupported())
        {
            if(doMountSDC())
            {
                STMGRLOG_INFO("[%s] SDcard Mount Successfully.\n", __FUNCTION__);
                get_SdcPropertiesStatvfs();
            }
            else {
                STMGRLOG_ERROR ("[%s]Failed to mount, so disabled tsb.\n", __FUNCTION__);
                m_tsbStatus = RDK_STMGR_TSB_STATUS_DISABLED;
            }
        }
        else
        {
            STMGRLOG_ERROR ("[%s]TSB NOT_QUALIFIED, so disabled tsb.\n", __FUNCTION__);
            m_status = RDK_STMGR_DEVICE_STATUS_NOT_QUALIFIED;
            m_tsbStatus = RDK_STMGR_TSB_STATUS_DISABLED;
        }
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
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

    if(isTSBSupported)
    {
        m_isTSBEnabled = true;
        m_isTSBSupported = true;
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return isTSBSupported;
}



/* Execute_Mount_Script */
bool rStorageSDCard::doMountSDC()
{
    int ret = 0, status = 0;
    char mountbuff[200] = {'\0'};

    pthread_mutex_lock(&m_mountLock);

    if(m_isMounted == true) {
        STMGRLOG_INFO("[%s:%d] Already Mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        return true;
    }

    /* Now mount : disk_check mount /dev/mmcpblkp01 /media/tsb*/
    sprintf(mountbuff, "%s %s %s %s %s", "sh", SM_DISK_CHECK, "mount", m_devMount.c_str(), SM_MOUNT_PATH);

    ret = system(mountbuff);
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
    sprintf(umountbuff, "%s %s %s %s %s", "sh", SM_DISK_CHECK, "umount", m_devMount.c_str(), SM_MOUNT_PATH);

    ret = system(umountbuff);
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

                    /*Actual values */
                    pObj->m_freeSpaceinKB = (unsigned long long)(vfs.f_bsize * vfs.f_bfree)/1024; /* In KB */
                    sprintf(pObj->m_format, fs->mnt_type);
                    sprintf(pObj->m_mountPath, fs->mnt_dir);

                    pObj->m_status = RDK_STMGR_DEVICE_STATUS_OK;
                    pObj->m_isTSBSupported = true;
                    pObj->m_isDVRSupported = false;

                    unsigned long long frameRate = DEFULT_DATARATE_PER_SEC*(60*1024)/8;

                    unsigned short actualTsbMaxMin = pObj->m_capacityinKB/frameRate;

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
                        m_maxTSBLengthConfigured = DEFULT_TSB_MIN_MINUTE;
                    }

                    if((pObj->m_capacityinKB - pObj->m_freeSpaceinKB)<= 0) {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_FAILED;
                    }
                    else
                    {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_OK;
                    }

                    STMGRLOG_DEBUG("[%s:%d] Partition Details: \n", __FUNCTION__, __LINE__);
                    STMGRLOG_DEBUG("===========================================\n");
                    STMGRLOG_DEBUG("[%s, mounted on %s: of type: %s option: %s\n",
                                   fs->mnt_dir, fs->mnt_fsname, fs->mnt_type, fs->mnt_opts);
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
            endmntent(fp);
        }
    }

    return ret;
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}
