#ifndef __RDK_STMGR_SDCARD_H__
#define __RDK_STMGR_SDCARD_H__
#include "rdkStorageBase.h"

class rStorageSDCard : public rStorageMedia
{
public:
    rStorageSDCard(std::string devicePath) {};
    ~rStorageSDCard() {};

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails() {return RDK_STMGR_RETURN_SUCCESS; };

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void) {return RDK_STMGR_RETURN_SUCCESS; };
};

#endif /* __RDK_STMGR_SDCARD_H__ */

