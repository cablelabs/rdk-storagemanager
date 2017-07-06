#include "rdkStorageMgr.h"
#include "libIBus.h"
#include "libIARM.h"
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


eSTMGRDeviceIDs gDeviceIDs;
void _eventCallback (eSTMGREventMessage events)
{
    std::cout <<"@@@@@@@ events.ID =" << events.m_eventType << "\n";
    return;
}

void printOptions(void)
{
    std::cout <<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
    std::cout <<" 1. GetDeviceIDs\n";
    std::cout <<" 2. GetDeviceInfo\n";
    std::cout <<" 3. GetDeviceInfoList\n";
    std::cout <<" 4. GetPartitionInfo\n";
    std::cout <<" 5. GetTSBStatus\n";
    std::cout <<" 6. SetTSBMaxMinutes\n";
    std::cout <<" 7. GetTSBMaxMinutes\n";
    std::cout <<" 8. GetTSBCapacityMinutes\n";
    std::cout <<" 9. GetTSBCapacity\n";
    std::cout <<"10. GetTSBFreeSpace\n";
    std::cout <<"11. GetDVRCapacity\n";
    std::cout <<"12. GetDVRFreeSpace\n";
    std::cout <<"13. IsTSBEnabled\n";
    std::cout <<"14. SetTSBEnabled\n";
    std::cout <<"15. IsDVREnabled\n";
    std::cout <<"16. SetDVREnabled\n";
    std::cout <<"17. GetHealth\n";
    std::cout <<"18. GetTSBPartitionMountPath\n";
    std::cout <<"33. EXIT\n";
    std::cout <<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
}

void printDeviceIDs ()
{
    if (gDeviceIDs.m_numOfDevices > 0)
    {
        for (int i = 0; i < gDeviceIDs.m_numOfDevices; i++)
        {
            std::cout << (i + 1) << "\t-\t" << gDeviceIDs.m_deviceIDs[i] << "\n";
        }
    }
    else
        std::cout <<"No device found\n";
}
void _getDeviceIds()
{
    eSTMGRReturns rc;
    rc = rdkStorage_getDeviceIds(&gDeviceIDs);

    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        printDeviceIDs();
    }
}

void printDeviceInfo (const eSTMGRDeviceInfo &deviceInfo)
{
    std::cout <<"Device ID           = " << deviceInfo.m_deviceID << "\n";
    std::cout <<"Device Type         = " << deviceInfo.m_type << "\n";
    std::cout <<"Capacity            = " << deviceInfo.m_capacity << "\n";
    std::cout <<"Status              = " << deviceInfo.m_status << "\n";
    std::cout <<"Partitions          = " << deviceInfo.m_partitions << "\n";
    std::cout <<"Manufacturer        = " << deviceInfo.m_manufacturer << "\n";
    std::cout <<"Model               = " << deviceInfo.m_model << "\n";
    std::cout <<"SerialNumber        = " << deviceInfo.m_serialNumber << "\n";
    std::cout <<"FirmwareVersion     = " << deviceInfo.m_firmwareVersion << "\n";
    std::cout <<"IfATAstandard       = " << deviceInfo.m_ifATAstandard << "\n";
    std::cout <<"HasSMARTSupport     = " << deviceInfo.m_hasSMARTSupport << "\n";
}

void _getDeviceInfo(char* pDeviceID)
{
    eSTMGRReturns rc;
    eSTMGRDeviceInfo deviceInfo;
    memset (&deviceInfo, 0, sizeof(deviceInfo));
    rc = rdkStorage_getDeviceInfo(pDeviceID, &deviceInfo);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        printDeviceInfo (deviceInfo);
    }
}


void _getDeviceInfoList()
{
    eSTMGRDeviceInfoList deviceInfoList;
    eSTMGRReturns rc;
    rc = rdkStorage_getDeviceInfoList(&deviceInfoList);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        if (deviceInfoList.m_numOfDevices > 0)
        {
            for (int i = 0; i < deviceInfoList.m_numOfDevices; i++)
            {
                std::cout <<"**********************************\n";
                printDeviceInfo (deviceInfoList.m_devices[i]);
                std::cout <<"**********************************\n";
            }
        }
        else
            std::cout <<"No devices present\n";
    }
}

