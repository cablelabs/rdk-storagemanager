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
eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long *pCapacityInKB);

/* Get TSBFreeSpace*/
eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long *pFreeSpaceInKB);

/* Get DVRCapacity */
eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long *pCapacityInKB);

/* Get DVRFreeSpace*/
eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long *pFreeSpaceInKB);

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

#ifdef __cplusplus
}
#endif

#endif /* __RDK_STORAGE_MGR_H__ */
