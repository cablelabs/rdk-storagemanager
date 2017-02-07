#ifndef __RDK_STMGR_HDD_H__
#define __RDK_STMGR_HDD_H__
#include "rdkStorageBase.h"

class rStorageHDDrive : public rStorageMedia
{
private:
    bool get_SmartMonAttribute_Info(); /*  smartctl -H /dev/sda */
    bool get_OverallHealth_State(); /*  smartctl -i /dev/sda */

public:
    rStorageHDDrive(std::string devicePath); /*{};*/
    ~rStorageHDDrive() {};

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails();

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void) {
        return RDK_STMGR_RETURN_SUCCESS;
    }
    eSTMGRReturns getHealth (eSTMGRHealthInfo* pHealthInfo);
};

#endif /* __RDK_STMGR_HDD_H__ */

