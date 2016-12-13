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


#include <stdio.h>
#include <memory.h>
#include <unistd.h>

#include "storageMgrInternal.h"
#include "storageMgr.h"
#include "libIARMCore.h"
#include "libIBus.h"
#include "udevUtils.h"
#include "sdCardReader.h"
#include "pwrMgr.h"
#ifdef ENABLE_DEEP_SLEEP
#define SM_MOUNT_PATH	"/media/tsb"
#define SM_DISK_CHECK	"/lib/rdk/disk_checkV2"
#else
#define SM_MOUNT_PATH	""
#define SM_DISK_CHECK	""
#endif

pthread_mutex_t hsLock = PTHREAD_MUTEX_INITIALIZER; /* Health status mutex lock*/

static int is_connected = 0;
static uint32_t tsbMaxMinute = 20;
static bool tsbEnable = true;
extern char strMgr_ConfigProp_FilePath[100];
static bool tsbMaxMin_InitFlag = false;
strMgrDeviceInfoParam_t gdeviceInfo; /**@<	Global structure for SD Card	*/
static unsigned short actualTsbMaxMin = 0;

static void storageManager_init();
static bool stMgr_ConfigProperties_Init();
static void _eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
stmMgrConfigProp confProp;

static IARM_Result_t getDeviceIds(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(!isSDCardPresent())
    {
        param->status = false;
        return retCode;
    }

    memcpy(param->data.deviceID.devId, gdeviceInfo.devid, DEV_ID_BUF);
    param->status = true;
    LOG_DEBUG("[%s:%d] Dev Id: %s and CID: %s\n", __FUNCTION__, __LINE__, param->data.deviceID.devId, gdeviceInfo.devid);
    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return retCode;
}


static IARM_Result_t getDeviceInfo(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;
    strMgrDeviceInfoParam_t sdObj;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);


    if(false == isSDCardPresent())
    {
        param->status = false;
        return retCode;
    }

    LOG_INFO("[%s:%d] param->data.deviceInfo.devid : %s \n", __FUNCTION__, __LINE__, param->data.deviceInfo.devid);

    memset(&sdObj, 0, sizeof(sdObj));

    if(get_SDCard_Properties(&sdObj))
    {
        if((strlen(param->data.deviceInfo.devid) == 0 ) || ((strcasecmp(param->data.deviceInfo.devid, gdeviceInfo.devid) == 0)))
        {
            LOG_INFO("[%s:%d] param->data.deviceInfo.devid : %s & gdeviceInfo.devid: %s \n",
                    __FUNCTION__, __LINE__, param->data.deviceInfo.devid, gdeviceInfo.devid);
            memcpy(param->data.deviceInfo.devid, gdeviceInfo.devid, DEV_ID_BUF);
            memcpy(param->data.deviceInfo.mountPartition, (const char *)gdeviceInfo.mountPartition, MAX_BUF);
            memcpy(param->data.deviceInfo.format, gdeviceInfo.format, MAX_BUF);
            memcpy(param->data.deviceInfo.manufacturer, sdObj.manufacturer, MAX_BUF);
            param->data.deviceInfo.type = sdObj.type;
            param->data.deviceInfo.capacity = gdeviceInfo.capacity;
            param->data.deviceInfo.free_space = sdObj.free_space;
            if(isSDCardMounted())
                param->data.deviceInfo.status = (gdeviceInfo.status == SM_DEV_STATUS_OK || gdeviceInfo.status == SM_DEV_STATUS_READY)? SM_DEV_STATUS_OK: gdeviceInfo.status;
            else
                param->data.deviceInfo.status = SM_DEV_STATUS_NOT_PRESENT;
            param->data.deviceInfo.isTSBSupported = sdObj.isTSBSupported;
            param->data.deviceInfo.isDVRSupported = sdObj.isDVRSupported;
            param->status = true;
            retCode=IARM_RESULT_SUCCESS;
        }
        else
        {
            param->status = false;
        }
    }

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;
}

static IARM_Result_t getTSBStatus(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    if(isSDCardPresent() && getSDCard_TSB_Supported())
    {
        smDeviceStatusCode_t tsbStatus = (smDeviceStatusCode_t)gdeviceInfo.status;
        param->data.tsbStatusCode.tsbStatus = (tsbStatus == SM_DEV_STATUS_OK || tsbStatus == SM_DEV_STATUS_READY)? SM_DEV_STATUS_OK: tsbStatus;
        param->status = true;
    }
    else
    {
        param->data.tsbStatusCode.tsbStatus = 0;
        param->status = false;
    }

    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return retCode;
}


