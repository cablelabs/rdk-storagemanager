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
* @file
*
* @brief IARM-Bus STORAGE Manager Internal API.
*
* This API defines the operations used to starting and stopping IARM-Bus
* STORAGE Manager.
*
* @par Document
* Document reference.
*
* @par Open Issues (in no particular order)
* -# None
*
* @par Assumptions
* -# None
*
* @par Abbreviations
* - BE:       ig-Endian.
* - cb:       allback function (suffix).
* - DS:      Device Settings.
* - FPD:     Front-Panel Display.
* - HAL:     Hardware Abstraction Layer.
* - LE:      Little-Endian.
* - LS:      Least Significant.
* - MBZ:     Must be zero.
* - MS:      Most Significant.
* - RDK:     Reference Design Kit.
* - _t:      Type (suffix).
*
* @par Implementation Notes
* -# None
*
*/

/** @defgroup IARM_BUS IARM-Bus HAL API
*   @ingroup IARM_RDK
*
*  IARM-Bus is a platform agnostic Inter-process communication (IPC) interface. It allows
*  applications to communicate with each other by sending Events or invoking Remote
*  Procedure Calls. The common programming APIs offered by the RDK IARM-Bus interface is
*  independent of the operating system or the underlying IPC mechanism.
*
*  Two applications connected to the same instance of IARM-Bus are able to exchange events
*  or RPC calls. On a typical system, only one instance of IARM-Bus instance is needed. If
*  desired, it is possible to have multiple IARM-Bus instances. However, applications
*  connected to different buses will not be able to communicate with each other.
*/






/**
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/


#ifndef _STORAGEEVENTMGRINTERNAL_H_
#define _STORAGEEVENTMGRINTERNAL_H_


#include "libIARM.h"
#include <string.h>
#include "iarmUtil.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#ifdef ENABLE_SD_NOTIFY
#include <systemd/sd-daemon.h>
#endif
#ifdef __cplusplus
}
#endif 

#include "rdk_debug.h"

#define LOG_TRACE(FORMAT, ...) RDK_LOG (RDK_LOG_TRACE1, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_DEBUG(FORMAT, ...) RDK_LOG (RDK_LOG_DEBUG, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_INFO(FORMAT, ...) RDK_LOG (RDK_LOG_INFO, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_NOTICE(FORMAT, ...) RDK_LOG (RDK_LOG_NOTICE, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_WARN(FORMAT, ...) RDK_LOG (RDK_LOG_WARN, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_ERROR(FORMAT, ...) RDK_LOG (RDK_LOG_ERROR, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)
#define LOG_FATAL(FORMAT, ...) RDK_LOG (RDK_LOG_FATAL, "LOG.RDK.STORAGEMGR", FORMAT, ##__VA_ARGS__)

#define SDCARD_CONFIG "SDCard_Config"

#define FC_MMC_DEV		"MMC_DEV_NODE"
#define FC_MMC_SRC_DEV		"MMC_SRC_DEV_NODE"
#define FC_MOUNT_PATH 		"MOUNT_PATH"
#define FC_FRAME_RATE_MBPS 	"FRAME_RATE_MBPS"
#define FC_IsTSBEnableOverride		"IsTSBEnableOverride"
#define FILE_SYSTEM_TYPE	"filesystemtype"
#define FC_DEFAULT_TSB_MAX_MINUTE "DEFAULT_TSB_MAX_MINUTE"
#define DISK_CHECK_SCRIPT	"DISK_CHECK_SCR_PATH"
#define TSB_VALIDATION_FLAG	"TSB_VALIDATION_FLAG"
#define DEV_NAME		"DEV_NAME"

typedef struct  _stmMgrConfigProp
{
    char sdCardDevNode[100];
    char sdCardSrcDevNode[100];
    char sdCardMountPath[100];
    int frameRate_mbps;
    bool isTsbEnableOverride;
    char fileSysType[20];
    unsigned short tsbMaxMin;
    char disk_check_script[100];
    bool tsbValidationFlag;
    char devFile[100];
} stmMgrConfigProp;


/** @defgroup IARM_BUS STORAGE Manager storageMgr
*   @ingroup IARM_RDK
*
*/

/** @addtogroup IARM_BUS_STORAGE_INTERNAL_API IARM-STORAGE Manager internal API
*  @ingroup IARM_BUS
*
*  Described herein are functions that are used to initialize and manage storageMgrrary.
*
*  @{
*/

/**
* @brief Starts the storageMgr.
*
* This function registers and connects STORAGE Manager to the iarm bus.
* Register Events that this module publishes and register APIs that
* can be RPCs by other entities on the bus.
*
* @return IARM_Result_t Error Code.
* @retval IARM_RESULT_SUCCESS on success
*/
IARM_Result_t storageManager_Start(void);

/**
 * @brief Terminates the storageMgr
 *
 * This function disconnects STORAGE Manager from the iarm bus and terminates
 * STORAGE Manager.
 *
 * @return IARM_Result_t Error Code.
 * @retval IARM_RESULT_SUCCESS on success
 */
IARM_Result_t storageManager_Stop(void);

/**
 * @brief Listens for component specific events from drivers.
 *
 * @return IARM_Result_t Error Code.
 * @retval IARM_RESULT_SUCCESS on success
 */
IARM_Result_t storageManager_Loop(void);



#endif


/* End of IARM_BUS_STORAGE_INTERNAL_API doxygen group */
/**
 * @}
 */


/** @} */
/** @} */
