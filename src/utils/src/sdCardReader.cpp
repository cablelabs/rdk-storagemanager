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


/**
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/


#include "sdCardReader.h"
#include "storageMgrInternal.h"

extern stmMgrConfigProp confProp;
extern strMgrDeviceInfoParam_t gdeviceInfo;
extern pthread_mutex_t hsLock;

char sysPath_SDCard[200] = {'\0'};
static uint8_t tsbStatusCode = 0;

static bool sSDCardPresent;						/**@<	SD Card present	*/
static bool sIsTSBSupported = false;
static bool sdCardReadyStatus = false;
static bool isCMD56Enable =false;
static bool isMounted = false;
static eTsbStatus tsb_HM_Status = NO_TSB_SUPPORT;

pthread_mutex_t mountLock = PTHREAD_MUTEX_INITIALIZER;

void createFile(char *fileName);

bool isSDCardPresent()
{
    return sSDCardPresent;
}

void setSDCardPresenceFlag(bool sdCardPresenceFlag)
{
    sSDCardPresent = sdCardPresenceFlag;
}


bool getSDCard_TSB_Supported()
{
    return sIsTSBSupported;
}

void setSDCard_TSB_Supported(bool tsbFlag)
{
    sIsTSBSupported = tsbFlag;
}

uint8_t getTSBStatusCode()
{
    return tsbStatusCode;
}

void setsdCardStatusOnReadyEvent(bool sdCardStatus)
{
    sdCardReadyStatus = sdCardStatus;
}

bool getsdCardStatusOnReadyEvent()
{
    return sdCardReadyStatus;
}

bool isCMD56Supported()
{
    return isCMD56Enable;
}

