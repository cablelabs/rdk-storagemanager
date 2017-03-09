#ifndef __RDK_STMGR_NVRAM_H__
#define __RDK_STMGR_NVRAM_H__
#include "rdkStorageBase.h"
/* For Udev Access */
#include <libudev.h>

class rStorageNVRAM : public rStorageMedia
{

    struct udev* m_pUDevNVRAM;
    struct udev_device* m_parentDevice;
    int m_eraseCount;

    void readEraseCount();
    void readChipDetails();

public:
    rStorageNVRAM(std::string devicePath);

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails();

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void);

    eSTMGRReturns getHealth(eSTMGRHealthInfo* pHealthInfo);
};

#endif /* __RDK_STMGR_NVRAM_H__ */

