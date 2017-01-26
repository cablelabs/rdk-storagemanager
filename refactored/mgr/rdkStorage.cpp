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
    struct udev_monitor *pDevMonitor = NULL;
    pDevMonitor = udev_monitor_new_from_netlink(g_uDevInstance, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(pDevMonitor, "mmc", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(pDevMonitor, "ubi", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(pDevMonitor, "scsi", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(pDevMonitor, "block", NULL);
    udev_monitor_enable_receiving(pDevMonitor);

    while (1)
    {
        ;
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

#ifdef ENABLE_HDD
    /* Check the presense of HDD; Create n Add it to the map */
    ...
    ...
    rSTMgrMainClass::getInstance()->addNewMemoryDevice(key, RDK_STMGR_DEVICE_TYPE_HDD);
#endif /* ENABLE_HDD */

#ifdef ENABLE_SDCARD
    /* Check the presense of SDCard; Create n Add it to the map */
    ...
    ...
    rSTMgrMainClass::getInstance()->addNewMemoryDevice(key, RDK_STMGR_DEVICE_TYPE_HDD);
#endif /* ENABLE_SDCARD */

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
    return RDK_STMGR_RETURN_SUCCESS;
}

