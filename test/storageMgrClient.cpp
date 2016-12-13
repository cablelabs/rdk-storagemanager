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
* @defgroup test
* @{
**/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "libIARMCore.h"
#include "storageMgr.h"

static void getDeviceIds();
static void getDeviceInfo();
static void getDeviceInfo(char*);
static void isTSBEnabled();
static void getTSBStatus();
static void setTSBEnabled();
static void getTSBMaxMinutes();
static void setTSBMaxMinutes();
static void IsTSBCapable();
static void getSDCardReadyStatus();
static void getMountPath();

static void _evtHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    unsigned long messageLength=0;

    if (strcmp(owner, IARM_BUS_ST_MGR_NAME)  == 0)
    {

        switch(eventId)
        {
        case IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged:
        {
//            IARM_Bus_STORAGE_Mgr_EventId_t *eventData = (IARM_Bus_STORAGE_Mgr_EventId_t*)data;
            //messageLength = eventData->data.xupnpData.deviceInfoLength;
            printf(" >>>>>>>>>>  EVENT_ADD_DATA_UPDATE. \n");
            break;
        }
        case IARM_BUS_STORAGE_MGR_EVENT_TSB_ERROR:
        {
//            IARM_Bus_STORAGE_Mgr_EventId_t *eventData = (IARM_Bus_STORAGE_Mgr_EventId_t*)data;
            //messageLength = eventData->data.xupnpData.deviceInfoLength;
            printf(" >>>>>>>>>>  EVENT_TSB_ERROR_DATA_UPDATE.\n" );
            break;
        }

        default:
            break;
        }
    }
}

int main()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    //    IARM_Bus_STORAGE_Mgr_EventId_t eventData;
    //    device::StorageManager::getInstance()->initialize();

    printf("Client Entering %d\n", getpid());
    IARM_Bus_Init("StoargeMgrClientTest");
    IARM_Bus_Connect();

    /*Event Handling */
    //    IARM_Bus_RegisterEventHandler(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged, _evtHandler);
    //    IARM_Bus_RegisterEventHandler(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_EVENT_TSB_ERROR, _evtHandler);


    //    while(1) {
    //        sleep(30);
    //        sleep(30);
    //    }

    /*Test for getDeviceIds: from storagemanager */
    /* Get/Set RPC handler*/
    std::cout << "==========================================\n";
    getDeviceIds();
    std::cout << "==========================================\n";
    getDeviceInfo();
    std::cout << "==========================================\n";
    isTSBEnabled();
    std::cout << "==========================================\n";
    getTSBStatus();
    std::cout << "==========================================\n";
    getSDCardReadyStatus();
    std::cout << "==========================================\n";
//    setTSBEnabled();
//    std::cout << "==========================================\n";
//    getTSBStatus();
//    std::cout << "==========================================\n";
    getTSBMaxMinutes();
    std::cout << "==========================================\n";
//    setTSBMaxMinutes();
//    std::cout << "==========================================\n";
//    getTSBMaxMinutes();
//    std::cout << "==========================================\n";
    IsTSBCapable();
    std::cout << "==========================================\n";
    getMountPath();
    std::cout << "==========================================\n";

    IARM_Bus_Disconnect();
    IARM_Bus_Term();
    printf("Storage Manager Client Exiting\n");

}


static void getDeviceIds()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetDeviceIds, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("[%s] : (%s) \n",IARM_BUS_STORAGE_MGR_API_GetDeviceIds, param.data.deviceID.devId);
    }
}


static void getDeviceInfo()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param, param1;
    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetDeviceInfo, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && (param.status))
    {
        if(param.status == true)
        {
            printf("[%s]: \n", IARM_BUS_STORAGE_MGR_API_GetDeviceInfo);
            std::cout << "==========================================\n";
            std::cout << "Device Id\t		: " << param.data.deviceInfo.devid << "\n";
            std::cout << "Type\t			: " << param.data.deviceInfo.type << "\n";;
            std::cout << "Capacity\t		: " << param.data.deviceInfo.capacity << "\n";
            std::cout << "free_space\t 		: " << param.data.deviceInfo.free_space << "\n";
            std::cout << "status\t 			: " << param.data.deviceInfo.status << "\n";
            std::cout << "MountPartition\t 	: " << param.data.deviceInfo.mountPartition << "\n";
            std::cout << "Format\t	 		: " << param.data.deviceInfo.format << "\n";
            std::cout << "Manufacturer\t	: " << param.data.deviceInfo.manufacturer << "\n";
            std::cout << "isTSBSupported\t	: " << param.data.deviceInfo.isTSBSupported << "\n";
            std::cout << "isDVRSupported\t	: " << param.data.deviceInfo.isDVRSupported << "\n";
            std::cout << "==========================================\n";
        }
    }

    memset(&param1, 0, sizeof(param1));
    strcpy(param1.data.deviceInfo.devid, param.data.deviceInfo.devid);
    printf("DeviceId (%s)\n", param.data.deviceInfo.devid);

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetDeviceInfo, (void *)&param1, sizeof(param1));
    if(param.status == true)
    {
        printf("[%s(%s)]: \n", IARM_BUS_STORAGE_MGR_API_GetDeviceInfo, param1.data.deviceInfo.devid);
        std::cout << "==========================================\n";
        std::cout << "Device Id\t		: " << param1.data.deviceInfo.devid << "\n";
        std::cout << "Type\t			: " << param1.data.deviceInfo.type << "\n";;
        std::cout << "Capacity\t		: " << param1.data.deviceInfo.capacity << "\n";
        std::cout << "free_space\t 		: " << param1.data.deviceInfo.free_space << "\n";
        std::cout << "status\t 			: " << param1.data.deviceInfo.status << "\n";
        std::cout << "MountPartition\t 	: " << param1.data.deviceInfo.mountPartition << "\n";
        std::cout << "Format\t	 		: " << param1.data.deviceInfo.format << "\n";
        std::cout << "Manufacturer\t	: " << param1.data.deviceInfo.manufacturer << "\n";
        std::cout << "isTSBSupported\t	: " << param1.data.deviceInfo.isTSBSupported << "\n";
        std::cout << "isDVRSupported\t	: " << param1.data.deviceInfo.isDVRSupported << "\n";
        std::cout << "==========================================\n";
    }
}

