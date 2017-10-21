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
#include "rdkStorageMgr.h"
#include "rdkStorageMgrTypes.h"
#include "rdkStorageMgrLogger.h"

/* For Udev Access */
#include <libudev.h>

/* For file access */
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

/* Storage Main Class */
#include "rdkStorageMain.h"

#define EMMC_TSB_DEV_NODE "/dev/mmcblk0"

/* Global variables */
pthread_t stmgrDeviceMonitorTID;
struct udev* g_uDevInstance;

/* Consts */
const char* STMGR_DEBUG_ACTUAL_PATH    = "/etc/debug.ini";
const char* STMGR_DEBUG_OVERRIDE_PATH  = "/opt/debug.ini";

const char* STMGR_CONFIG_ACTUAL_PATH    = "/etc/stmgrConfig.ini";
const char* STMGR_CONFIG_OVERRIDE_PATH  = "/opt/stmgrConfig.ini";

/* Local Functions */
void rStorage_logInit (char* pDebug)
{
    static bool isInited = false;
    const char* pDebugConfig = NULL;

    if (!isInited)
    {
        if (!pDebug)
        {
            /* check the logger config */
            if( access(STMGR_DEBUG_OVERRIDE_PATH, F_OK) != -1 )
                pDebugConfig = STMGR_DEBUG_OVERRIDE_PATH;
            else
                pDebugConfig = STMGR_DEBUG_ACTUAL_PATH;
        }
        else
        {
            pDebugConfig = pDebug;
        }

        rdk_logger_init(pDebugConfig);
        STMGRLOG_INFO("Loger Inited Successfully\n");
    }
    else
    {
        STMGRLOG_INFO("Already Logger Inited Successfully\n");
    }

    return;
}

void rStorage_configInit (char* pConfig)
{

}

void*  deviceConnectRemoveMonitorfn  (void *pData)
{
    /* Start the UDEV monitoring for those Memory units including USB */
    struct udev_monitor *pUDevMonitor = NULL;
    const char *pUDevAction = NULL;
    fd_set rFileDescriptors;

    pUDevMonitor = udev_monitor_new_from_netlink(g_uDevInstance, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(pUDevMonitor, "mmc", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(pUDevMonitor, "block", NULL);
    udev_monitor_enable_receiving(pUDevMonitor);

    while (1)
    {
        int fdcount = 0;
        FD_ZERO(&rFileDescriptors);

        FD_SET(udev_monitor_get_fd(pUDevMonitor), &rFileDescriptors);
        fdcount = select( (udev_monitor_get_fd(pUDevMonitor) + 1), &rFileDescriptors, NULL, NULL, NULL);
        if (fdcount < 0)
        {
            if (errno != EINTR)
                STMGRLOG_ERROR("Could not receive uDev event message\n");
            continue;
        }

        if (FD_ISSET(udev_monitor_get_fd(pUDevMonitor), &rFileDescriptors))
        {
            struct udev_device *pDevice;
            pDevice = udev_monitor_receive_device(pUDevMonitor);
            
            if (pDevice == NULL) {
                STMGRLOG_ERROR("Error Occurrec; Could not get a Device from receive_device()...\n");
                continue;
            }

            pUDevAction = udev_device_get_action(pDevice);
            STMGRLOG_INFO("udev_device_get_action() returns: %s\n", pUDevAction);
            const char* pDevType = NULL;
            const char* pDevPath = NULL;
            std::string deviceType;
            std::string devicePath;
            pDevType = udev_device_get_devtype(pDevice);
            if(pDevType)
            {
              deviceType = pDevType;
            }
            pDevPath = udev_device_get_devnode(pDevice);
            if(pDevPath)
            {
              devicePath = udev_device_get_devnode(pDevice);
            }
            eSTMGRReturns retCode = RDK_STMGR_RETURN_INVALID_INPUT;

                STMGRLOG_INFO ("Sys    Path : %s\n", udev_device_get_syspath(pDevice));
                STMGRLOG_INFO ("Device Path : %s\n", udev_device_get_devpath(pDevice));
                STMGRLOG_INFO ("Device Type : %s\n", udev_device_get_devtype(pDevice));
                STMGRLOG_INFO ("Device Node : %s\n", udev_device_get_devnode(pDevice));
            /* Get the device Type and see whether it is DISK device */
            if ((!deviceType.empty()) && (strcmp (deviceType.c_str(), "disk") == 0))
            {
                if(0 == strcasecmp(pUDevAction, "add"))
                {
                    /* Add the new device */
                    struct udev_device *pParentDevice = NULL;
                    pParentDevice = udev_device_get_parent_with_subsystem_devtype(pDevice, "mmc", NULL);
                    if (pParentDevice)
                    {
                        STMGRLOG_INFO ("IS MMC TYPE : Yes\n");
                        rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_SDCARD);
                    }
                    else
                    {
                        /* FIXME: Get the device Class and Decide on the device type; for now HDD */
                        const char* pDevBus = udev_device_get_property_value(pDevice, "ID_BUS");
                        STMGRLOG_INFO ("ID_BUS      : %s\n", pDevBus);
                        if ((pDevBus) && ((0 == strcasecmp(pDevBus, "scsi")) || (0 == strcasecmp(pDevBus, "ata"))))
                            retCode = rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_HDD);
                        else if ((pDevBus) && (0 == strcasecmp(pDevBus, "usb")))
                            retCode = rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_USB);
                    }
                }
                else if (0 == strcasecmp(pUDevAction, "remove"))
                {
                    /* Remove the device from monitoring */
                    retCode = rSTMgrMainClass::getInstance()->deleteMemoryDevice(devicePath);
                }
                else
                {
                    STMGRLOG_INFO("Invalid Action..!!! (%s)\n", pUDevAction);
                }

                if (retCode != RDK_STMGR_RETURN_SUCCESS)
                {
                    /**/
                    STMGRLOG_ERROR("Add/Remove device Failed\n");
                }
            }
        }
        sleep(5);
    }

    return NULL;
}

