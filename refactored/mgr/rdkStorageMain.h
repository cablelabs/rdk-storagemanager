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
#ifndef __RDK_STMGR_MAIN_H__
#define __RDK_STMGR_MAIN_H__
#include "rdkStorageBase.h"

class rSTMgrMainClass
{
    pthread_t stmgrHealthMonitorTID;
    pthread_mutex_t m_mainMutex;
    fnSTMGR_EventCallback m_eventCallback;

    /* Table of entries */
    std::map <std::string, rStorageMedia*> m_storageDeviceObjects;
    rSTMgrMainClass();
    void enquireHealthInfo(void);
    void notifyEvent(eSTMGREventMessage);

    /* Static */
    static rSTMgrMainClass* g_instance;

public:
    eSTMGRReturns addNewMemoryDevice(std::string devicePath, eSTMGRDeviceType type);
    eSTMGRReturns deleteMemoryDevice(std::string key);
    rStorageMedia* getMemoryDevice(std::string key);
    void devHealthMonThreadEntryFunc (void);
    static rSTMgrMainClass* getInstance();

    /* Public Interfaces */
    eSTMGRReturns getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs);
    eSTMGRReturns getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo );
    eSTMGRReturns getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList);
    eSTMGRReturns getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo);
    eSTMGRReturns getTSBStatus (eSTMGRTSBStatus *pTSBStatus);
    eSTMGRReturns setTSBMaxMinutes (unsigned int minutes);
    eSTMGRReturns getTSBMaxMinutes (unsigned int *pMinutes);
    eSTMGRReturns getTSBCapacityMinutes(unsigned int *pMinutes);
    eSTMGRReturns getTSBCapacity(unsigned long long *pCapacity);
    eSTMGRReturns getTSBFreeSpace(unsigned long long *pFreeSpace);
    eSTMGRReturns getDVRCapacity(unsigned long long *pCapacity);
    eSTMGRReturns getDVRFreeSpace(unsigned long long *pFreeSpace);
    bool isTSBEnabled(void);
    eSTMGRReturns setTSBEnabled (bool isEnabled);
    bool isDVREnabled(void);
    eSTMGRReturns setDVREnabled (bool isEnabled);
    eSTMGRReturns getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo);
    eSTMGRReturns getTSBPartitionMountPath (char* pMountPath);
    eSTMGRReturns notifyMGRAboutFailure (eSTMGRErrorEvent failEvent);
    eSTMGRReturns registerEventCallback(fnSTMGR_EventCallback eventCallback);
};


#endif /* __RDK_STMGR_MAIN_H__ */
