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
    eSTMGRReturns getTSBCapacity(unsigned long *pCapacity);
    eSTMGRReturns getTSBFreeSpace(unsigned long *pFreeSpace);
    eSTMGRReturns getDVRCapacity(unsigned long *pCapacity);
    eSTMGRReturns getDVRFreeSpace(unsigned long *pFreeSpace);
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