void _getPartitionInfo (char* pDeviceID, char* pPartitionId)
{
    eSTMGRPartitionInfo partitionInfo;
    eSTMGRReturns rc;
    rc = rdkStorage_getPartitionInfo (pDeviceID, pPartitionId, &partitionInfo);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "PartitionId       " << partitionInfo.m_partitionId << "\n";
        std::cout << "Format            " << partitionInfo.m_format << "\n";
        std::cout << "Status            " << partitionInfo.m_status << "\n";
        std::cout << "MountPath         " << partitionInfo.m_mountPath << "\n";
        std::cout << "Capacity          " << partitionInfo.m_capacity << "\n";
        std::cout << "FreeSpace         " << partitionInfo.m_freeSpace << "\n";
        std::cout << "Is TSB Supported  " << partitionInfo.m_isTSBSupported << "\n";
        std::cout << "Is DVR Supported  " << partitionInfo.m_isDVRSupported << "\n";
    }
}

void _getTSBStatus ()
{
    eSTMGRTSBStatus TSBStatus;
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBStatus (&TSBStatus);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "TSB Status = " << TSBStatus << "\n";
    }
}

void _setTSBMaxMinutes (unsigned int minutes)
{
    eSTMGRReturns rc;
    rc = rdkStorage_setTSBMaxMinutes (minutes);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "setTSBMaxMinutes with " << minutes << "\n";
    }
}

void _getTSBMaxMinutes ()
{
    unsigned int minutes;
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBMaxMinutes (&minutes);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "GetTSBMaxMinutes = " << minutes << "\n";
    }
}

void _getTSBCapacityMinutes()
{
    unsigned int minutes;
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBCapacityMinutes(&minutes);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "GetTSBCapacity (in Mins) = " << minutes << "\n";
    }
}

void _getTSBCapacity()
{
    unsigned long capacity;
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBCapacity(&capacity);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "GetTSBCapacity = " << capacity << "\n";
    }
}

void _getDVRCapacity()
{
    unsigned long capacity;
    eSTMGRReturns rc;
    rc = rdkStorage_getDVRCapacity(&capacity);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "GetDVRCapacity = " << capacity << "\n";
    }
}

void _getTSBFreeSpace()
{
    unsigned long freeSpace;
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBFreeSpace(&freeSpace);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "getTSBFreeSpace = " << freeSpace << "\n";
    }
}

void _getDVRFreeSpace()
{
    unsigned long freeSpace;
    eSTMGRReturns rc;
    rc = rdkStorage_getDVRFreeSpace(&freeSpace);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "getDVRFreeSpace = " << freeSpace << "\n";
    }
}

void _isTSBEnabled()
{
    bool rc = rdkStorage_isTSBEnabled();
    std::cout << "Is TSB Enabled " << rc << "\n";
}

void _isDVREnabled()
{
    bool rc = rdkStorage_isDVREnabled();
    std::cout << "Is DVR Enabled " << rc << "\n";
}

void _setTSBEnabled(bool isEnabled)
{
    eSTMGRReturns rc;
    rc = rdkStorage_setTSBEnabled (isEnabled);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "setTSBEnabled = " << isEnabled << "\n";
    }
}

void _setDVREnabled(bool isEnabled)
{
    eSTMGRReturns rc;
    rc = rdkStorage_setDVREnabled (isEnabled);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "setDVREnabled = " << isEnabled << "\n";
    }
}

void _getHealth (char* pDeviceID)
{
    eSTMGRHealthInfo healthInfo;
    eSTMGRReturns rc;
    rc = rdkStorage_getHealth (pDeviceID, &healthInfo);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout <<"Device ID           = " << healthInfo.m_deviceID << "\n";
        std::cout <<"Device Type         = " << healthInfo.m_deviceType << "\n";
        std::cout <<"Is Operational      = " << healthInfo.m_isOperational << "\n";
        std::cout <<"Is Healthy          = " << healthInfo.m_isHealthy << "\n";
    }
}

void _getTSBPartitionMountPath ()
{
    char mountPath[200];
    eSTMGRReturns rc;
    rc = rdkStorage_getTSBPartitionMountPath (mountPath);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Sorry failed..\n";
    }
    else
    {
        std::cout << "TSB MountPath = " << mountPath << "\n";
    }
}