/* Public functions */
void rdkStorage_init (void)
{
    /* log init */
    rStorage_logInit(NULL);

    /* Config file parsing */
    rStorage_configInit(NULL);
 
    /* if not uDev, use some other way to identify the presense of storage mediums and add create the instances */
    g_uDevInstance = udev_new();

    {
        /* Check the presense of HDD; Create n Add it to the map */
        struct udev_device *pDevice = NULL;
        struct udev_device *pParentDevice = NULL;
        struct udev_enumerate *pEnumerate = NULL;
        struct udev_list_entry *pDeviceList = NULL;
        struct udev_list_entry *pDeviceListEntry = NULL;
        bool isNVRAMDeviceFound = false;

        /* FIXME: This below check can be removed as the ubi enumeratation search below will take care */
        /* This ensures that the disk that is identified is UBI subsystem */
        if (!isNVRAMDeviceFound)
        {
            pParentDevice = udev_device_new_from_subsystem_sysname (g_uDevInstance, "ubi", "ubi0");
            if (pParentDevice)
            {
                const char* pData = NULL;
                std::string devicePath;
                std::string sysname = udev_device_get_sysname(pParentDevice);
                isNVRAMDeviceFound = true;
                STMGRLOG_INFO ("IS UBI/NVRAM TYPE : Yes\n");
                STMGRLOG_INFO ("Sys  Name  : %s\n", sysname.c_str());
                pData = udev_device_get_devnode(pParentDevice);
                if (pData)
                {
                    devicePath = pData;
                }
                else
                {
                    STMGRLOG_INFO ("UBI/NVRAM TYPE is Yes but could not able to get dev node.. lets concatenate the /dev with sysname\n");
                    devicePath = "/dev/" + sysname;
                }
                rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_NVRAM);
            }
        }

        /* Loop in for other disk devices */
        pEnumerate = udev_enumerate_new(g_uDevInstance);
        udev_enumerate_add_match_subsystem(pEnumerate, "block");
        if(!isNVRAMDeviceFound)
            udev_enumerate_add_match_subsystem(pEnumerate, "ubi");

        if (0 != udev_enumerate_scan_devices(pEnumerate))
        {
            STMGRLOG_ERROR("Failed to scan the devices \n");
        }

        pDeviceList = udev_enumerate_get_list_entry(pEnumerate);
        udev_list_entry_foreach(pDeviceListEntry, pDeviceList)
        {
            const char* pSysPath = NULL;
            const char* pDevType = NULL;
            const char* pDevBus = NULL;
            const char* pSysAttrType = NULL;
            pSysPath = udev_list_entry_get_name(pDeviceListEntry);
            pDevice = udev_device_new_from_syspath(g_uDevInstance, pSysPath);
            pDevType = udev_device_get_devtype(pDevice);
            STMGRLOG_DEBUG ("Sys  2 Path : %s\n", udev_device_get_syspath(pDevice));
            STMGRLOG_DEBUG ("Sys  2 Path : %s\n", pSysPath);
            STMGRLOG_DEBUG ("Device Path : %s\n", udev_device_get_devpath(pDevice));
            STMGRLOG_DEBUG ("Device Type : %s\n", udev_device_get_devtype(pDevice));

            if((!isNVRAMDeviceFound) && (pSysPath) && (strstr(pSysPath, "ubi")))
            {
                pParentDevice = udev_device_get_parent_with_subsystem_devtype(pDevice, "ubi", NULL);
                if (pParentDevice)
                {
                    const char* pData = NULL;
                    std::string devicePath;
                    std::string sysname = udev_device_get_sysname(pParentDevice);
                    isNVRAMDeviceFound = true;
                    STMGRLOG_INFO ("IS UBI/NVRAM TYPE : Yes\n");
                    STMGRLOG_INFO ("Sys 3 Name  : %s\n", sysname.c_str());
                    pData = udev_device_get_devnode(pParentDevice);
                    STMGRLOG_INFO ("Sys 3 Path  : %s\n", pData);
                    if (pData)
                        devicePath = pData;
                    else
                    {
                        STMGRLOG_INFO ("UBI/NVRAM TYPE is Yes but could not able to get dev node.. lets make it up\n");
                        devicePath = "/dev/" + sysname;
                    }
                    rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_NVRAM);
                }
            }
            if ((pDevType) && (strcasecmp (pDevType, "disk") == 0))
            {
                pParentDevice = NULL;
                const char* pDevNode = NULL;
                std::string devicePath;
                pDevNode = udev_device_get_devnode(pDevice);
                if(pDevNode)
                {
                  devicePath = udev_device_get_devnode(pDevice);
                }
                STMGRLOG_INFO ("Sys    Path : %s\n", udev_device_get_syspath(pDevice));
                STMGRLOG_INFO ("Device Path : %s\n", udev_device_get_devpath(pDevice));
                STMGRLOG_INFO ("Device Type : %s\n", udev_device_get_devtype(pDevice));
                STMGRLOG_INFO ("Device Node : %s\n", pDevNode);

                //Check the presense of SDCard; Create n Add it to the map

                /* This ensures that the disk that is identified is MMC subsystem */
                if(pDevNode) 
                {
                    pParentDevice = udev_device_get_parent_with_subsystem_devtype(pDevice, "mmc", NULL);
                    if (pParentDevice)
                    {
                        pSysAttrType = udev_device_get_sysattr_value(pParentDevice, "type");
                        if ((pSysAttrType && (strcasecmp (pSysAttrType, "MMC") == 0)) &&
                            (pDevNode && (strcasecmp (pDevNode, EMMC_TSB_DEV_NODE) == 0)))
                        {
                            STMGRLOG_INFO ("IS MMC TYPE : Yes\n");
                            rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_EMMCCARD);
                        }
                        else if (pSysAttrType && (strcasecmp (pSysAttrType, "SD") == 0))
                        {
                            STMGRLOG_INFO ("IS SD TYPE : Yes\n");
                            rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_SDCARD);
                        }
                    }
                    else
                    {
                        /* This check is to avoid RAM & other devices */
                        pParentDevice = udev_device_get_parent (pDevice);
                        if (pParentDevice)
                        {
                            pDevBus = udev_device_get_property_value(pDevice, "ID_BUS");
                            STMGRLOG_INFO ("ID_BUS      : %s\n", pDevBus);

                            if ((pDevBus) && ((0 == strcasecmp(pDevBus, "scsi")) || (0 == strcasecmp(pDevBus, "ata"))))
                            {
                                rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_HDD);
                            }
                            else if ((pDevBus) && (0 == strcasecmp(pDevBus, "usb")))
                                rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_USB);
                            else
                            {
                                /* FIXME */
                                STMGRLOG_ERROR("Unhandled DISK Storage found!!! Must be analyzed and handled.\n");
                            }
                        }
                    }
                }
            }
            else 
            {
                STMGRLOG_ERROR("Failed to addNewMemoryDevice for storage. Device Node is NULL !!! Must have valid \'dev/<node>\'.\n");
            }
        }

        /* NVRAM */
        if (!isNVRAMDeviceFound)
        {
            STMGRLOG_ERROR ("Seems like this platform does not use/support NVRAM\n");
        }
    }


    /* Start the Polling thread which does the health Monitoring */
    pthread_create(&stmgrDeviceMonitorTID, NULL, deviceConnectRemoveMonitorfn, (void*) g_uDevInstance);

    return;
}

