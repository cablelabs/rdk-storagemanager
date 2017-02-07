#include "rdkStorageMgr.h"
#include "rdkStorageMgrTypes.h"
#include "rdkStorageMgrLogger.h"

/* For Udev Access */
#include <libudev.h>

/* For file access */
#include <sys/types.h>
#include <unistd.h>

/* Storage Main Class */
#include "rdkStorageMain.h"



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
    udev_monitor_filter_add_match_subsystem_devtype(pUDevMonitor, "ubi", NULL);
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
            std::string deviceType = udev_device_get_devtype(pDevice);
            std::string devicePath = udev_device_get_devnode(pDevice);
            eSTMGRReturns retCode = RDK_STMGR_RETURN_INVALID_INPUT;

            /* Get the device Type and see whether it is DISK device */
            if ((!deviceType.empty()) && (strcmp (deviceType.c_str(), "disk") == 0))
            {
                if(0 == strcasecmp(pUDevAction, "add"))
                {
                    /* Add the new device */
                    /* FIXME: Get the device Class and Decide on the device type; for now HDD */
                    retCode = rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_HDD);
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

#ifdef ENABLE_NVRAM
    /* Check the presense of NVRAM; Create n Add it to the map */
    ...
    ...
    rSTMgrMainClass::getInstance()->addNewMemoryDevice(key, RDK_STMGR_DEVICE_TYPE_NVRAM);
#endif /* ENABLE_NVRAM */

    {
        /* Check the presense of HDD; Create n Add it to the map */
        struct udev_device *pDevice = NULL;
        struct udev_device *pParentDevice = NULL;
        struct udev_enumerate *pEnumerate = NULL;
        struct udev_list_entry *pDeviceList = NULL;
        struct udev_list_entry *pDeviceListEntry = NULL;

        pEnumerate = udev_enumerate_new(g_uDevInstance);
        udev_enumerate_add_match_subsystem(pEnumerate, "block");
        udev_enumerate_scan_devices(pEnumerate);

        pDeviceList = udev_enumerate_get_list_entry(pEnumerate);
        udev_list_entry_foreach(pDeviceListEntry, pDeviceList)
        {
            const char* pSysPath = NULL;
            const char* pDevType = NULL;
            const char* pDevBus = NULL;
            pSysPath = udev_list_entry_get_name(pDeviceListEntry);
            pDevice = udev_device_new_from_syspath(g_uDevInstance, pSysPath);
            pDevType = udev_device_get_devtype(pDevice);
            if ((pDevType) && (strcasecmp (pDevType, "disk") == 0))
            {
                std::string devicePath = udev_device_get_devnode(pDevice);
                STMGRLOG_INFO ("Sys    Path : %s\n", udev_device_get_syspath(pDevice));
                STMGRLOG_INFO ("Device Path : %s\n", udev_device_get_devpath(pDevice));
                STMGRLOG_INFO ("Device Type : %s\n", udev_device_get_devtype(pDevice));
                STMGRLOG_INFO ("Device Node : %s\n", devicePath.c_str());

                //Check the presense of SDCard; Create n Add it to the map

                /* This ensures that the disk that is identified is MMC subsystem */
                pParentDevice = udev_device_get_parent_with_subsystem_devtype(pDevice, "mmc", NULL);
                if (pParentDevice)
                {
                    STMGRLOG_INFO ("IS MMC TYPE : Yes\n");
                    rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_SDCARD);
                }
                else
                {
                    /* This check is to avoid RAM & other devices */
                    pParentDevice = udev_device_get_parent (pDevice);
                    if (pParentDevice)
                    {
                        pDevBus = udev_device_get_property_value(pDevice, "ID_BUS");
                        STMGRLOG_INFO ("ID_BUS      : %s\n", pDevBus);

                        if ((pDevBus) && (strcasecmp(pDevBus, "scsi")))
                            rSTMgrMainClass::getInstance()->addNewMemoryDevice(devicePath, RDK_STMGR_DEVICE_TYPE_HDD);
                        else if ((pDevBus) && (strcasecmp(pDevBus, "usb")))
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
    }


    /* Start the Polling thread which does the health Monitoring */
    pthread_create(&stmgrDeviceMonitorTID, NULL, deviceConnectRemoveMonitorfn, (void*) g_uDevInstance);

    return;
}

/* Get DeviceIDs*/
eSTMGRReturns rdkStorage_getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs)
{
    return rSTMgrMainClass::getInstance()->getDeviceIds(pDeviceIDs);
}

/* Get DeviceInfo */
eSTMGRReturns rdkStorage_getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo)
{
    return rSTMgrMainClass::getInstance()->getDeviceInfo(pDeviceID, pDeviceInfo);
}

/* Get DeviceInfoList */
eSTMGRReturns rdkStorage_getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList)
{
    return rSTMgrMainClass::getInstance()->getDeviceInfoList(pDeviceInfoList);
}

