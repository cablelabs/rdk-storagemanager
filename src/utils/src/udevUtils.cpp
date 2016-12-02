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


#include "udevUtils.h"
#include "sdCardReader.h"
#include "storageMgr.h"
#include "storageMgrInternal.h"

static bool checkUdevDevices(struct udev *device, const char *subSystem, char *sysPath);
static bool isMmcDevPresent(struct udev_device *device);
static void printUdevDeviceInfo(struct udev_device *dev, const char *src);
static bool isDeviceAdded(struct udev_device *device);


void stMgr_udev_Init();
static void stMgr_udev_Init_RootObj();
static void stMgr_Udev_Monitor();
static bool stMgr_check_udev_SDCard();

extern char sysPath_SDCard[200];
extern sdCardInfo sdCardProp;
extern stmMgrConfigProp confProp;
extern strMgrDeviceInfoParam_t gdeviceInfo;

void *stMgrMonitorThead(void *ptr)
{
    stMgr_udev_Init();
    stMgr_Udev_Monitor();
    stMgr_udev_Deinit();
    return ptr; /*CID-19230*/
}


void stMgr_udev_Init()
{
    stMgr_udev_Init_RootObj();

}

static void stMgr_udev_Init_RootObj()
{
    /* Initialized root udev Object*/
    if(NULL == _udevDevInstance)
    {
        _udevDevInstance = udev_new();
    }
}


void stMgr_udev_Deinit()
{
    if(_udevDevInstance)
        udev_unref(_udevDevInstance);
}


