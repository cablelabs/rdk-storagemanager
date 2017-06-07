#include "rdkStorageeMMC.h"
#include "rdkStorageMgrLogger.h"

#define SUBSYSTEM_FILTER_MMC "mmc"

#define EMMC_TSB_DEV_NODE "/dev/mmcblk0"
#define EMMC_TSB_DEV_PARTITION "/dev/mmcblk0p1"

const short FRAME_RATE_MBPS = 18;
const short DEFAULT_TSB_MAX_MINUTE=25;

const char* SM_EMMC_MOUNT_PATH = "/tmp/data";
const char* SM_EMMC_DISK_CHECK = "/lib/rdk/disk_check";

rStorageeMMC::rStorageeMMC(std::string devicePath)
{
    m_type = RDK_STMGR_DEVICE_TYPE_EMMCCARD;

    m_hasSMARTSupport = false;
    memset(m_deviceID, 0, sizeof(m_deviceID));

    memset (&m_manufacturer, 0 , sizeof(m_manufacturer));
    memset (&m_model, 0 , sizeof(m_model));
    memset (&m_serialNumber, 0 , sizeof(m_serialNumber));
    memset (&m_firmwareVersion, 0 , sizeof(m_firmwareVersion));
    memset (&m_ifATAstandard, 0 , sizeof(m_ifATAstandard));

    m_pUDeveMMC = udev_new();
    m_isMounted = false;
    m_tsbHM = NO_EMMC_TSB_SUPPORT;
    m_isDVREnabled = false;
    m_isDVRSupported = false;

    m_devicePath = devicePath;
    STMGRLOG_TRACE("[%s] Device Path : %s\n", __FUNCTION__, devicePath.c_str());
};

rStorageeMMC::~rStorageeMMC()
{
    doUmounteMMC();
}