int main (int argc, char* argv[])
{
    char processName[256] = "";
    int deviceIDIndex = 0;
    char partitionID[512] = "";
    unsigned short input = 0;
    bool isEnabled = false;
    unsigned int minutes;


    sprintf (processName, "StMgr-user-%u", getpid());
    IARM_Bus_Init((const char*) &processName);
    IARM_Bus_Connect();

    rdkStorage_init();

    eSTMGRReturns rc;
    rc = rdkStorage_RegisterEventCallback(_eventCallback);
    if (rc != RDK_STMGR_RETURN_SUCCESS)
    {
        std::cout <<"Event Subscription Failed\n";
    }

    /* Add this for Automation team to get all the info */
    if (argc >= 2)
    {
        /* Print All the device Info */
        _getDeviceInfoList();

        /* TSB Status */
        _getTSBStatus();

        /* TSB Mins */
        _getTSBMaxMinutes();

        /* TSB Capacity in mins */
        _getTSBCapacityMinutes();

        /* TSB Capacity */
        _getTSBCapacity();

        /* TSB Free Space */
        _getTSBFreeSpace();

        /* DVR Capacity */
        _getDVRCapacity();

        /* DVR Free Space */
        _getDVRFreeSpace();

        /* Is TSB Enabled */
        _isTSBEnabled();

        /* Is DVR Enabled */
        _isDVREnabled();

        return 0;
    }

    while (1)
    {
        printOptions();
        std::cout <<"Enter the test index that you want to do.. \t";
        std::cin >> input;

        switch (input)
        {
        default:
            std::cout <<"Seems like you typed some wrong keys... :)\n";
            break;
        case 33:
            std::cout <<"Seems like you wanted to Quit.. Bye Bye... :)\n";
            exit(0);
            break;
        case 1:
            _getDeviceIds();
            break;
        case 2:
            printDeviceIDs();
            std::cout << "Enter the deviceID Index\n";
            std::cin >> deviceIDIndex;
            if ((deviceIDIndex > 0) && (deviceIDIndex <= gDeviceIDs.m_numOfDevices))
                _getDeviceInfo( gDeviceIDs.m_deviceIDs[(deviceIDIndex - 1)]);
            else
                std::cout << "Wrong Index\n";

            break;
        case 3:
            _getDeviceInfoList();
            break;
        case 4:
            std::cout << "Enter the deviceID Index\n";
            std::cin >> deviceIDIndex;
            std::cout << "Enter the partitionID\n";
            std::cin >> partitionID;
            if ((deviceIDIndex > 0) && (deviceIDIndex <= gDeviceIDs.m_numOfDevices))
                _getPartitionInfo (gDeviceIDs.m_deviceIDs[(deviceIDIndex - 1)], partitionID);
            else
                std::cout << "Wrong Index\n";

            break;
        case 5:
            _getTSBStatus();
            break;
        case 6:
            std::cout << "Enter the TSB Mins\n";
            std::cin >> minutes;
            _setTSBMaxMinutes(minutes);
            break;
        case 7:
            _getTSBMaxMinutes();
            break;
        case 8:
            _getTSBCapacityMinutes();
            break;
        case 9:
            _getTSBCapacity();
            break;
        case 10:
            _getTSBFreeSpace();
            break;
        case 11:
            _getDVRCapacity();
            break;
        case 12:
            _getDVRFreeSpace();
            break;
        case 13:
            _isTSBEnabled();
            break;
        case 14:
            std::cout << "Enter TSB Enable or Disable";
            std::cin >> isEnabled;
            _setTSBEnabled(isEnabled);
            break;
        case 15:
            _isDVREnabled();
            break;
        case 16:
            std::cout << "Enter DVR Enable or Disable";
            std::cin >> isEnabled;
            _setDVREnabled(isEnabled);
            break;
        case 17:
            printDeviceIDs();
            std::cout << "Enter the deviceID Index\n";
            std::cin >> deviceIDIndex;
            if ((deviceIDIndex > 0) && (deviceIDIndex <= gDeviceIDs.m_numOfDevices))
                _getHealth( gDeviceIDs.m_deviceIDs[(deviceIDIndex - 1)]);
            else
                std::cout << "Wrong Index\n";
            break;
        case 18:
            _getTSBPartitionMountPath();
        }
    }
    return 0;
}