static void getDeviceInfo(char*)
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;
    memset(&param, 0, sizeof(param));

//	memcpy(devInfo.data.deviceInfo.devid, devInfoParam.data.deviceInfo.devid, DEV_ID_BUF);

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetDeviceInfo, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && (param.status))
    {
        std::cout << "==========================================\n";
        std::cout << "Device Id\t		: " << param.data.deviceInfo.devid << "\n";
        std::cout << "Type\t			: " << param.data.deviceInfo.type << "\n";;
        std::cout << "Capacity\t 		: " << param.data.deviceInfo.capacity << "\n";
        std::cout << "free_space\t 		: " << param.data.deviceInfo.free_space << "\n";
        std::cout << "status\t 			: " << param.data.deviceInfo.status << "\n";
        std::cout << "MountPartition\t 	: " << param.data.deviceInfo.mountPartition << "\n";
        std::cout << "Format\t	 		: " << param.data.deviceInfo.format << "\n";
        std::cout << "Manufacturer\t	: " << param.data.deviceInfo.manufacturer << "\n";
        std::cout << "isTSBSupported\t	: " << param.data.deviceInfo.isTSBSupported << "\n";
        std::cout << "isDVRSupported\t	: " << param.data.deviceInfo.isDVRSupported << "\n";
        std::cout << "==========================================\n";
    }
}

static void isTSBEnabled()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_IsTSBEnabled, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The value for isTSBEnabled : (%d) \n", param.data.tsbEnableData.tsbEnabled);
    }
}

static void getTSBStatus()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetTSBStatus, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value : (%d).\n", IARM_BUS_STORAGE_MGR_API_GetTSBStatus, param.data.tsbStatusCode.tsbStatus);
    }
}

static void setTSBEnabled()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    param.data.tsbEnableData.tsbEnabled = true;

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_SetTSBEnabled, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\': value : (%d) \n", IARM_BUS_STORAGE_MGR_API_SetTSBEnabled, param.data.tsbEnableData.tsbEnabled);
    }
}

static void getTSBMaxMinutes()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetTSBMaxMinutes, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value: (%d) \n", IARM_BUS_STORAGE_MGR_API_GetTSBMaxMinutes, param.data.tsbMaxMin.tsbMaxMin);
    }
}

static void setTSBMaxMinutes()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));
    param.data.tsbMaxMin.tsbMaxMin = 10;

    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_SetTSBMaxMinutes, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value: (%d) \n", IARM_BUS_STORAGE_MGR_API_SetTSBMaxMinutes, param.data.tsbMaxMin.tsbMaxMin);
    }
}

static void IsTSBCapable()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));
    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_IsTSBCapable, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value: (%d) \n", IARM_BUS_STORAGE_MGR_API_IsTSBCapable, param.data.tsbCapableData.istsbCapable);
    }
}
static void getSDCardReadyStatus()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));
    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetSDcardStatus, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value: (%d) \n", IARM_BUS_STORAGE_MGR_API_GetSDcardStatus, param.data.sdCardStatusData.isSDCardReady);
    }
}

void getMountPath()
{
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_STRMgr_Param_t param;

    memset(&param, 0, sizeof(param));
    retVal = IARM_Bus_Call(IARM_BUS_ST_MGR_NAME, IARM_BUS_STORAGE_MGR_API_GetMountPath, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        printf("The \'%s\', value: (%s) \n", IARM_BUS_STORAGE_MGR_API_GetMountPath, param.data.mountPath.mountDir);
    }
}

/** @} */
/** @} */
