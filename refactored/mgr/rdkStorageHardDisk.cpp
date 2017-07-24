
#include "rdkStorageHardDisk.h"
#include "rdkStorageMgrLogger.h"
#include "smartmonUtiles.h"

using namespace std;



rStorageHDDrive::rStorageHDDrive(std::string devicePath)
{
    m_devicePath = devicePath;
};


rStorageHDDrive::~rStorageHDDrive()
{
    for (auto it = m_partitionInfo.begin(); it != m_partitionInfo.end(); ++it )
    {
        rStoragePartition* pObj = it->second;
        if(pObj) {
            delete pObj;
        }
    }
};


eSTMGRReturns rStorageHDDrive::populateDeviceDetails()
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    /* Initial properties by smartctl api */
    get_SmartMonAttribute_Info();

    //    m_devicePath = "/dev/sda";
    m_type = RDK_STMGR_DEVICE_TYPE_HDD;

    /* Here, updating the partition table for harddisk
     * Also does DVR and TSB check.*/

    populatePartitionDetails();

    if(false == m_isTSBEnabled)
    {
        m_isTSBSupported = false;
        m_maxTSBCapacity = 0;
        m_freeTSBSpaceLeft = 0;
        m_maxTSBCapacityinMinutes = 0;
        m_maxTSBLengthConfigured = 0;
    }
    /* This is to track whether this memory device is supporting it our not; ie one of the partition could be used for DVR.*/
    if(false == m_isDVREnabled) {
        m_maxDVRCapacity = 0;
        m_freeDVRSpaceLeft = 0;
        m_isDVRSupported = false;
    }
    /*Add Telemetry logging */
    telemetry_logging();

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return RDK_STMGR_RETURN_SUCCESS;
}

bool rStorageHDDrive::get_SmartMonAttribute_Info()
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    bool ret = true;
    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
