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



/**
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/


#ifndef SDCARDREADER_H_
#define SDCARDREADER_H_


#include <iostream>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <linux/types.h>
#include <linux/mmc/ioctl.h>
#include "udevUtils.h"
#include "storageMgr.h"
#include <sys/mount.h>


#define MMC_BLOCK_MAJOR			179

#define MMC_RSP_SPI_S1	(1 << 7)

#define MMC_RSP_SPI_R1	(MMC_RSP_SPI_S1)
#define MMC_CMD_ADTC	(1 << 5)
#define MMC_RSP_PRESENT	(1 << 0)
#define MMC_RSP_CRC	(1 << 2)
#define MMC_RSP_OPCODE	(1 << 4)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMC_DEV "/dev/mmcblk0"

#define MMC_SRC_DEV "/dev/mmcblk0p1"

#define MMC_TRGT_PATH "/mnt/nvram/data"

struct health_status {
    uint16_t tsb_id;
    char man_date[6];
    uint8_t used;
    uint8_t used_user_area;
    uint8_t used_spare_block;
    char customer_name[32];
};

typedef enum {
    NO_TSB_SUPPORT = -1,
    TSB_SUPPORT_WITH_HEALTH_MONITORING = 1,
    TSB_SUPPORT_WITHOUT_HEALTH_MONITORING = 2
} eTsbStatus;

#define TSB_ID0 0x5344
#define TSB_ID1 0x4453
#define EXCEPTION_CID "0353445453424248"

#define DEFULT_DATARATE_PER_SEC 20 /* MBPS*/
#define DEFULT_TSB_MAX_MINUTE 	60

typedef struct _SDCardInfo
{
    char 	cid[50];								/**@<	Device id	*/
//	smDeviceStatusCode_t status;					/**@< 	An integer representing one or more device status values*/
//	smDeviceType_t type;							/**@<	One of 0-Hard disk, 1-SD card, 2-USB drive	*/
    char manufacturer[20];							/**@< 	The manufacturer of this storage device		*/
    char oemid[10];									/**@< 	The oem/application id 	*/
    char pnm[10];									/**@< 	The product name		*/
    char	model[10];
    char	lotID[10];
    int		serialNumber;
    unsigned long long	capacity;					/**@<  	Capacity in bytes							*/
    unsigned long long free_space; 					/**@< 	free space in bytes							*/
    bool	readOnly;
    bool	tsbQualified;
    unsigned int	lifeElapsed;
    unsigned int	timeUsed;
    bool	writeFailed;
    bool	cardFailed;
    unsigned long	badBlocks;
    unsigned long	totalBlocks;
    bool	formatNow;
    char mountPartition[100]; 						/**@< 	The location of this storage device's partition*/
    char format[20];								/**@< 	The formatting used by this storage device	*/
    bool isTSBSupported; 							/**@<	true if this device is used for TSB storage	*/
    bool isDVRSupported;							/**@< 	true if this device is used for DVR storage	*/
    char sysPath[100];								/**@< 	system path of SD card						*/

} sdCardInfo;


void setSDCardPresenceFlag(bool);
bool isSDCardPresent();
bool stMgr_Check_SDCard_HealthStatus(struct health_status *);

void get_SDCard_PropertiesFromFile();
void get_SDCard_Properties_FromStatvfs(strMgrDeviceInfoParam_t *devInfoList);
void get_statvfs(const struct mntent *, strMgrDeviceInfoParam_t *);
bool get_SDCard_PropValueFromFile(char *o_value, const char *in_filePath);
bool get_SDCard_Properties(strMgrDeviceInfoParam_t *devInfoList);

bool read_SDCard_cid(char *);
bool read_SDCard_manfid(char *);
bool read_SDCard_serial(char *);

bool check_tsb_Supported();
bool getSDCard_TSB_Supported();
void setSDCard_TSB_Supported(bool );
uint8_t getTSBStatusCode();
bool mount_SDCard(const char*, const char*, const char  *, smDeviceStatusCode_t *);
bool umount_SDCard(const char* );
void setsdCardStatusOnReadyEvent(bool );
bool getsdCardStatusOnReadyEvent();

bool isCMD56Supported();

#ifdef USE_DISK_CHECK
bool execute_Mount_Script();
bool execute_Umount_Script();
#endif

bool isSDCardMounted();
eTsbStatus get_SDcardTsbHealthMonStatus();
void createFile(char *fileName);
#endif /* SDCARDUTILS_H_ */


/** @} */
/** @} */
