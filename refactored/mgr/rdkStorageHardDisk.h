#ifndef __RDK_STMGR_HDD_H__
#define __RDK_STMGR_HDD_H__
#include "rdkStorageBase.h"

class rStorageHDDrive : public rStorageMedia
{

public:
    rStorageHDDrive(std::string devicePath) {};
    ~rStorageHDDrive() {};

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails() { return RDK_STMGR_RETURN_SUCCESS;}

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void) { return RDK_STMGR_RETURN_SUCCESS;}
};

#endif /* __RDK_STMGR_HDD_H__ */