static IARM_Result_t getTSBMaxMinutes(void *arg)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    param->data.tsbMaxMin.tsbMaxMin = 0;
    param->status = false;

    if (!isSDCardPresent())
    {
        LOG_ERROR("[%s:%d] SD card is not present\n", __FUNCTION__, __LINE__);
    }
    else if (!getSDCard_TSB_Supported())
    {
        LOG_ERROR("[%s:%d] SD card TSB is not supported\n", __FUNCTION__, __LINE__);
    }
    else if (!isSDCardMounted())
    {
        LOG_ERROR("[%s:%d] SD card is not mounted\n", __FUNCTION__, __LINE__);
    }
    else if (tsbMaxMin_InitFlag)
    {
        LOG_INFO("[%s:%d] Using previously set value for TSBMaxMinutes\n", __FUNCTION__, __LINE__);
        param->data.tsbMaxMin.tsbMaxMin = tsbMaxMinute;
        param->status = true;
    }
    else if (gdeviceInfo.capacity == 0)
    {
        LOG_ERROR("[%s:%d] gdeviceInfo.capacity is 0\n", __FUNCTION__, __LINE__);
    }
    else
    {
        /*Converting from Mbps to Minute*/
        unsigned long long frameRate = ((confProp.frameRate_mbps ? confProp.frameRate_mbps : DEFULT_DATARATE_PER_SEC)*60*1024*1024)/8;

        actualTsbMaxMin = gdeviceInfo.capacity/frameRate;

        if(confProp.tsbMaxMin)
        {
            /* Need to check whether tsbMaxMin is less than that supported by the platform */
            if((actualTsbMaxMin > DEFULT_TSB_MAX_MINUTE) || (actualTsbMaxMin > confProp.tsbMaxMin))
            {
                actualTsbMaxMin = confProp.tsbMaxMin;
                LOG_INFO("[%s:%d] TSBMaxMinutes got from config file\n", __FUNCTION__, __LINE__);
            }
        }

        param->data.tsbMaxMin.tsbMaxMin = actualTsbMaxMin;
        param->status = true;
    }

    LOG_INFO("[%s:%d] TSBMaxMinutes = %d\n", __FUNCTION__, __LINE__, param->data.tsbMaxMin.tsbMaxMin);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return IARM_RESULT_SUCCESS;
}

static IARM_Result_t setTSBMaxMinutes(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(param)
    {
        if(isSDCardPresent() && getSDCard_TSB_Supported())
        {
            if(!actualTsbMaxMin)
            {
                unsigned long long frameRate = 0;
                if(confProp.frameRate_mbps) {
                    frameRate = (confProp.frameRate_mbps*60*1024*1024)/8;
                } else {
                    frameRate = (DEFULT_DATARATE_PER_SEC*60*1024*1024)/8;
                }
                actualTsbMaxMin = gdeviceInfo.capacity/frameRate;
            }

            if(param->data.tsbMaxMin.tsbMaxMin <= actualTsbMaxMin)
            {
                tsbMaxMinute = param->data.tsbMaxMin.tsbMaxMin;
                param->status = true;
                if(!tsbMaxMin_InitFlag)
                    tsbMaxMin_InitFlag = true;
                LOG_INFO("[%s:%d] The setTSBMaxMinutes is set to new value : %d\n", __FUNCTION__, __LINE__, param->data.tsbMaxMin.tsbMaxMin);
            }
            else {
                LOG_INFO("[%s:%d] The setTSBMaxMinutes value (%d) is more than actual TSBMaxMinutes (%d), so set TSBMaxMinutes to actual TSBMaxMinutes (%d).\n",
                       __FUNCTION__, __LINE__, param->data.tsbMaxMin.tsbMaxMin, actualTsbMaxMin, actualTsbMaxMin);
                tsbMaxMinute = actualTsbMaxMin;
                param->status = true;
            }
        }
        else
        {
            param->status = false;
        }
        retCode=IARM_RESULT_SUCCESS;
    }
    else
    {
        LOG_ERROR ("Invalid value to set TSB status.\n");
        param->status = false;
    }

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return retCode;
}


