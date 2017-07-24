#ifndef __RDK_STORAGE_MGR_BASE_H__
#define __RDK_STORAGE_MGR_BASE_H__

#include "rdkStorageMgrTypes.h"
#include "rdkStorageMgrLogger.h"
#include <string>
#include <map>

using namespace std;

class rStoragePartition {
public:
//    rStoragePartition();
    char m_partitionId [RDK_STMGR_MAX_STRING_LENGTH];
    char m_name [RDK_STMGR_MAX_STRING_LENGTH];
    char m_mountPath [RDK_STMGR_MAX_STRING_LENGTH];
    char m_format[RDK_STMGR_MAX_STRING_LENGTH];
    unsigned long long m_capacity;
    unsigned long long m_freeSpace;
    eSTMGRDeviceStatus m_status;
    bool m_isTSBSupported;
    bool m_isDVRSupported;
};


class rStorageMedia {

protected:
    std::string m_devicePath;
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];

    eSTMGRDeviceType m_type;
    unsigned long long m_capacity;
    eSTMGRDeviceStatus m_status;
    char m_manufacturer[RDK_STMGR_MAX_STRING_LENGTH];
    char m_model[RDK_STMGR_MAX_STRING_LENGTH];
    char m_serialNumber[RDK_STMGR_MAX_STRING_LENGTH];
    char m_firmwareVersion[RDK_STMGR_MAX_STRING_LENGTH];
    char m_hwVersion[RDK_STMGR_MAX_STRING_LENGTH];    /* LotID */
    char m_ifATAstandard[RDK_STMGR_MAX_STRING_LENGTH];
    bool m_hasSMARTSupport;
    eSTMGRHealthInfo m_healthInfo;
    eSTMGRTSBStatus m_tsbStatus;
    std::map <std::string, rStoragePartition*> m_partitionInfo;

    unsigned int m_maxTSBCapacityinMinutes;
    unsigned int m_maxTSBLengthConfigured;

    unsigned long long m_maxTSBCapacity;
    unsigned long long m_freeTSBSpaceLeft;
    unsigned long long m_maxDVRCapacity;
    unsigned long long m_freeDVRSpaceLeft;
    bool m_isTSBEnabled; /* This is to track whether this memory device is supporting it our not; ie one of the partition could be used for TSB.*/
    bool m_isDVREnabled; /* This is to track whether this memory device is supporting it our not; ie one of the partition could be used for DVR.*/
    bool m_isTSBSupported;
    bool m_isDVRSupported;
    fnSTMGR_EventCallback m_eventCallback;

public:
    rStorageMedia();
    virtual ~rStorageMedia();
    virtual eSTMGRDeviceType getDeviceType(void) { return m_type; };
    virtual eSTMGRDeviceStatus getDeviceStatus(void) { return m_status; };
    virtual eSTMGRReturns getDeviceId(char* pDeviceID);
    virtual eSTMGRReturns getDeviceInfo(eSTMGRDeviceInfo* pDeviceInfo );
    virtual eSTMGRReturns getPartitionInfo (char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo);
    virtual eSTMGRReturns getTSBStatus (eSTMGRTSBStatus *pTSBStatus);
    virtual eSTMGRReturns setTSBMaxMinutes (unsigned int minutes);
    virtual eSTMGRReturns getTSBMaxMinutes (unsigned int *pMinutes);
    virtual eSTMGRReturns getTSBCapacityMinutes(unsigned int *pMinutes);
    virtual eSTMGRReturns getTSBCapacity(unsigned long long *pCapacity);
    virtual eSTMGRReturns getTSBFreeSpace(unsigned long long *pFreeSpace);
    virtual eSTMGRReturns getDVRCapacity(unsigned long long *pCapacity);
    virtual eSTMGRReturns getDVRFreeSpace(unsigned long long *pFreeSpace);
    virtual eSTMGRReturns setTSBEnabled (bool isEnabled);
    virtual eSTMGRReturns setDVREnabled (bool isEnabled);
    virtual bool isTSBEnabled();
    virtual bool isDVREnabled();
    virtual eSTMGRReturns getHealth (eSTMGRHealthInfo* pHealthInfo);
    virtual bool isTSBSupported();
    virtual bool isDVRSupported();
    virtual eSTMGRReturns getTSBPartitionMountPath(char* pMountPath);
    /* Populates all the device data base */
    virtual eSTMGRReturns populateDeviceDetails (void) = 0;
    /* Queries the Device for the health info */
    virtual eSTMGRReturns doDeviceHealthQuery(void) = 0;

    eSTMGRReturns registerEventCallback(fnSTMGR_EventCallback );
    void notifyEvent(eSTMGREventMessage);

};

#endif /* __RDK_STORAGE_MGR_BASE_H__ */