bool check_tsb_Supported()
{
    bool status = true;
    uint16_t tsbValue = 0;
    struct health_status hs;

    IARM_Bus_STRMgr_EventData_t eventData;

    if(false == isSDCardPresent())
    {
        sIsTSBSupported = false;
        return false;
    }

    if(false == confProp.tsbValidationFlag)
    {
        sIsTSBSupported = true;
        return status;
    }

    /*Check for TSB*/
    status = stMgr_Check_SDCard_HealthStatus(&hs);

    if(status)
    {
        tsbValue = hs.tsb_id;

        LOG_DEBUG("[%s:%d] tsbValue: %d \n", __FUNCTION__, __LINE__, tsbValue);
        if ((tsbValue == TSB_ID0 || (tsbValue == TSB_ID1)) )
        {
            /* TSB Supports with Health Monitoring*/
            tsb_HM_Status = TSB_SUPPORT_WITH_HEALTH_MONITORING;
            LOG_NOTICE("[%s:%d]  \"TSB_SUPPORT_WITH_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__);
            sIsTSBSupported = true;
            tsbStatusCode = hs.used;
        }
        else if(0 == strncasecmp(gdeviceInfo.devid, EXCEPTION_CID, strlen(EXCEPTION_CID)))
        {
            tsb_HM_Status = TSB_SUPPORT_WITHOUT_HEALTH_MONITORING;
            LOG_NOTICE("[%s:%d]  \"TSB_SUPPORT_WITHOUT_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__);
            sIsTSBSupported = true;
        }
        else
        {
            tsb_HM_Status = NO_TSB_SUPPORT;
            LOG_WARN("[%s:%d] Incompatible SD Card (CID: %s). It doen't support tsb. \n",
                   __FUNCTION__, __LINE__, gdeviceInfo.devid );

        }
    }
    else {
        if(0 == strncasecmp(gdeviceInfo.devid, EXCEPTION_CID, strlen(EXCEPTION_CID)))
        {
            tsb_HM_Status = TSB_SUPPORT_WITHOUT_HEALTH_MONITORING;
            LOG_NOTICE("[%s:%d]  \"TSB_SUPPORT_WITHOUT_HEALTH_MONITORING\" \n", __FUNCTION__, __LINE__);
            sIsTSBSupported = true;
        }
        else
        {
            LOG_WARN("[%s:%d] Incompatible SD Card (CID: %s), it doen't support tsb. \n",
                   __FUNCTION__, __LINE__, gdeviceInfo.devid );
            tsb_HM_Status = NO_TSB_SUPPORT;
            sIsTSBSupported = false;
        }
    }

    sIsTSBSupported = (tsb_HM_Status != NO_TSB_SUPPORT)?true:false;
    if(false == sIsTSBSupported)
    {
        memset(&eventData, 0, sizeof(eventData));
        memcpy(eventData.data.tsbErrStatus.deviceId, gdeviceInfo.devid, strlen(gdeviceInfo.devid));
        eventData.data.tsbErrStatus.status = SM_DEV_STATUS_NOT_QUALIFIED;
        gdeviceInfo.status = SM_DEV_STATUS_NOT_QUALIFIED;
        memcpy(eventData.data.tsbErrStatus.description, INCOMPATIBLE_SDCARD , (strlen(INCOMPATIBLE_SDCARD)+1));
        IARM_Bus_BroadcastEvent(IARM_BUS_ST_MGR_NAME, (IARM_EventId_t) IARM_BUS_STORAGE_MGR_EVENT_TSB_ERROR, (void *)&eventData, sizeof(eventData));

        LOG_INFO("[%s:%d] \"%s\" sends \"TSB_ERROR\" IARM_Bus_BroadcastEvent with [\"%s:%d:%s\"]\n",
               __FUNCTION__, __LINE__, IARM_BUS_ST_MGR_NAME,
               eventData.data.tsbErrStatus.deviceId,
               eventData.data.tsbErrStatus.status,
               eventData.data.tsbErrStatus.description);
        status = false;
    }
    else {
        status = true;
    }

    return status;
}

bool stMgr_Check_SDCard_HealthStatus(struct health_status *hsbuf)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    uint8_t health_status_buffer[512];
    int fd = -1;
    int ret = -1;
    bool retVal = false;
    struct health_status *hs = NULL;

    pthread_mutex_lock(&hsLock);

    fd = open(confProp.sdCardDevNode, O_RDONLY);

    if (fd < 0)
    {
        LOG_ERROR("[%s:%d] Failed opening file \'%s\', returns with error no : %d, i.e., \'%s\'] \n",
                __FUNCTION__, __LINE__, confProp.sdCardDevNode, errno, strerror(errno));
        pthread_mutex_unlock(&hsLock);
        return false;
    }

    struct mmc_ioc_cmd idata;

    memset(hsbuf, 0, sizeof(struct health_status)); /*CID-19240*/
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

    LOG_INFO("[%s:%d] For CMD56: ioctl call returns: %d with error no : %d, (\'%s\') \n",
            __FUNCTION__, __LINE__, ret, errno, strerror(errno));

    if (ret != 0) {
        perror("ioctl");
        LOG_ERROR("[%s:%d] Failed: ioctl call returns: %d, error no : %d, i.e., \'%s\'] \n",
                __FUNCTION__, __LINE__, ret, errno, strerror(errno));

        memset(&idata, 0, sizeof(idata));
        idata.write_flag = 0;
        idata.opcode = 12;
        idata.arg = 0x00000000;
        idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
        ret = ioctl(fd, MMC_IOC_CMD, &idata);
        if (ret)
            perror("ioctl stop transmission");
        close(fd);
        /* Set the CMD56 status, so don't check 'LifeElapsed' for none supported CMD56 cmd SDcard*/
        isCMD56Enable = retVal;
        pthread_mutex_unlock(&hsLock);
        return retVal;
    }
    close(fd);

    LOG_DEBUG("CMD 56 returned 0\n");

    if (ret == 0)
    {
        hs = (struct health_status *) health_status_buffer;
        retVal = true;
        hsbuf->tsb_id = hs->tsb_id;
        if(hs->tsb_id == TSB_ID0 || (hs->tsb_id == TSB_ID1))
        {
            if(hs->man_date[0] != '\0')
                memcpy(hsbuf->man_date,hs->man_date,strlen(hs->man_date));
            hsbuf->used = hs->used;
            hsbuf->used_user_area = hs->used_user_area;
            hsbuf->used_spare_block = hs->used_spare_block;

            if(hs->customer_name[0] != '\0')
                memcpy(hsbuf->customer_name, hs->customer_name, strlen(hs->customer_name));
        }

        LOG_DEBUG("id=%x decimal id=%d date=%.6s used=%d  used_user_area=%d used_spare_block=%d customer_name=%.32s\n",
               hs->tsb_id, hs->tsb_id, hs->man_date, hs->used, hs->used_user_area, hs->used_spare_block, hs->customer_name);

#if 0
        idump = creat("health_status.bin", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (idump == -1)
            perror("health_status.bin");
        else {
            int rc = write(idump, health_status_buffer, sizeof(health_status_buffer));
            if (sizeof(health_status_buffer) != rc)
                perror("health_status.bin not all bytes written");
            close(idump);
        }
#endif

    }
    isCMD56Enable = retVal;
    pthread_mutex_unlock(&hsLock);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return retVal;
}



void get_SDCard_Properties_FromStatvfs(strMgrDeviceInfoParam_t *devInfoList)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    const char *file = "/etc/mtab";
    struct mntent *fs = NULL;
    FILE *fp = setmntent(file, "r");
    if (fp == NULL) {
        LOG_ERROR("[%s:%d] %s: could not open: %s\n", __FUNCTION__, __LINE__, file, strerror(errno));
    }

    while ((fs = getmntent(fp)) != NULL)
    {
        if(0 == strncasecmp(fs->mnt_fsname, confProp.sdCardSrcDevNode, strlen((const char *)MMC_SRC_DEV)))
        {
            get_statvfs(fs, devInfoList);
            break;
        }
    }

    endmntent(fp);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}


void get_statvfs(const struct mntent *fs, strMgrDeviceInfoParam_t *devInfoList)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if (fs->mnt_fsname[0] != '/')	/* skip nonreal filesystems */
        return;

    struct statvfs vfs;
    if (statvfs(fs->mnt_dir, & vfs) != 0) {
        LOG_ERROR("[%s:%d] %s: statvfs failed: %s\n", __FUNCTION__, __LINE__, fs->mnt_dir, strerror(errno));
        return;
    }

    /* For SD card */
    if(0 == strncasecmp(fs->mnt_fsname, confProp.sdCardSrcDevNode, strlen((const char *)MMC_SRC_DEV)))
    {
        if(devInfoList)
        {
            /*Actual values */
            devInfoList->capacity = (unsigned long long)vfs.f_bsize * vfs.f_blocks;
            devInfoList->free_space = (unsigned long long)vfs.f_bsize * vfs.f_bfree;
            memcpy(devInfoList->format, fs->mnt_type, strlen(fs->mnt_type));
            memcpy(devInfoList->mountPartition, fs->mnt_fsname, strlen(fs->mnt_fsname));
            devInfoList->fileSysflag = vfs.f_flag;
        }
    }

    LOG_DEBUG("%s, mounted on %s: of type: %s option: %s\n", fs->mnt_dir, fs->mnt_fsname, fs->mnt_type, fs->mnt_opts);
    LOG_DEBUG("\tf_bsize: %ld\n",  (long) vfs.f_bsize);
    LOG_DEBUG("\tf_frsize: %ld\n", (long) vfs.f_frsize);
    LOG_DEBUG("\tf_blocks: %lu\n", (unsigned long) vfs.f_blocks);
    LOG_DEBUG("\tf_bfree: %lu\n",  (unsigned long) vfs.f_bfree);
    LOG_DEBUG("\tf_bavail: %lu\n", (unsigned long) vfs.f_bavail);
    LOG_DEBUG("\tf_files: %lu\n",  (unsigned long) vfs.f_files);
    LOG_DEBUG("\tf_ffree: %lu\n",  (unsigned long) vfs.f_ffree);
    LOG_DEBUG("\tf_favail: %lu\n", (unsigned long) vfs.f_favail);
    LOG_DEBUG("\tf_fsid: %#lx\n",  (unsigned long) vfs.f_fsid);
    LOG_DEBUG("\tf_flag: %lu\n", (unsigned long) vfs.f_flag);

    LOG_DEBUG("\tf_flag: ");
    if (vfs.f_flag == 0)
        LOG_DEBUG("(none)\n");
    else {
        if ((vfs.f_flag & ST_RDONLY) != 0)
            LOG_DEBUG("ST_RDONLY ");
        if ((vfs.f_flag & ST_NOSUID) != 0)
            LOG_DEBUG("ST_NOSUID");
        LOG_DEBUG("\n");
    }

    LOG_DEBUG("\tf_namemax: %#ld\n", (long)vfs.f_namemax);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}



