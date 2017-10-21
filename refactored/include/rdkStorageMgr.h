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
#ifndef __RDK_STORAGE_MGR_H__
#define __RDK_STORAGE_MGR_H__

#include "rdkStorageMgrTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Init */
void rdkStorage_init (void);

/* Get DeviceIDs*/
eSTMGRReturns rdkStorage_getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs);

/* Get DeviceInfo */
eSTMGRReturns rdkStorage_getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo);

/* Get DeviceInfoList */
eSTMGRReturns rdkStorage_getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList);

/* Get PartitionInfo */
eSTMGRReturns rdkStorage_getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo);

/* Get TSBStatus */
eSTMGRReturns rdkStorage_getTSBStatus (eSTMGRTSBStatus *pTSBStatus);

/* Set TSBMaxMinutes */
eSTMGRReturns rdkStorage_setTSBMaxMinutes (unsigned int minutes);

/* Get TSBMaxMinutes */
eSTMGRReturns rdkStorage_getTSBMaxMinutes (unsigned int *pMinutes);

/* Get TSBCapacityMinutes */
eSTMGRReturns rdkStorage_getTSBCapacityMinutes(unsigned int *pMinutes);

/* Get TSBCapacity*/
eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long long *pCapacity);

/* Get TSBFreeSpace*/
eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long long *pFreeSpace);

/* Get DVRCapacity */
eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long long *pCapacity);

/* Get DVRFreeSpace*/
eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long long *pFreeSpace);

/* Get isTSBEnabled*/
bool rdkStorage_isTSBEnabled();

/* Set TSBEnabled */
eSTMGRReturns rdkStorage_setTSBEnabled (bool isEnabled);

/* Get isDVREnabled*/
bool rdkStorage_isDVREnabled();

/* Set DVREnabled */
eSTMGRReturns rdkStorage_setDVREnabled (bool isEnabled);

/* Get Health */
eSTMGRReturns rdkStorage_getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo);

/* Callback Function */
eSTMGRReturns rdkStorage_RegisterEventCallback(fnSTMGR_EventCallback eventCallback);

/* Get TSB Partition Mount Path */
eSTMGRReturns rdkStorage_getTSBPartitionMountPath (char* pMountPath);

void rdkStorage_notifyMGRAboutFailure (eSTMGRErrorEvent failEvent);
#ifdef __cplusplus
}
#endif

#endif /* __RDK_STORAGE_MGR_H__ */