/* Get PartitionInfo */
eSTMGRReturns rdkStorage_getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    return rSTMgrMainClass::getInstance()->getPartitionInfo (pDeviceID, pPartitionId, pPartitionInfo);
}

/* Get TSBStatus */
eSTMGRReturns rdkStorage_getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    return rSTMgrMainClass::getInstance()->getTSBStatus (pTSBStatus);
}

/* Set TSBMaxMinutes */
eSTMGRReturns rdkStorage_setTSBMaxMinutes (unsigned int minutes)
{
    return rSTMgrMainClass::getInstance()->setTSBMaxMinutes (minutes);
}

/* Get TSBMaxMinutes */
eSTMGRReturns rdkStorage_getTSBMaxMinutes (unsigned int *pMinutes)
{
    return rSTMgrMainClass::getInstance()->getTSBMaxMinutes (pMinutes);
}

/* Get TSBCapacityMinutes */
eSTMGRReturns rdkStorage_getTSBCapacityMinutes(unsigned int *pMinutes)
{
    return rSTMgrMainClass::getInstance()->getTSBCapacityMinutes(pMinutes);
}

/* Get TSBCapacity*/
eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long *pCapacityInKB)
{
    return rSTMgrMainClass::getInstance()->getTSBCapacity(pCapacityInKB);
}

/* Get TSBFreeSpace*/
eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long *pFreeSpaceInKB)
{
    return rSTMgrMainClass::getInstance()->getTSBFreeSpace(pFreeSpaceInKB);
}

/* Get DVRCapacity */
eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long *pCapacityInKB)
{
    return rSTMgrMainClass::getInstance()->getDVRCapacity(pCapacityInKB);
}

/* Get DVRFreeSpace*/
eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long *pFreeSpaceInKB)
{
    return rSTMgrMainClass::getInstance()->getDVRFreeSpace(pFreeSpaceInKB);
}

/* Get isTSBEnabled*/
bool rdkStorage_isTSBEnabled()
{
    return rSTMgrMainClass::getInstance()->isTSBEnabled();
}

/* Set TSBEnabled */
eSTMGRReturns rdkStorage_setTSBEnabled (bool isEnabled)
{
    return rSTMgrMainClass::getInstance()->setTSBEnabled(isEnabled);
}

/* Get isDVREnabled*/
bool rdkStorage_isDVREnabled()
{
    return rSTMgrMainClass::getInstance()->isDVREnabled();
}

/* Set DVREnabled */
eSTMGRReturns rdkStorage_setDVREnabled (bool isEnabled)
{
    return rSTMgrMainClass::getInstance()->setDVREnabled(isEnabled);
}

/* Get Health */
eSTMGRReturns rdkStorage_getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo)
{
    return rSTMgrMainClass::getInstance()->getHealth (pDeviceID, pHealthInfo);
}

/* Callback Function */
eSTMGRReturns rdkStorage_RegisterEventCallback(fnSTMGR_EventCallback eventCallback)
{
    return rSTMgrMainClass::getInstance()->registerEventCallback(eventCallback);
}

