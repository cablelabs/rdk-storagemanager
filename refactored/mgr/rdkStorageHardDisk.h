/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifndef __RDK_STMGR_HDD_H__
#define __RDK_STMGR_HDD_H__
#include "rdkStorageBase.h"

#include <xfs/xfs.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <sys/fcntl.h>
#include <unistd.h>

class rStorageHDDrive : public rStorageMedia
{
private:
    bool get_SmartMonAttribute_Info();
    bool get_OverallHealth_State();
    bool get_DiagnosticAttribues(eSTMGRDiagAttributesList* m_diagnosticsList);
    bool populatePartitionDetails();
    bool get_filesystem_statistics(const struct mntent *fs, const char* dev);
    bool get_Xfs_fs_stat(rStoragePartition *partition, const char* mountpoint);
    bool logTel_DiagnosticAttribues(std::string& attrString );
    void telemetry_logging();
public:
    rStorageHDDrive(std::string devicePath); /*{};*/
    ~rStorageHDDrive();

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails();

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void); /*{
        return RDK_STMGR_RETURN_SUCCESS;
    }*/
    eSTMGRReturns getHealth (eSTMGRHealthInfo* pHealthInfo);

//    eSTMGRReturns getPartitionInfo (char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo);

};

#endif /* __RDK_STMGR_HDD_H__ */