/* Get DeviceIDs*/
eSTMGRReturns rdkStorage_getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getDeviceIds(pDeviceIDs);
}

/* Get DeviceInfo */
eSTMGRReturns rdkStorage_getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getDeviceInfo(pDeviceID, pDeviceInfo);
}

/* Get DeviceInfoList */
eSTMGRReturns rdkStorage_getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getDeviceInfoList(pDeviceInfoList);
}

/* Get PartitionInfo */
eSTMGRReturns rdkStorage_getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getPartitionInfo (pDeviceID, pPartitionId, pPartitionInfo);
}

/* Get TSBStatus */
eSTMGRReturns rdkStorage_getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBStatus (pTSBStatus);
}

/* Set TSBMaxMinutes */
eSTMGRReturns rdkStorage_setTSBMaxMinutes (unsigned int minutes)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->setTSBMaxMinutes (minutes);
}

/* Get TSBMaxMinutes */
eSTMGRReturns rdkStorage_getTSBMaxMinutes (unsigned int *pMinutes)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBMaxMinutes (pMinutes);
}

/* Get TSBCapacityMinutes */
eSTMGRReturns rdkStorage_getTSBCapacityMinutes(unsigned int *pMinutes)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBCapacityMinutes(pMinutes);
}

/* Get TSBCapacity*/
eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long long *pCapacity)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBCapacity(pCapacity);
}

