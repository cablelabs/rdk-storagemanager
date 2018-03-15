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
#ifndef __RDK_STMGR_SDCARD_H__
#define __RDK_STMGR_SDCARD_H__
#include "rdkStorageBase.h"

#include <cstring>
#include <libudev.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <linux/types.h>
#include <linux/mmc/ioctl.h>
#include <sys/fcntl.h>


typedef struct _sdCard_Health_Status {
    uint16_t tsb_id;
    char man_date[6];
    uint8_t used;
    uint8_t used_user_area;
    uint8_t used_spare_block;
    char customer_name[32];
} eSTMGRSdHSInfo;

typedef enum {
    NO_TSB_SUPPORT = -1,
    TSB_SUPPORT_WITH_HEALTH_MONITORING = 1,
    TSB_SUPPORT_WITHOUT_HEALTH_MONITORING = 2
} eTsbHMStatus;

#define TSB_ID0 0x5344
#define TSB_ID1 0x4453
#define EXCEPTION_CID "0353445453424248"

#define DEFULT_DATARATE_PER_SEC 18 /* MBPS*/
#define DEFULT_TSB_MAX_MINUTE  60
#define DEFULT_TSB_MIN_MINUTE  25

class rStorageSDCard : public rStorageMedia
{
    string m_scr;
    string m_date;
    string m_oemid;
    string m_SMMountPath;
    string m_SMDiskValid;
    bool m_isCMD56Enable;
    eTsbHMStatus m_tsbHM;
    bool m_isMounted;
    string m_devMount;
    struct udev* m_pUDevSDC;
    pthread_mutex_t m_mountLock = PTHREAD_MUTEX_INITIALIZER;

    bool readSdcSysAttributes(struct udev_device *);
    bool check_HealthStatus(eSTMGRSdHSInfo *);
    bool isSDTSBSupported();
    bool doMountSDC();
    bool doUmountSDC();
    bool get_SdcPropertiesStatvfs();

public:
    rStorageSDCard(std::string devicePath);
    ~rStorageSDCard();

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails();

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void);

    eSTMGRReturns getHealth(eSTMGRHealthInfo* pHealthInfo);
};

#endif /* __RDK_STMGR_SDCARD_H__ */