static void stMgr_Udev_Monitor()
{
    bool status= false;
    struct udev_monitor *udev_monitor = NULL;
    fd_set readfds;
    IARM_Bus_STRMgr_EventData_t eventData;
    const char *udev_action = NULL;
    smDeviceStatusCode_t errStatus = SM_DEV_STATUS_OK;
    unsigned short fileStatus = SM_DEV_STATUS_OK;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    /* Create the udev object */
    if(_udevDevInstance)
    {
        /*Check for MMC Device (SD Card) */
        if( stMgr_check_udev_SDCard())
        {
            LOG_INFO("[%s:%d] SDCard is present.\n", __FUNCTION__, __LINE__);
            setSDCardPresenceFlag(true);

            /* Read CID*/
            status = read_SDCard_cid(gdeviceInfo.devid);

            if(!status) {
                LOG_ERROR("[%s:%d] Failed to read CID structure of SD card\n", __FUNCTION__, __LINE__);
            }

            if(status) {
                status = check_tsb_Supported();
            }

            if(status) {
#ifdef USE_DISK_CHECK
                status = execute_Mount_Script();

#elif USE_SYSTEMD_SERVICE
                LOG_INFO("[%s:%d] Use systemD events for mounting.\n", __FUNCTION__, __LINE__);
#else
                status = mount_SDCard(confProp.sdCardSrcDevNode, confProp.sdCardMountPath, confProp.fileSysType, &errStatus);
#endif
            }

#ifdef ENABLE_SD_NOTIFY
            sd_notifyf(0, "READY=1\n"
                       "STATUS=storageMgrMain is ready, executed Mount for SDcard\n" "MAINPID=%lu",  (unsigned long) getpid());
#endif

            if(status)
            {
                LOG_INFO("[%s:%d] SDCard mounted Successfully.\n", __FUNCTION__, __LINE__);

                /*Check for f_flag for RO flag. */
                get_SDCard_Properties_FromStatvfs(&gdeviceInfo);

#ifdef USE_DISK_CHECK
                if((gdeviceInfo.fileSysflag & ST_RDONLY)!= 0)
#else
                if(errStatus == SM_DEV_STATUS_READ_ONLY || gdeviceInfo.fileSysflag == ST_RDONLY)
#endif
                {
                    setSDCard_TSB_Supported(false);
                    gdeviceInfo.status = SM_DEV_STATUS_READ_ONLY;
                    LOG_INFO("[%s:%d] SDcard is mounted as \"read-only\", [f_flag:%d] . \t  \n",
                            __FUNCTION__, __LINE__, gdeviceInfo.fileSysflag);
                }
                else
                {
                    gdeviceInfo.status = SM_DEV_STATUS_READY;
                    setsdCardStatusOnReadyEvent(true);
                }

                /*Trigger event*/
                memset(&eventData, 0, sizeof(eventData));
                memcpy(eventData.data.cardStatus.deviceId, gdeviceInfo.devid, strlen(gdeviceInfo.devid));

                eventData.data.cardStatus.status = (smDeviceStatusCode_t)gdeviceInfo.status;

                if (gdeviceInfo.status == SM_DEV_STATUS_READY) {
                    setsdCardStatusOnReadyEvent(true);
                }
                IARM_Bus_BroadcastEvent(IARM_BUS_ST_MGR_NAME, (IARM_EventId_t) IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged, (void *)&eventData, sizeof(eventData));
                LOG_INFO("[%s:%d] \"%s\" sends SDCard \'%s\' \"OnDeviceStatusChanged\" IARM_Bus_BroadcastEvent with values as \"%s:%d\".\n",
                       __FUNCTION__, __LINE__, IARM_BUS_ST_MGR_NAME,
                       (gdeviceInfo.status == SM_DEV_STATUS_READY) ? "SM_DEV_STATUS_READY" : "SM_DEV_STATUS_READ_ONLY",
                       eventData.data.cardStatus.deviceId, eventData.data.cardStatus.status);
            }
            else
            {
                setSDCard_TSB_Supported(false);
                LOG_ERROR("[%s:%d] Failed to mount SDCard, so disabling tsb.\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            LOG_INFO("[%s:%d] SDCard is not present.\n", __FUNCTION__, __LINE__);
            setSDCardPresenceFlag(false);
#ifdef ENABLE_SD_NOTIFY
            sd_notifyf(0, "READY=1\n"
                       "STATUS=storageMgrMain is Ready with No SDcard.\n" "MAINPID=%lu",  (unsigned long) getpid());
#endif
        }

        udev_monitor = udev_monitor_new_from_netlink(_udevDevInstance, "udev");
        if (udev_monitor == NULL) {
            LOG_ERROR("[%s:%d] udev_monitor_new_from_netlink FAILED.\n", __FUNCTION__, __LINE__);
//			return 1;
        }

        //add some filters
        if( udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, SUBSYSTEM_FILTER_MMC, NULL) < 0 )
        {
            LOG_ERROR("[%s:%d] udev_monitor_filter_add_match_subsystem_devtype FAILED\n", __FUNCTION__, __LINE__);
//			return 1;
        }

        if (udev_monitor_enable_receiving(udev_monitor) < 0)
        {
            LOG_ERROR("[%s:%d] udev_monitor_enable_receiving FAILED\n", __FUNCTION__, __LINE__);
//			return 1;
        }

        while (true)
        {
            int fdcount = 0;
            struct timeval tv;

            FD_ZERO(&readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            if (udev_monitor != NULL)
            {
                FD_SET(udev_monitor_get_fd(udev_monitor), &readfds);
            }

            fdcount = select(udev_monitor_get_fd(udev_monitor)+1, &readfds, NULL, NULL, NULL);
            if (fdcount < 0)
            {
                if (errno != EINTR)
                    LOG_ERROR("[%s:%d] Error receiving uevent message\n", __FUNCTION__, __LINE__);
                continue;
            }

            if ((udev_monitor != NULL) && FD_ISSET(udev_monitor_get_fd(udev_monitor), &readfds))
            {
                struct udev_device *device;

                device = udev_monitor_receive_device(udev_monitor);

                if (device == NULL) {
                    LOG_ERROR("[%s:%d] No Device from receive_device(). An error occurred.\n", __FUNCTION__, __LINE__);
                    continue;
                }

                udev_action = udev_device_get_action(device);

                LOG_INFO("[%s:%d] udev_device_get_action() returns: \'%s\'\n", __FUNCTION__, __LINE__, udev_action);

                if(0 == strcasecmp(udev_action, "add"))
                {
                    //check presence
                    if( isMmcDevPresent(device))
                    {
                        if(!isSDCardPresent())
                        {
                            sync();
                            LOG_INFO("[%s:%d] SD Card plugged in...!!! \n", __FUNCTION__, __LINE__);
                            sleep(2);

                            const char *sys_dev_path = udev_device_get_syspath(device);
                            LOG_DEBUG( "[%s:%d] Device syspath: \"%s\"\n", __FUNCTION__, __LINE__, sys_dev_path);
                            memset(sysPath_SDCard, 0, 200);
                            memcpy(sysPath_SDCard, sys_dev_path, strlen(sys_dev_path)+1 );

                            /* Read CID*/
                            status = read_SDCard_cid(gdeviceInfo.devid);

                            if(!status) {
                                LOG_ERROR("[%s:%d] Failed to read CID of SD card\n", __FUNCTION__, __LINE__);
                            }

                            /*Set SD card presence field*/
                            setSDCardPresenceFlag(true);

                            /*Check TSB status card presence field*/
                            if(status)
                                status = check_tsb_Supported();

                            if(status)
                            {
#ifdef USE_DISK_CHECK
                                status = execute_Mount_Script();
#elif USE_SYSTEMD_SERVICE
                                LOG_INFO("[%s:%d] Use systemD events for mounting.", __FUNCTION__, __LINE__);
#else
                                status = mount_SDCard(confProp.sdCardSrcDevNode, confProp.sdCardMountPath, confProp.fileSysType, &errStatus);
#endif
                            }

                            if(status)
                            {
                                LOG_INFO("[%s:%d] SDCard mounted Successfully. \n", __FUNCTION__, __LINE__);
                                get_SDCard_Properties_FromStatvfs(&gdeviceInfo);
                                LOG_INFO("[%s:%d] gdeviceInfo.fileSysflag = %d \t errStatus = %d \n",
                                        __FUNCTION__, __LINE__, gdeviceInfo.fileSysflag, errStatus);
#ifdef USE_DISK_CHECK
                                if((gdeviceInfo.fileSysflag & ST_RDONLY)!= 0)
#else
                                if(errStatus == SM_DEV_STATUS_READ_ONLY || gdeviceInfo.fileSysflag == ST_RDONLY)
#endif
                                {
                                    setSDCard_TSB_Supported(false);
                                    setsdCardStatusOnReadyEvent(false);
                                    gdeviceInfo.status = SM_DEV_STATUS_READ_ONLY;
                                    LOG_INFO("[%s:%d] SDcard is mounted as \"read-only\", [f_flag:%d] . \t  \n",
                                            __FUNCTION__, __LINE__, gdeviceInfo.fileSysflag);
                                }
                                else
                                {
                                    gdeviceInfo.status = SM_DEV_STATUS_OK;
                                    setsdCardStatusOnReadyEvent(true);
                                }

                                /*Trigger event*/
                                memset(&eventData, 0, sizeof(eventData));
                                memcpy(eventData.data.cardStatus.deviceId, gdeviceInfo.devid, strlen(gdeviceInfo.devid));
                                eventData.data.cardStatus.status = (smDeviceStatusCode_t)gdeviceInfo.status;
                                IARM_Bus_BroadcastEvent(IARM_BUS_ST_MGR_NAME, (IARM_EventId_t) IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged, (void *)&eventData, sizeof(eventData));
                                LOG_INFO("[%s:%d] \"%s\" sends \"OnDeviceStatusChanged\" IARM_Bus_BroadcastEvent on \"%s\" with values as \"%s:%d\".\n",
                                       __FUNCTION__, __LINE__, IARM_BUS_ST_MGR_NAME, udev_action,
                                       eventData.data.cardStatus.deviceId, eventData.data.cardStatus.status);
                            }
                            else
                            {
                                setSDCard_TSB_Supported(false);
                                LOG_ERROR("[%s:%d] Failed to mount SDCard, so disabling tsb.\n", __FUNCTION__, __LINE__);
                            }
                        }
                    }
                }
                else if (0 == strcasecmp(udev_action, "remove"))
                {
                    if(isSDCardPresent())
                    {
                        sync();
                        LOG_INFO("[%s:%d] SD Card unplugged...!!! \n", __FUNCTION__, __LINE__);
                        setSDCardPresenceFlag(false);
                        gdeviceInfo.status = SM_DEV_STATUS_NOT_PRESENT;
                        /*Trigger event*/
                        memset(&eventData, 0, sizeof(eventData));
                        memcpy(eventData.data.cardStatus.deviceId, gdeviceInfo.devid, strlen(gdeviceInfo.devid));
                        eventData.data.cardStatus.status = SM_DEV_STATUS_NOT_PRESENT;
                        IARM_Bus_BroadcastEvent(IARM_BUS_ST_MGR_NAME, (IARM_EventId_t) IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged, (void *)&eventData, sizeof(eventData));
                        LOG_INFO("[%s:%d] \"%s\" sends \"OnDeviceStatusChanged\" IARM_Bus_BroadcastEvent on \"%s\" with values as \"%s:%d\".\n",
                               __FUNCTION__, __LINE__, IARM_BUS_ST_MGR_NAME, udev_action,
                               eventData.data.cardStatus.deviceId, eventData.data.cardStatus.status);

                        setsdCardStatusOnReadyEvent(false);
#ifdef USE_DISK_CHECK
                        execute_Umount_Script();
#elif USE_SYSTEMD_SERVICE
                        LOG_INFO("[%s:%d] Use systemD events for umounting.", __FUNCTION__, __LINE__);
#else
                        if(umount_SDCard(confProp.sdCardMountPath))
                        {
                            LOG_INFO("[%s:%d] SDCard unmounted. \n", __FUNCTION__, __LINE__);
                            memset(gdeviceInfo.devid, '\0', DEV_ID_BUF);
                        }
#endif
                    }
                }
                else
                {
                    LOG_ERROR("[%s:%d] Invalid Action..!!! (%s)\n", __FUNCTION__, __LINE__, udev_action);
                }
            }
            usleep(250*1000);
        }
    }
    else
    {
#ifdef ENABLE_SD_NOTIFY
        sd_notifyf(0, "READY=1\n"
                   "STATUS=storageMgrMain is Ready, but failed to start udev monitor context.\n" "MAINPID=%lu",  (unsigned long) getpid());
#endif
    }
    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}

static bool stMgr_check_udev_SDCard()
{
    bool retVal = false;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    const char *sysDevPath = NULL;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    enumerate = udev_enumerate_new(_udevDevInstance);

    /* udev_enumerate_add_match_subsystem */
    udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM_FILTER_MMC);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    /* Iterate through the enumerated list. */
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        struct udev_device *dev;
        const char* dev_path = udev_list_entry_get_name(dev_list_entry);

        LOG_DEBUG("[%s:%d] Device Path : %s\n", __FUNCTION__, __LINE__, dev_path);
        LOG_DEBUG("[%s:%d] Dev Path Value : %s\n", __FUNCTION__, __LINE__, udev_list_entry_get_value(dev_list_entry));

        dev = udev_device_new_from_syspath(_udevDevInstance, dev_path);

        if( isMmcDevPresent(dev))
        {
            retVal = true;
            sysDevPath = udev_device_get_syspath(dev);
            memcpy(sysPath_SDCard, sysDevPath, strlen(sysDevPath));
            udev_device_unref(dev);
            break;
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return retVal;
}




static bool isMmcDevPresent(struct udev_device *device)
{
    bool isSDPresent = false;
    struct udev_list_entry *listEntry = NULL, *valEntry = NULL;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    LOG_DEBUG("[%s:%d] Syspath: %s\n", __FUNCTION__, __LINE__, udev_device_get_syspath(device));

//    memcpy(sysPath_SDCard, sysDevPath, strlen(sysDevPath));

    listEntry = udev_device_get_properties_list_entry(device);
    if(!listEntry)
        return isSDPresent;

    valEntry = udev_list_entry_get_by_name(listEntry, ATTR_FILTER_MMC);

    if(!valEntry)
        return isSDPresent;

    const char* value = udev_list_entry_get_value(valEntry);

    LOG_DEBUG("[%s:%d] Value: %s\n", __FUNCTION__, __LINE__, value);

    if(0 == strcmp( value, ATTR_VALUE_SD))
    {
        LOG_INFO("[%s:%d] Udev device is \'SD\' \n", __FUNCTION__, __LINE__);
        isSDPresent = true;

#ifdef DEBUG_LOGS
        printUdevDeviceInfo(device, "UDEV");
#endif

    }
    else if(0 == strcmp( value, ATTR_VALUE_MMC))
    {
        LOG_INFO("[%s:%d] Udev device is \'eMMC\' \n", __FUNCTION__, __LINE__);
        isSDPresent = true;

#ifdef DEBUG_LOGS
        print_device_info(device, "UDEV");
#endif

    }

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return isSDPresent;
}

static bool isDeviceAdded(struct udev_device *device)
{
    bool retVal = false;
    struct udev_list_entry *list_entry = 0;
    struct udev_list_entry* added_disk_entry = 0;


    list_entry = udev_device_get_properties_list_entry(device);
    added_disk_entry = udev_list_entry_get_by_name(list_entry,/* "DEVNAME" */ ATTR_ADDED_DISK);
    if( 0 != added_disk_entry )
    {
        retVal = true;
    }
    return retVal;
}


static void printUdevDeviceInfo(struct udev_device *dev, const char *src)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    struct udev_list_entry *uDevListEntry = NULL;

    LOG_INFO("[%s:%d] Udev action \"%-7s\", %s, (%s)\n", __FUNCTION__, __LINE__, src, udev_device_get_action(dev), udev_device_get_devpath(dev),udev_device_get_subsystem(dev));
    /*Iterate all the udev entries*/
    LOG_INFO("*************************************************\n");
    udev_list_entry_foreach(uDevListEntry, udev_device_get_properties_list_entry(dev))
    LOG_INFO("[%s:%d] [\'%s\' : \'%s\'\n", __FUNCTION__, __LINE__,  udev_list_entry_get_name(uDevListEntry), udev_list_entry_get_value(uDevListEntry));
    LOG_INFO("*************************************************\n");

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}




/** @} */
/** @} */