//    string args = "-iH /dev/sda";
    string args = "-iH " + m_devicePath;

    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {

        std::string deviceId;
        if(smCmd->getDeviceId(deviceId)) {
            memset(m_deviceID, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_deviceID, deviceId.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }
        string manufacture;
        if( smCmd->getManufacture(manufacture))    {
            memset(m_manufacturer, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_manufacturer, manufacture.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string model;
        if(smCmd->getModel(model)) {
            memset(m_model, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_model, model.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string firmwareVersion;
        if(smCmd->getFirmwareVersion(firmwareVersion)) {
            memset(m_firmwareVersion, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_firmwareVersion, firmwareVersion.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        std::string serialNumber;
        if(smCmd->getSerialNumber(serialNumber)) {
            memset(m_serialNumber, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_serialNumber, serialNumber.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string ata_standard;
        if(smCmd->getAtaStandard(ata_standard)) {
            memset(m_ifATAstandard, '\0', RDK_STMGR_MAX_STRING_LENGTH);
            strncpy(m_ifATAstandard, ata_standard.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        m_capacity = smCmd->getCapacity();
        m_hasSMARTSupport = smCmd->isSmartSupport();

        m_healthInfo.m_isHealthy = smCmd->isOverallHealthOkay();

    }
    else {
        ret = false;
    }
    delete smCmd;
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return ret;
}

bool rStorageHDDrive::get_OverallHealth_State()
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    bool overall_health = false;
    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
//    string args = "-H /dev/sda";
    string args = "-H " + m_devicePath;

    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {
        overall_health = smCmd->isOverallHealthOkay();
    }
    delete smCmd;

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return overall_health;
}


bool rStorageHDDrive::get_DiagnosticAttribues(eSTMGRDiagAttributesList* m_diagnosticsList)
{
    bool overall_health = false;

    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
    string args = "-A " + m_devicePath;

    memset(m_diagnosticsList, 0, sizeof(eSTMGRDiagAttributesList));
    std::map <string, string> attrList;

    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {
        overall_health = smCmd->getDiagnosticAttributes(attrList);
    }
    delete smCmd;

    std::map<std::string, std::string>::iterator it = attrList.begin();
    int i = 0;
    while(it != attrList.end())
    {
//		cout << "Key : " << it->first << "Value : " << it->second << endl;
        strncpy(m_diagnosticsList->m_diagnostics[i].m_name, it->first.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        strncpy(m_diagnosticsList->m_diagnostics[i].m_value, it->second.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        it++;
        i++;
        if(i >= RDK_STMGR_MAX_DIAGNOSTIC_ATTRIBUTES)
            break;
    }

    m_diagnosticsList->m_numOfAttributes = i;

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return overall_health;
}


bool rStorageHDDrive::logTel_DiagnosticAttribues(std::string& teleAttrString )
{
    bool overall_health = false;

    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
    string args = "-A " + m_devicePath;

    std::map <string, string> tel_attr_map;

    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {
        overall_health = smCmd->logTelemetry_DiagnosticAttributes(tel_attr_map);
    }

    delete smCmd;

    if(tel_attr_map.empty())
        return false;

    for (auto it = tel_attr_map.begin(); it != tel_attr_map.end(); ++it ) {
        string index = (std::string)(it->first);
        string value = (std::string)(it->second);
        teleAttrString.append(index + ":" + value + ",");
        STMGRLOG_DEBUG("[ %s:%s]\n", index.c_str(), value.c_str());
    }

    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return overall_health;
}


eSTMGRReturns rStorageHDDrive::getHealth (eSTMGRHealthInfo* pHealthInfo)
{
    memset(pHealthInfo, 0, sizeof(eSTMGRHealthInfo));
    if(get_OverallHealth_State()) {
        pHealthInfo->m_isHealthy = true;
        pHealthInfo->m_isOperational = true;
    }
    else
    {
        pHealthInfo->m_isHealthy = false;
        pHealthInfo->m_isOperational = false;
    }

    strncpy(pHealthInfo->m_deviceID, m_deviceID, RDK_STMGR_MAX_STRING_LENGTH);
    pHealthInfo->m_deviceType = m_type;

    get_DiagnosticAttribues(&pHealthInfo->m_diagnosticsList);

    return RDK_STMGR_RETURN_SUCCESS;
}

eSTMGRReturns rStorageHDDrive::doDeviceHealthQuery() {
    if(get_OverallHealth_State()) {
        return RDK_STMGR_RETURN_SUCCESS;
    }
    else
        return RDK_STMGR_RETURN_GENERIC_FAILURE;

}

/*
eSTMGRReturns rStorageHDDrive::getPartitionInfo (char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo) {

    if(NULL == pPartitionId)
    {

    }
    return RDK_STMGR_RETURN_SUCCESS;
}
*/


bool rStorageHDDrive::populatePartitionDetails()
{
    struct mntent *fs = NULL;
//    const char *dev  = "/dev/sda";
    const char *dev = m_devicePath.c_str();
    const char *file = "/proc/mounts";
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    FILE *fp = setmntent(file, "r");

    if (fp == NULL) {
        printf("[%s:%d] %s: could not open: %s\n", __FUNCTION__, __LINE__, file, strerror(errno));
    }

    while ((fs = getmntent(fp)) != NULL)
    {
        if(0 == strncasecmp(fs->mnt_fsname, dev, strlen(dev)))
        {
            get_filesystem_statistics(fs, dev);
        }
    }
    endmntent(fp);
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    return true;
}


bool rStorageHDDrive::get_filesystem_statistics(const struct mntent *fs, const char* dev)
{
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if (fs->mnt_fsname[0] != '/')
        return false;

    struct statvfs vfs;

    if (statvfs(fs->mnt_dir, & vfs) != 0) {
        STMGRLOG_ERROR ("[%s:%d]Failed in statvfs(%s) due to \' %s\' (%d).\n", __FUNCTION__, __LINE__, fs->mnt_dir, strerror(errno), errno);
        return false;
    }

    STMGRLOG_DEBUG("[%s:%d] %s, mounted on %s: of type: %s option: %s\n",__FUNCTION__, __LINE__, fs->mnt_dir, fs->mnt_fsname, fs->mnt_type, fs->mnt_opts);


    std::string key = fs->mnt_fsname;
    rStoragePartition *pObj = NULL;
    std::map <std::string, rStoragePartition*> :: const_iterator it = m_partitionInfo.find (key);

    if(it == m_partitionInfo.end()) {
        pObj = new rStoragePartition();
        STMGRLOG_DEBUG("[%s:%d] Creating object instance for rStoragePartition class.\n",__FUNCTION__, __LINE__);
    }
    else {
        pObj = it->second;
        STMGRLOG_DEBUG("[%s:%d] Using object instance for rStoragePartition. key (%s) of m_partitionInfo map.\n",__FUNCTION__, __LINE__, (string(it->first)).c_str());
    }


    if(pObj)
    {

        if(0 == strcasecmp(fs->mnt_type, (const char *)"xfs"))
        {
            get_Xfs_fs_stat(pObj, (const char*) fs->mnt_dir);
        }
        else
        {
            unsigned long long totat_space = vfs.f_blocks * vfs.f_frsize;
            unsigned long long avail_space = vfs.f_bavail * vfs.f_frsize;
            pObj->m_capacity = totat_space;
            pObj->m_freeSpace = avail_space;
            pObj->m_isDVRSupported = false;
            pObj->m_isTSBSupported = false;

            if ((vfs.f_flag & ST_RDONLY) != 0)
                pObj->m_status = RDK_STMGR_DEVICE_STATUS_READ_ONLY;
            if(avail_space == 0)
                pObj->m_status = RDK_STMGR_DEVICE_STATUS_DISK_FULL;

            STMGRLOG_DEBUG("[%s:%d] %s Filesystem Statistics (mounted on [%s]): \n", __FUNCTION__, __LINE__, fs->mnt_type, fs->mnt_dir);
            STMGRLOG_DEBUG("\t**********************************************\n");
            STMGRLOG_DEBUG("\t Total Capacity [%lld], Available Capacity [%lld]\n", totat_space, avail_space);
            STMGRLOG_DEBUG("\t m_isDVRSupported [%d], m_isDVREnabled [%d]\n", pObj->m_isDVRSupported,pObj->m_isTSBSupported);
            STMGRLOG_DEBUG("\t m_status [%d]\n", pObj->m_status);
            STMGRLOG_DEBUG("\t**********************************************\n");
        }

        string parName = fs->mnt_fsname;
        strncpy(pObj->m_partitionId, fs->mnt_fsname, strlen(fs->mnt_fsname));
        strncpy(pObj->m_format, fs->mnt_type, strlen(fs->mnt_type));
        /* Adding to m_partitionInfo map */
        m_partitionInfo.insert({parName, pObj});
    }
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return true;
}

bool rStorageHDDrive::get_Xfs_fs_stat(rStoragePartition *partition, const char* mountpoint)
{
    xfs_fsop_counts_t xfsFree;
    struct xfs_fsop_geom fsInfo;
    int rc, rc1;
    int fd;
    unsigned long long totalCapacity= 0;
    unsigned long long availCapacity= 0;
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    fd = open( mountpoint, O_RDONLY);
    if ( fd >= 0 )
    {
        rc = xfsctl( mountpoint, fd, XFS_IOC_FSCOUNTS, &xfsFree );
        rc1 = xfsctl( mountpoint, fd, XFS_IOC_FSGEOMETRY, &fsInfo );
        if ( rc >= 0 )
        {
            availCapacity= (((long long)(xfsFree.freedata))*((long long)(fsInfo.blocksize)));
        } else {
            STMGRLOG_ERROR("Failed in xfsctl (XFS_IOC_FSCOUNT) %s (%d), couldn't calculate free capacity.\n", strerror(errno), errno);
        }

        if ( rc1 >= 0 )
        {

            totalCapacity = ((long long)fsInfo.blocksize*(long long)fsInfo.rtblocks);
            partition->m_capacity = totalCapacity;
            partition->m_freeSpace = availCapacity;

            partition->m_status =  RDK_STMGR_DEVICE_STATUS_OK;

            /* In HDD, tsb stores in XFS file, if xfs file system present, enable tsb */
            m_isTSBEnabled = true;
            m_isTSBSupported = true;
            partition->m_isTSBSupported = true;
            m_maxTSBCapacity = 40*1024*1024; /*!< reserved 40GB in dvr */
            m_freeTSBSpaceLeft = 0;			/*!< Now way to calculate since same partition used for dvr and tsb */
            m_maxTSBCapacityinMinutes = 60; 	/*!< Reserved 1hr in tsb, defined in rmfconfig.ini as 'dvr.info.tsb.maxDuration = 3600 seconds'*/
            m_maxTSBLengthConfigured = 60;

            if(availCapacity == 0) {
                partition->m_status = RDK_STMGR_DEVICE_STATUS_DISK_FULL;
                m_tsbStatus = RDK_STMGR_TSB_STATUS_DISK_FULL;
            }
            else
                m_tsbStatus = RDK_STMGR_TSB_STATUS_OK;

            /* In HDD, DVR stores in XFS file, if xfs file system present, enable DVR */
            m_isDVRSupported = true;
            m_isDVREnabled = true;
            partition->m_isDVRSupported = true;
            m_maxDVRCapacity = totalCapacity - m_maxTSBCapacity;
            m_freeDVRSpaceLeft = availCapacity - m_maxTSBCapacity;

            STMGRLOG_DEBUG("[%s:%d] XFS Filesystem Statistics (mounted on [%s]): \n", __FUNCTION__, __LINE__, mountpoint);
            STMGRLOG_DEBUG("\t**********************************************\n");
            STMGRLOG_DEBUG("\t Total Capacity [%lld], Available Capacity [%lld]\n", totalCapacity, availCapacity);
            STMGRLOG_DEBUG("\t m_isDVRSupported [%d], m_isDVREnabled [%d]\n", m_isDVRSupported,m_isDVREnabled);
            STMGRLOG_DEBUG("\t m_maxTSBCapacity [%ld], m_freeTSBSpaceLeft [%ld]\n", m_maxTSBCapacity,m_freeTSBSpaceLeft);
            STMGRLOG_DEBUG("\t m_maxTSBCapacityinMinutes [%ld], m_maxTSBLengthConfigured [%ld]\n",  m_maxTSBCapacityinMinutes,m_maxTSBLengthConfigured);
            STMGRLOG_DEBUG("\t m_maxDVRCapacity [%ld] m_freeDVRSpaceLeft [%ld]\n", m_maxDVRCapacity,m_freeDVRSpaceLeft);
            STMGRLOG_DEBUG("\t**********************************************\n");
        } else {
            STMGRLOG_ERROR("Failed in xfsctl (XFS_IOC_FSGEOMETRY) %s (%d)\n", strerror(errno), errno);
            m_isTSBEnabled = false;
            m_isTSBSupported = false;
            STMGRLOG_ERROR("Failed to read xfs patition, so Disabling TSB.\n");
        }
        close(fd);
    } else {
        STMGRLOG_ERROR ("Failed to open mount path (%s) due to [\' %s\' (%d)].\n", mountpoint, strerror(errno), errno);
        return false;
    }
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return true;
}

void rStorageHDDrive::telemetry_logging() {
    STMGRLOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    /* Adding Telemetry Logs:*/
    std::string tel_log = "TELEMETRY_HDD:";	/*!< Starts with TELEMETRY_HDD String*/
    tel_log.append(m_manufacturer);	/*!< Add model string*/
    tel_log.append(",");

    /*Get HDD Overall Health */
    std::string status = (m_healthInfo.m_isHealthy)?"PASSED":"FAILURE";
    tel_log.append(status);	/*!< Add Overall Health string*/

    /*Get HDD Attributes*/
    std::string tel_attr_str;
    logTel_DiagnosticAttribues(tel_attr_str);
    if(!tel_attr_str.empty()) {
        tel_log.append(",");
        tel_log.append(tel_attr_str);	/*!< Add Attributes with index and raw value string*/
    }
    STMGRLOG_WARN ("%s\n", tel_log.c_str());
    STMGRLOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}
