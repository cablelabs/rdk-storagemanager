#ifndef __RDK_STMGR_NVRAM_H__
#define __RDK_STMGR_NVRAM_H__
#include "rdkStorageBase.h"

class rStorageNVRAM : public rStorageMedia
{
public:
    rStorageNVRAM(std::string devicePath) {};
    ~rStorageNVRAM() {};

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails() {return RDK_STMGR_RETURN_SUCCESS; };

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void) {return RDK_STMGR_RETURN_SUCCESS; };
};

#endif /* __RDK_STMGR_NVRAM_H__ */