/* Populates all the device data base */
eSTMGRReturns rStorageeMMC::populateDeviceDetails()
{
    eSTMGRReturns rc = RDK_STMGR_RETURN_SUCCESS;
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    if (!m_pUDeveMMC)
    {
        STMGRLOG_ERROR ("eMMC Storage Class: Could not able to create udev instance of its own.\n");
        rc = RDK_STMGR_RETURN_INIT_FAILURE;
    }
    else
    {
        struct udev_device *pDevice = NULL;
        struct udev_enumerate *pEnumerate = NULL;
        struct udev_list_entry *pDeviceList = NULL;
        struct udev_list_entry *pDeviceListEntry = NULL;

        pEnumerate = udev_enumerate_new(m_pUDeveMMC);
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
            const char* pSysAttrType = NULL;
            const char* pDevNode = NULL;
            const char* pDevType = NULL;

            pSysPath = udev_list_entry_get_name(pDeviceListEntry);
            pDevice = udev_device_new_from_syspath(m_pUDeveMMC, pSysPath);

            STMGRLOG_TRACE("[%s:%d] SysPath : %s  \n", __FUNCTION__, __LINE__, pSysPath);

            if (pDevice)
            {
                pSysAttrType = udev_device_get_sysattr_value(pDevice, "type");
                if (pSysAttrType && (strcasecmp (pSysAttrType, "MMC") == 0))
                {
                    STMGRLOG_DEBUG("[%s:%d]Read eMMC Attributes. \n", __FUNCTION__, __LINE__);
                    readeMMCSysAttributes(pDevice);
                }

                pDevNode = udev_device_get_devnode(pDevice);
                pDevType = udev_device_get_devtype(pDevice);

                if (pDevNode && (strncasecmp (pDevNode, m_devicePath.c_str(), m_devicePath.length()) == 0))
                {
                    STMGRLOG_TRACE("[%s:%d] Dev Node: %s, Dev Type: %s  \n", __FUNCTION__, __LINE__, pDevNode, pDevType);
                    if((0 == strcasecmp(pDevType, "disk") && (0 == strcasecmp (pDevNode, m_devicePath.c_str())))) {
                        const char *pCapacity = udev_device_get_sysattr_value(pDevice, "size");
                        if(pCapacity) {
                            m_capacity = (unsigned long) (((atol(pCapacity))*512)/1024);
                            STMGRLOG_INFO("[%s] m_capacity is : [%d]\n", __FUNCTION__, m_capacity);
                        }
                        const char* ro = udev_device_get_sysattr_value(pDevice, "ro");
                        STMGRLOG_INFO("[%s] The eMMC Card \'READ ONLY\' attribute is %s.\n", __FUNCTION__, ro);
                        if(ro)
                        {
                            int irw = atoi(ro);
                            m_status = (irw == 0)?RDK_STMGR_DEVICE_STATUS_OK:RDK_STMGR_DEVICE_STATUS_READ_ONLY;
                            STMGRLOG_INFO("[%s] The eMMC Card is %s and status is %d.\n", __FUNCTION__, ((irw)?"READ-ONLY":"READ-WRITE"),m_status);
                        }

                    }
                    /*Check for partition */
                    else if((0 == strcasecmp(pDevType, "partition")) && (0 == strcasecmp (pDevNode, EMMC_TSB_DEV_PARTITION))) {
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
                                pPartitionPtr->m_capacityinKB = (unsigned long) (((atol(pCapacity))*512)/1024);
                                STMGRLOG_INFO("[%s] Partition capacity is : [%lu]\n", __FUNCTION__, pPartitionPtr->m_capacityinKB);
                            }
                            const char* ro = udev_device_get_sysattr_value(pDevice, "ro");
                            if(ro)
                            {
                                pPartitionPtr->m_status = (atoi(ro) == 0)?RDK_STMGR_DEVICE_STATUS_OK:RDK_STMGR_DEVICE_STATUS_READ_ONLY;
                                sprintf(pPartitionPtr->m_mountPath, "%s", SM_EMMC_MOUNT_PATH);
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
        if(iseMMCTSBSupported())
        {
            if(doMounteMMC())
            {
                STMGRLOG_INFO("[%s] SDcard Mount Successfully.\n", __FUNCTION__);
                get_eMMCPropertiesStatvfs();
            }
            else {
                STMGRLOG_ERROR ("[%s]Failed to mount, so disabled tsb.\n", __FUNCTION__);
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
            sprintf(events.m_description, "%s", "Incompatible eMMC Card.");
            notifyEvent(events);
        }
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return rc;

}

/* Queries the Device for the health info */
eSTMGRReturns rStorageeMMC::doDeviceHealthQuery(void)
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return RDK_STMGR_RETURN_SUCCESS;
}


eSTMGRReturns rStorageeMMC::getHealth(eSTMGRHealthInfo* pHealthInfo)
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return RDK_STMGR_RETURN_SUCCESS;
}

bool rStorageeMMC::check_HealthStatus(eSTMGReMMCHSInfo *emmcHS)
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return true;
}


bool rStorageeMMC::iseMMCTSBSupported()
{
    bool isTSBSupported = false;

    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(0 == strcasecmp (EMMC_TSB_DEV_NODE, m_devicePath.c_str()))
    {
        isTSBSupported = true;
    }
    if(isTSBSupported)
    {
        m_isTSBEnabled = true;
        m_isTSBSupported = true;
        m_tsbHM = EMMC_TSB_SUPPORT_WITHOUT_HEALTH_MONITORING;
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return isTSBSupported;
}



/* Execute_Mount_Script */
bool rStorageeMMC::doMounteMMC()
{
    int ret = 0, status = 0;
    char mountbuff[200] = {'\0'};

    pthread_mutex_lock(&m_mountLock);

    if(m_isMounted == true) {
        STMGRLOG_INFO("[%s:%d] Already Mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&m_mountLock);
        return true;
    }

    /* Now mount : disk_check mount /dev/mmcpblkp01 /tmp/data*/
    sprintf(mountbuff, "%s %s %s %s %s", "sh", SM_EMMC_DISK_CHECK, "mount", EMMC_TSB_DEV_PARTITION, SM_EMMC_MOUNT_PATH);

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
bool rStorageeMMC::doUmounteMMC()
{
    int ret = 0, status = 0;
    char umountbuff[200] = {'\0'};

    if(m_isMounted == false) {
        STMGRLOG_INFO("[%s:%d] Already Umounted.\n", __FUNCTION__, __LINE__);
        return false; // TBD: return true?
    }

    pthread_mutex_lock(&m_mountLock);

    /* Now mount : disk_check umount /tmp/data*/
    sprintf(umountbuff, "%s %s %s %s %s", "sh", SM_EMMC_DISK_CHECK, "umount", EMMC_TSB_DEV_PARTITION, SM_EMMC_MOUNT_PATH);

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

bool rStorageeMMC::readeMMCSysAttributes(struct udev_device *device)
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

bool rStorageeMMC::get_eMMCPropertiesStatvfs()
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

                    unsigned long capacity  = vfs.f_blocks * (vfs.f_frsize/1024);
                    unsigned long freeSpace = vfs.f_bavail * (vfs.f_frsize/1024);
                    STMGRLOG_INFO ("Update the capacity n freespace as per STATVFS\n");

                    pObj->m_capacityinKB = capacity; /* in KB */
                    /*Actual values */
                    pObj->m_freeSpaceinKB = freeSpace; /* in KB */
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

                        /* Set the TSB Length to m_maxTSBCapacityinMinutes && Let the Service Manager Decide how much TSB is wanted */
                        m_maxTSBLengthConfigured = m_maxTSBCapacityinMinutes;
                    }

                    if((pObj->m_capacityinKB - pObj->m_freeSpaceinKB)<= 0) {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_DISK_FULL;
                    }
                    else
                    {
                        m_tsbStatus = RDK_STMGR_TSB_STATUS_OK;
                    }

                    STMGRLOG_INFO("[%s:%d] Partition Details: \n", __FUNCTION__, __LINE__);
                    STMGRLOG_INFO("===========================================\n");
                    STMGRLOG_INFO("[%s, mounted on %s: of type: %s option: %s\n",
                                   fs->mnt_dir, fs->mnt_fsname, fs->mnt_type, fs->mnt_opts);
                    STMGRLOG_INFO("\tm_partitionId: %s\n",  pObj->m_partitionId);
                    STMGRLOG_INFO("\tm_format: %s\n",  pObj->m_format);
                    STMGRLOG_INFO("\tm_mountPath: %s\n",  pObj->m_mountPath);
                    STMGRLOG_INFO("\tm_capacityinKB: %d\n",  pObj->m_capacityinKB);
                    STMGRLOG_INFO("\tm_freeSpaceinKB: %d\n",  pObj->m_freeSpaceinKB);
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
            endmntent(fp);
        }
    }

    return ret;
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}