bool get_SDCard_PropertiesFromFile(strMgrDeviceInfoParam_t *devInfoList)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    bool status = false;

    if(isSDCardPresent())
    {
        get_SDCard_PropValueFromFile(devInfoList->devid, "/cid");
        get_SDCard_PropValueFromFile(devInfoList->manufacturer, "/manfid");

        LOG_DEBUG("[%s:%d] cid : %s\n", __FUNCTION__, __LINE__, devInfoList->devid);
        LOG_DEBUG("[%s:%d] manfid : %s\n", __FUNCTION__, __LINE__, devInfoList->manufacturer);

        status = true;
    }

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return status;
}


bool get_SDCard_PropValueFromFile(char *o_value, const char *in_filePath)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    char path[200] = {'\0'};

    if(sysPath_SDCard[0] == '\0') {
        LOG_DEBUG("[%s:%d] sysPath_SDCard : %s.\n", __FUNCTION__, __LINE__, sysPath_SDCard);
        return false;
    }
    else {
        LOG_DEBUG("[%s:%d] sysPath_SDCard : %s.\n", __FUNCTION__, __LINE__, sysPath_SDCard);
        strcpy(path, sysPath_SDCard);
        strcat(path, in_filePath);
    }

    std::ifstream filePath(path);
    try {
        if (filePath.is_open())
        {
            LOG_DEBUG("[%s:%d] Open path: %s \n", __FUNCTION__, __LINE__, path);
            std::string line;
            while ( std::getline (filePath,line))
            {
                LOG_DEBUG("[%s:%d] line : \'%s\' \n", __FUNCTION__, __LINE__, line.c_str());
                strcpy(o_value, line.c_str());
                LOG_DEBUG("[%s:%d] o_value : \'%s\' \n", __FUNCTION__, __LINE__, o_value);
                break;
            }
            filePath.close();
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("[%s:%d] Exception caught: %s\n", __FUNCTION__, __LINE__, e.what());
        return false;
    }
    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return true;
}