/* Get TSBFreeSpace*/
eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long long *pFreeSpace)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBFreeSpace(pFreeSpace);
}

/* Get DVRCapacity */
eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long long *pCapacity)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getDVRCapacity(pCapacity);
}

/* Get DVRFreeSpace*/
eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long long *pFreeSpace)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getDVRFreeSpace(pFreeSpace);
}

/* Get isTSBEnabled*/
bool rdkStorage_isTSBEnabled()
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->isTSBEnabled();
}

/* Set TSBEnabled */
eSTMGRReturns rdkStorage_setTSBEnabled (bool isEnabled)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->setTSBEnabled(isEnabled);
}

/* Get isDVREnabled*/
bool rdkStorage_isDVREnabled()
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->isDVREnabled();
}

/* Set DVREnabled */
eSTMGRReturns rdkStorage_setDVREnabled (bool isEnabled)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->setDVREnabled(isEnabled);
}

/* Get Health */
eSTMGRReturns rdkStorage_getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getHealth (pDeviceID, pHealthInfo);
}

eSTMGRReturns rdkStorage_getTSBPartitionMountPath (char* pMountPath)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->getTSBPartitionMountPath(pMountPath);
}

void rdkStorage_notifyMGRAboutFailure (eSTMGRErrorEvent failEvent)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    rSTMgrMainClass::getInstance()->notifyMGRAboutFailure(failEvent);
}

/* Callback Function */
eSTMGRReturns rdkStorage_RegisterEventCallback(fnSTMGR_EventCallback eventCallback)
{
    STMGRLOG_DEBUG("ENTRY of %s\n", __FUNCTION__);

    return rSTMgrMainClass::getInstance()->registerEventCallback(eventCallback);
}

