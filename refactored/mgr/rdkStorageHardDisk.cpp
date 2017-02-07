#include "rdkStorageHardDisk.h"

#include "smartmonUtiles.h"

using namespace std;



rStorageHDDrive::rStorageHDDrive(std::string devicePath)
{
    m_devicePath = devicePath;
};

/*
rStorageHDDrive::~rStorageHDDrive()
{

};
*/


eSTMGRReturns rStorageHDDrive::populateDeviceDetails()
{
    /* Initial properties by smartctl api */
    get_SmartMonAttribute_Info();

    //    m_devicePath = "/dev/sda";
    m_type = RDK_STMGR_DEVICE_TYPE_HDD;

    /*@TODO: Need to update, currently commenting*/
//    m_partitionInfo;

    /* This is to track whether this memory device is supporting it our not; ie one of the partition could be used for TSB.*/
    /*@TODO: Need to get the source. Keeping default value as 'true'*/
    /*Get TSB status */
    m_isTSBEnabled = false;

    if(false == m_isTSBEnabled)
    {
        m_isTSBSupported = false;
        m_maxTSBCapacityinKB = 0;
        m_freeTSBSpaceLeftinKB = 0;
        m_maxTSBCapacityinMinutes = 0;
        m_maxTSBLengthConfigured = 0;
        m_tsbStatus = RDK_STMGR_TSB_STATUS_DISABLED;
    }

    /* This is to track whether this memory device is supporting it our not; ie one of the partition could be used for DVR.*/
    /*@TODO: Need to get the source. Keeping default value as 'false'*/
    m_isDVREnabled = false;
    if(m_isDVREnabled) {
        m_maxDVRCapacityinKB = 0;
        m_freeDVRSpaceLeftinKB = 0;
        m_isDVRSupported = false;
    }

    return RDK_STMGR_RETURN_SUCCESS;
}

bool rStorageHDDrive::get_SmartMonAttribute_Info()
{
    bool ret = true;
    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
    string args = "-iH /dev/sda";

    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {

        std::string deviceId;
        if(smCmd->getDeviceId(deviceId)) {
            strncpy(m_deviceID, deviceId.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }
        string manufacture;
        if( smCmd->getManufacture(manufacture))    {
            strncpy(m_manufacturer, manufacture.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string model;
        if(smCmd->getModel(model)) {
            strncpy(m_model, model.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string firmwareVersion;
        if(smCmd->getFirmwareVersion(firmwareVersion)) {
            strncpy(m_model, firmwareVersion.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        std::string serialNumber;
        if(smCmd->getSerialNumber(serialNumber)) {
            strncpy(m_serialNumber, serialNumber.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        string ata_standard;
        if(smCmd->getAtaStandard(ata_standard)) {
            strncpy(m_ifATAstandard, ata_standard.c_str(), RDK_STMGR_MAX_STRING_LENGTH);
        }

        m_capacity = smCmd->getCapacity();
        m_hasSMARTSupport = smCmd->isSmartSupport();

        m_healthInfo.m_isHealthy = smCmd->isOverallHealthOkay();

    }
    else {
        ret = false;
    }
    delete smCmd;
    return ret;
}

bool rStorageHDDrive::get_OverallHealth_State()
{
    bool overall_health = false;
    smartmonUtiles *smCmd = new smartmonUtiles();
    string smExePath = "/usr/sbin/smartctl";
    string args = "-H /dev/sda";
    smCmd->setCmd(smExePath, args);

    if(smCmd->execute())
    {
        overall_health = smCmd->isOverallHealthOkay();
    }
    delete smCmd;

    return overall_health;
}

eSTMGRReturns rStorageHDDrive::getHealth (eSTMGRHealthInfo* pHealthInfo)
{
    memset(pHealthInfo, 0, sizeof(eSTMGRHealthInfo));
    if(get_OverallHealth_State()) {
        pHealthInfo->m_isHealthy = true;
        pHealthInfo->m_isOperational = true;
    }
    else
    {
        pHealthInfo->m_isHealthy = false;
        pHealthInfo->m_isOperational = false;
    }

    strncpy(pHealthInfo->m_deviceID, m_deviceID, RDK_STMGR_MAX_STRING_LENGTH);
    pHealthInfo->m_deviceType = m_type;
    return RDK_STMGR_RETURN_SUCCESS;
}