bool read_SDCard_cid(char *o_cid)
{
    bool ret = false;
    const char *cid_filePath = "/cid";

    ret = get_SDCard_PropValueFromFile(o_cid, cid_filePath);
    LOG_DEBUG("[%s:%d] CID of SD card: [%s]\n", __FUNCTION__, __LINE__, o_cid);

    return ret;
}

bool read_SDCard_manfid(char *o_manfid)
{
    bool ret = false;
    const char *manfid_filePath = "/manfid";

//	ret = get_SDCard_PropValueFromFile(in_sysPath, o_manfid, manfid_filePath);
    return ret;
}

bool read_SDCard_serial(char *o_serial)
{
    bool ret = false;
    const char *ser_filePath = "/serial";

    ret = get_SDCard_PropValueFromFile(o_serial, ser_filePath);
    return ret;
}


bool get_SDCard_Properties(strMgrDeviceInfoParam_t *devInfoList)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    bool bStatus = true;

    if(false == isSDCardPresent())
        return false;

    /*Read from statvfs system call for mounted device */
    get_SDCard_Properties_FromStatvfs(devInfoList);

    /*Read from sys file path of SD card */
    get_SDCard_PropertiesFromFile(devInfoList);

    /*Add other property values from file/others*/
    devInfoList->type = SM_DEV_SD_MMC;
    devInfoList->status = SM_DEV_STATUS_OK;
    devInfoList->isTSBSupported = sIsTSBSupported;
    devInfoList->isDVRSupported = false;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return bStatus;
}

bool mount_SDCard(const char* src, const char* trgt, const char  *fileSysType, smDeviceStatusCode_t *status)
{
    int retryCount = 0;
    int result = 0, ret = 0;
    char mountbuff[200] = {'\0'};
    const unsigned long mntFlag = 0;
    if(trgt[0] == 0 || fileSysType[0] == 0)
    {
        return false;
    }
    LOG_DEBUG("Mount info :\n"
           "src %s\n"
           "target: %s\n"
           "fileSystemType %s\n", src, trgt, fileSysType);

    sprintf(mountbuff, "%s %s %s", "/bin/mount",  src, trgt);
    while(true)
    {
        sync();
        ret = system(mountbuff);
        result = WEXITSTATUS(result);
//        result = mount(src, trgt, fileSysType, mntFlag, NULL);
        LOG_INFO("[%s:%d] ret = %d, result = %d\n", __FUNCTION__, __LINE__, ret, result);
        sync();
        if (result == 0)
        {
            LOG_DEBUG("SD card mount created at %s	\n", trgt);
            *status = SM_DEV_STATUS_OK;
            break;
        }
        else
        {
            if(-1 == result && errno == EBUSY)
            {
                LOG_DEBUG("SD card is already mounted to : %s. \n", trgt);
                *status = SM_DEV_STATUS_OK;
                break;
            }
            LOG_ERROR("[%s:%d] Error : Failed to mount %s\n Reason: %s [%d] with return value %d \n",
                    __FUNCTION__, __LINE__, src, strerror(errno), errno, result);
            retryCount++;
            if(retryCount >= 3)
            {
                *status = (EROFS == errno)?SM_DEV_STATUS_READ_ONLY: SM_DEV_STATUS_OK;
                return false;
            }
            else
            {
                LOG_INFO("[%s:%d] Retrying %d \n", __FILE__, __LINE__, retryCount);
                sleep(1);
                continue;
            }
        }
    }

    return true;
}