static IARM_Result_t isTSBEnabled(void *arg)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    param->data.tsbEnableData.tsbEnabled = false;
    param->status = false;

    if (!isSDCardPresent())
    {
        LOG_ERROR("[%s:%d] SD card is not present\n", __FUNCTION__, __LINE__);
    }
    else if (!getSDCard_TSB_Supported())
    {
        LOG_ERROR("[%s:%d] SD card TSB is not supported\n", __FUNCTION__, __LINE__);
    }
    else if (!isSDCardMounted())
    {
        LOG_ERROR("[%s:%d] SD card is not mounted\n", __FUNCTION__, __LINE__);
    }
    else
    {
        param->data.tsbEnableData.tsbEnabled = tsbEnable;
        param->status = true;
    }

    LOG_INFO("[%s:%d] isTSBEnabled = %d\n", __FUNCTION__, __LINE__, param->status);

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);

    return IARM_RESULT_SUCCESS;
}


static IARM_Result_t isTSBCapable(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(!isSDCardPresent()) {
        param->data.tsbCapableData.istsbCapable = false;
    }
    else {
//    	if(confProp.isTsbEnabled)
        param->data.tsbCapableData.istsbCapable = (getSDCard_TSB_Supported())?true:false;
    }

    param->status = true;
    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;
}


static IARM_Result_t setTSBEnabled(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);
    if(isSDCardPresent())
    {
        if(getSDCard_TSB_Supported())
        {
            tsbEnable = param->data.tsbEnableData.tsbEnabled;
            param->status = true;
            LOG_INFO("[%s:%d] The TSBEnabled value set to : %s\n", __FUNCTION__, __LINE__, (param->data.tsbEnableData.tsbEnabled)?"TRUE":"FALSE");
        }
        else
        {
            param->status = false;
        }
    }
    else
    {
        param->status = false;
    }

    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;

}

static IARM_Result_t getSDCardStatus(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(!isSDCardPresent()) {
        param->data.sdCardStatusData.isSDCardReady = false;
    }
    else {
        param->data.sdCardStatusData.isSDCardReady = (getsdCardStatusOnReadyEvent())?true:false;
    }

    param->status = true;
    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;
}

static IARM_Result_t getSDcardPropertyInfo(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;
    param->status = true;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if(isSDCardPresent()) {
        LOG_DEBUG("[%s:%d] param->data.stSDCardParams.eSDPropType : [ %d ]\n", __FUNCTION__, __LINE__, param->data.stSDCardParams.eSDPropType);
        switch(param->data.stSDCardParams.eSDPropType)
        {
        case SD_Capacity:
            param->data.stSDCardParams.sdCardProp.ui32Val = (gdeviceInfo.capacity)/(1024*1024);
            break;
        case SD_CardFailed:
            if(gdeviceInfo.status == SM_DEV_STATUS_NOT_QUALIFIED || gdeviceInfo.status == SM_DEV_STATUS_NOT_PRESENT) {
                param->data.stSDCardParams.sdCardProp.bVal = false;
            }
            else
            {
                param->data.stSDCardParams.sdCardProp.bVal = false;
            }
            break;
        case SD_LifeElapsed:
        {
            struct health_status hs;
            memset(&hs, '\0', sizeof(hs));

            param->data.stSDCardParams.sdCardProp.iVal = isCMD56Supported() ? ((stMgr_Check_SDCard_HealthStatus(&hs)) ? hs.used : -1) : -1;
        }
        break;
        case SD_LotID:
            get_SDCard_PropValueFromFile((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)"/hwrev");
            break;
        case SD_Manufacturer:
            get_SDCard_PropValueFromFile((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)"/manfid");
            break;
        case SD_Model:
            get_SDCard_PropValueFromFile((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)"/name");
            break;
        case SD_ReadOnly:
            param->data.stSDCardParams.sdCardProp.bVal = (gdeviceInfo.status == SM_DEV_STATUS_READ_ONLY)?true:false;
            break;
        case SD_SerialNumber:
        {
            char chSerialNum[20] = {'\0'};
            if(get_SDCard_PropValueFromFile((char *)chSerialNum, (const char *)"/serial")) {
                param->data.stSDCardParams.sdCardProp.ui32Val = strtol (chSerialNum, NULL, 16);
            }
        }
        break;
        case SD_TSBQualified:
            param->data.stSDCardParams.sdCardProp.bVal = getSDCard_TSB_Supported();
            break;
        case SD_Status:
        {
            memset(param->data.stSDCardParams.sdCardProp.uchVal, '\0', MAX_BUF);
            eTsbStatus tsbHm = get_SDcardTsbHealthMonStatus();
            if(tsbHm == TSB_SUPPORT_WITH_HEALTH_MONITORING)
            {
                strcpy((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)PRESENT_WITH_HEALTH_MONITORING);
            }
            else if(tsbHm == TSB_SUPPORT_WITHOUT_HEALTH_MONITORING)
            {
                strcpy((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)PRESENT_WITHOUT_HEALTH_MONITORING);
            }
            else if(tsbHm == NO_TSB_SUPPORT)
            {
                strcpy((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char *)INCOMPATIBLE_SDCARD);
            }
        }
        break;
        default:
            param->status = false;
            break;
        }
    }
    else
    {
        switch(param->data.stSDCardParams.eSDPropType)
        {
        case SD_Status:
            memset(param->data.stSDCardParams.sdCardProp.uchVal, '\0', MAX_BUF);
            strcpy((char *)param->data.stSDCardParams.sdCardProp.uchVal, (const char*)"None");
            break;
        default:
            param->status = false;
            break;
        }
    }
    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;
}