bool umount_SDCard(const char* src_file)
{
    int result = 0;
    short retryCount = 0;
    if(!src_file)
    {
        LOG_ERROR("[%s:%d] Failed to unmount: No device file present.\n", __FUNCTION__, __LINE__);
        return false;
    }
    LOG_DEBUG("Umount info : src %s\n", src_file);
    while(true)
    {
        sync();
        result = umount(src_file);
        sync();
        if (result == 0)
        {
            LOG_INFO("[%s:%d] SD card \'%s\' is unmounted. \n", __FUNCTION__, __LINE__, src_file);
            break;
        }
        else
        {
            LOG_ERROR("[%s:%d] Error : Failed to unmount %s\n Reason: %s [%d] with return value %d \n",
                    __FUNCTION__, __LINE__, src_file, strerror(errno), errno, result);
            if(-1 == result)
            {
                if(EINVAL == errno) {
                    break;
                }
                retryCount++;
                if(retryCount >= 10)
                    break;
                else
                {
                    LOG_ERROR("[%s:%d] Failed to umount: retrying %d times.\n", __FUNCTION__, __LINE__, retryCount);
                    sleep(5);
                    continue;
                }
            }
        }
    }
    return true;
}

//#ifdef USE_DISK_CHECK
bool execute_Mount_Script()
{
    int ret = 0, ret_ExStat = 0;
    char mountbuff[200] = {'\0'};

    pthread_mutex_lock(&mountLock);

    if(isMounted == true) {
        LOG_INFO("[%s:%d] Already Mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&mountLock);
        return true;
    }

    /* Now mount : disk_check mount /dev/mmcpblkp01 /media/tsb*/
    sprintf(mountbuff, "%s %s %s %s %s", "sh", confProp.disk_check_script, "mount", confProp.devFile, confProp.sdCardMountPath);

    ret = system(mountbuff);
    ret_ExStat = WEXITSTATUS(ret);

    LOG_INFO("[%s:%d] Executed : \'%s\', return as [%d]\n", __FUNCTION__, __LINE__, mountbuff, ret_ExStat);

    if(!ret_ExStat)
    {
        LOG_INFO("[%s:%d] Successfully mounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&mountLock);
        isMounted = true;
        return true;
    }
    else
    {
        LOG_ERROR("[%s:%d] Fail to mount.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&mountLock);
        return false;
    }
}


bool execute_Umount_Script()
{
    int ret = 0, ret_ExStat = 0;
    char umountbuff[200] = {'\0'};

    if(isMounted == false) {
        LOG_INFO("[%s:%d] Already Umounted.\n", __FUNCTION__, __LINE__);
        return false; // TBD: return true?
    }

    pthread_mutex_lock(&mountLock);

    /* Now mount : disk_check umount /media/tsb*/
    sprintf(umountbuff, "%s %s %s %s", "sh", confProp.disk_check_script, "umount", confProp.devFile, confProp.sdCardMountPath);

    ret = system(umountbuff);
    ret_ExStat = WEXITSTATUS(ret);

    LOG_INFO("[%s:%d] Executed : \'%s\', return as [%d]\n", __FUNCTION__, __LINE__, umountbuff, ret_ExStat);

    if(!ret_ExStat)
    {
        LOG_INFO("[%s:%d] Successfully umounted.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&mountLock);
        isMounted = false;
        return true;
    }
    else
    {
        LOG_ERROR("[%s:%d] Fail to umount.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&mountLock);
        return false;
    }
}

void createFile(char *fileName)
{
    int ret = 0;
    ret = creat( fileName, 0666 );
    if ( ret >= 0 ) {
        LOG_INFO("[%s:%d] File is successfully created.\n", __FUNCTION__, __LINE__);
    }
    else {
        LOG_ERROR("[%s:%d] Fail to create.\n", __FUNCTION__, __LINE__);
    }
}

/*
 * isSDCardMounted :
 * If mounted, then return true;
 * else if not mounted then report as false;
 */
bool isSDCardMounted()
{
    return isMounted;
}

eTsbStatus get_SDcardTsbHealthMonStatus()
{
    return tsb_HM_Status;
}

/** @} */
/** @} */