static IARM_Result_t getMountPath(void *arg)
{
    IARM_Result_t retCode = IARM_RESULT_IPCCORE_FAIL;
    IARM_Bus_STRMgr_Param_t *param = (IARM_Bus_STRMgr_Param_t *)arg;

    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    if((false == isSDCardPresent()) && (false == isSDCardMounted())) {
        param->status = false;
        retCode=IARM_RESULT_SUCCESS;
        return retCode;
    }

    memset(param->data.mountPath.mountDir, '\0', MOUNT_PATH_BUF);
    if(getSDCard_TSB_Supported())
    {
        strncpy(param->data.mountPath.mountDir, confProp.sdCardMountPath, MOUNT_PATH_BUF);
        param->status = true;
    }
    else
    {
        param->status = false;
    }

    retCode=IARM_RESULT_SUCCESS;

    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
    return retCode;
}

IARM_Result_t storageManager_Start(void)
{
    IARM_Result_t err = IARM_RESULT_IPCCORE_FAIL;

    pthread_t stmgrMonitortid;
    pthread_attr_t attr; // thread attribute
    int iret = 0;

    setvbuf(stdout, NULL, _IOLBF, 0);
    storageManager_init();

    // set thread detachstate attribute to DETACHED
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    err = IARM_Bus_Init(IARM_BUS_ST_MGR_NAME);

    if(IARM_RESULT_SUCCESS != err)
    {
        LOG_ERROR ("Error initializing IARM.. error code : %d\n",err);
        return err;
    }

    err = IARM_Bus_Connect();

    if(IARM_RESULT_SUCCESS != err)
    {
        LOG_ERROR ("Error connecting to IARM.. error code : %d\n",err);
        return err;
    }

    is_connected = 1;

    /*Get/Set RPC: Register all the get/set api's */
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetDeviceIds, getDeviceIds);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetDeviceInfo, getDeviceInfo);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetTSBStatus, getTSBStatus);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetTSBMaxMinutes, getTSBMaxMinutes);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_SetTSBMaxMinutes, setTSBMaxMinutes);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_IsTSBEnabled, isTSBEnabled);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_SetTSBEnabled, setTSBEnabled);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_IsTSBCapable, isTSBCapable);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetSDcardStatus, getSDCardStatus);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetSDcardPropertyInfo, getSDcardPropertyInfo);
    err = IARM_Bus_RegisterCall(IARM_BUS_STORAGE_MGR_API_GetMountPath, getMountPath);

    if (IARM_RESULT_SUCCESS != (IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED,_eventHandler)))
        LOG_ERROR("Failed to IARM_Bus_RegisterEventHandler for %s\n", IARM_BUS_PWRMGR_NAME);
    else
        LOG_INFO("Successfully IARM_Bus_RegisterEventHandler for %s\n", IARM_BUS_PWRMGR_NAME);
    /* Notification RPC:*/
    IARM_Bus_RegisterEvent(IARM_BUS_STORAGE_MGR_EVENT_MAX);

    if(IARM_RESULT_SUCCESS != err)
    {
        if(is_connected)
        {
            IARM_Bus_Disconnect();
        }
        IARM_Bus_Term();
    }

    // create the thread
    iret = pthread_create(&stmgrMonitortid, &attr, stMgrMonitorThead, NULL);

    if(iret)
    {
        fprintf (stderr, "Error - pthread_create() return code: %d\n", iret);
        LOG_ERROR ("Error - pthread_create() return code: %d\n", iret);
#ifdef ENABLE_SD_NOTIFY
        sd_notifyf(0, "READY=1\n"
                   "STATUS=storageMgrMain is ready, but failed to start thread for monitoring udev events.\n" "MAINPID=%lu",  (unsigned long) getpid());
#endif
    }

    return err;
}

IARM_Result_t storageManager_Stop(void)
{
    if(is_connected)
    {
        IARM_Bus_Disconnect();
        IARM_Bus_Term();
    }
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t storageManager_Loop()
{
    time_t curr = 0;
    while(1)
    {
        time(&curr);
        LOG_INFO ("I-ARM STORAGE Mgr: HeartBeat at %s\n", ctime(&curr));
        sleep(300);
    }
    return IARM_RESULT_SUCCESS;
}

static void storageManager_init()
{
    stMgr_ConfigProperties_Init();
}


static bool stMgr_ConfigProperties_Init()
{
    bool status = false;
    GKeyFile *key_file = NULL;
    GError *error = NULL;
    gsize length = 0;
    gdouble double_value = 0;
    guint group = 0, key = 0;

    if(strMgr_ConfigProp_FilePath[0] == '\0')
    {
        LOG_ERROR("[%s:%d] Failed to read Storage Manager Configuration file \n", __FUNCTION__, __LINE__);
        return false;
    }

    key_file = g_key_file_new();

    if(!key_file) {
        LOG_ERROR("[%s:%d] Failed to g_key_file_new() \n", __FUNCTION__, __LINE__);
        return false;
    }

    if(!g_key_file_load_from_file(key_file, strMgr_ConfigProp_FilePath, G_KEY_FILE_KEEP_COMMENTS, &error))
    {
        LOG_ERROR("%s", error->message);
        return false;
    }
    else
    {
        strcpy(confProp.sdCardDevNode, "");
        strcpy(confProp.sdCardSrcDevNode, "");

        gsize groups_id, num_keys;
        gchar **groups = NULL, **keys = NULL, *value = NULL;  /*CID-19243*/
        groups = g_key_file_get_groups(key_file, &groups_id);
        for(group = 0; group < groups_id; group++)
        {
            LOG_INFO("group %u/%u: \t%s\n", group, groups_id - 1, groups[group]);
            if(0 == strncasecmp(SDCARD_CONFIG, groups[group], strlen(groups[group])))
            {
                keys = g_key_file_get_keys(key_file, groups[group], &num_keys, &error);
                for(key = 0; key < num_keys; key++)
                {
                    value = g_key_file_get_value(key_file,	groups[group],	keys[key],	&error);
                    LOG_INFO("\t\tkey %u/%u: \t%s => %s\n", key, num_keys - 1, keys[key], value);
                    if(0 == strncasecmp(FC_MMC_DEV, keys[key], strlen(keys[key])))
                    {
                        memcpy(confProp.sdCardDevNode, value, strlen(value));
                    }
                    if(0 == strncasecmp(FC_MMC_SRC_DEV, keys[key], strlen(keys[key])))
                    {
                        memcpy(confProp.sdCardSrcDevNode, value, strlen(value));
                    }
                    if(0 == strncasecmp(FC_MOUNT_PATH, keys[key], strlen(keys[key])))
                    {
                        if(value) {
                            strncpy(confProp.sdCardMountPath, ((0 == strlen(SM_MOUNT_PATH)) ? value : SM_MOUNT_PATH), MAX_BUF);
                        }
                    }
                    if(0 == strncasecmp(FC_FRAME_RATE_MBPS, keys[key], strlen(keys[key])))
                    {
                        confProp.frameRate_mbps = atoi(value);
                    }
                    if(0 == strncasecmp(FC_IsTSBEnableOverride, keys[key], strlen(keys[key])))
                    {
                        if(0 == strcasecmp(value, "true")) {
                            confProp.isTsbEnableOverride = true;
                            tsbEnable = confProp.isTsbEnableOverride;
                        }
                        else
                        {
                            confProp.isTsbEnableOverride = false;
                            tsbEnable = confProp.isTsbEnableOverride;
                        }
                    }
                    if(0 == strncasecmp(FILE_SYSTEM_TYPE, keys[key], strlen(keys[key])))
                    {
                        memcpy(confProp.fileSysType, value, strlen(value));
                    }
                    if(0 == strncasecmp(FC_DEFAULT_TSB_MAX_MINUTE, keys[key], strlen(keys[key])))
                    {
                        if(value)
                            confProp.tsbMaxMin  = atoi(value);
                    }

                    /* Path of Disck Check script */
                    if (0 == strncasecmp(DISK_CHECK_SCRIPT, keys[key], strlen(keys[key])))
                    {
                        if(value) {
                            strncpy(confProp.disk_check_script, ((0 == strlen(SM_DISK_CHECK))? value : SM_DISK_CHECK), MAX_BUF);
                        }
                    }
                    /* TSB validate based on the flag */
                    if (0 == strncasecmp(TSB_VALIDATION_FLAG, keys[key], strlen(keys[key])))
                    {
                        if(0 == strcasecmp(value, "false")) {
                            confProp.tsbValidationFlag = false;
                        }
                        else
                        {
                            confProp.tsbValidationFlag = true;
                        }
                    }

                    /* SD Card Device name*/
                    if (0 == strncasecmp(DEV_NAME, keys[key], strlen(keys[key])))
                    {
                        if(value) {
                            memcpy(confProp.devFile, value, strlen(value));
                        }
                    }
                    g_free(value);
                }
                if((confProp.sdCardDevNode[0]==0) || (confProp.sdCardSrcDevNode[0]==0))
                {
                    memcpy(confProp.sdCardDevNode, MMC_DEV, strlen(MMC_DEV));
                    memcpy(confProp.sdCardSrcDevNode, MMC_SRC_DEV, strlen(MMC_SRC_DEV));
                }
            }
            g_strfreev(keys);
        }
        g_strfreev(groups);
    }
    g_key_file_free(key_file);

    return true;
}

static void _eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    LOG_TRACE("[%s:%d] Entering..\n", __FUNCTION__, __LINE__);

    IARM_Bus_STRMgr_EventData_t eventData;
    if (strcasecmp(owner,IARM_BUS_PWRMGR_NAME) == 0)
    {
        LOG_INFO("[%s:%d] Received eventId --> %d\n",  __FUNCTION__, __LINE__, eventId);
        switch (eventId)
        {
            LOG_INFO("[%s:%d] Received eventId --> %d\n",  __FUNCTION__, __LINE__, eventId);
        case IARM_BUS_PWRMGR_EVENT_MODECHANGED:
        {
            IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;

            LOG_INFO("[%s:%d] StorageManager: IARM_BUS_PWRMGR_EVENT_MODECHANGED : State Changed %d -- > %d\n",
                   __FUNCTION__, __LINE__, param->data.state.curState, param->data.state.newState);

            switch( param->data.state.newState)
            {
            case IARM_BUS_PWRMGR_POWERSTATE_OFF:
            case IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP:
                LOG_INFO("[%s:%d] Received PWRMGR_EVENT_MODECHANGED, change Power State to \"%s\" .\n", __FUNCTION__, __LINE__,
                       (param->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP)
                       ? "PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP" : "PWRMGR_POWERSTATE_OFF");

                /*Trigger SD Card umount event*/
                memset(&eventData, 0, sizeof(eventData));
                memcpy(eventData.data.cardStatus.deviceId, gdeviceInfo.devid, strlen(gdeviceInfo.devid));
                eventData.data.cardStatus.status = SM_DEV_STATUS_UMOUNT;
                strcpy(eventData.data.cardStatus.mountDir, confProp.sdCardMountPath);

                IARM_Bus_BroadcastEvent(IARM_BUS_ST_MGR_NAME, (IARM_EventId_t) IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged, (void *)&eventData, sizeof(eventData));

                LOG_INFO("[%s:%d] \"%s\" Receive DEEP SLEEP event, ready to umount, sends \"OnDeviceStatusChanged\" IARM_Bus_BroadcastEvent with values as \"%s:%d\".\n",
                       __FUNCTION__, __LINE__, IARM_BUS_ST_MGR_NAME, eventData.data.cardStatus.deviceId, eventData.data.cardStatus.status);
                sleep(2);
#ifdef USE_DISK_CHECK
                /* do umount operations */
                execute_Umount_Script();
#endif

                break;
            case IARM_BUS_PWRMGR_POWERSTATE_ON:
                /* do  mount operations */
                LOG_INFO("[%s:%d] Power State On %d\n " ,__FUNCTION__, __LINE__, param->data.state.newState);

#ifdef USE_DISK_CHECK
                execute_Mount_Script();
#endif
                break;

            }
            break;
        }
        default:
            break;
        }
    }
    LOG_TRACE("[%s:%d] Exiting..\n", __FUNCTION__, __LINE__);
}

/** @} */
/** @} */
