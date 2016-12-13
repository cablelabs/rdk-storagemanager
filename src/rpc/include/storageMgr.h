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
* @brief IARM-Bus STORAGE Manager Public API.
*
* This API defines the operations for the IARM-Bus STORAGE Manager interface.
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

/** @defgroup IARM_BUS STORAGE Manager STORAGE lib
*   @ingroup IARM_RDK
*
*/

/** @addtogroup IARM_BUS_STORAGE_LIB_API IARM-STORAGE Manager API
*  @ingroup IARM_BUS
*
*  Described herein are functions and structures exposed by STORAGE library.
*
*  @{
*/



/**
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/


#ifndef _STORAGE_EVENT_MGR_H_
#define _STORAGE_EVENT_MGR_H_

#include "libIBus.h"
#include "libIARM.h"

#define IARM_BUS_ST_MGR_NAME "STORAGE_MGR"     /*!< Well-known Name for STORAGE libarary */

#define MAX_BUF 100
#define DEV_ID_BUF 50
#define MAX_DESC_BUF 250
#define MOUNT_PATH_BUF 	64

#define INCOMPATIBLE_SDCARD 			"Incompatible SD Card."
#define PRESENT_WITH_HEALTH_MONITORING		"Present"
#define PRESENT_WITHOUT_HEALTH_MONITORING	"Present with no health monitoring"
#define NOT_PRESENT				"None"
#define NOT_MOUNTED				"Error : Not Mounted"

#define IARM_BUS_STORAGE_MGR_API_GetDeviceIds 		"getDeviceIds" 	   	/*!< Retrives the array of strings representing ids of each device*/
#define IARM_BUS_STORAGE_MGR_API_GetDeviceInfo      "getDeviceInfo"    	/*!< Returns a hash of storage devices keyed by the device id from the box*/
#define IARM_BUS_STORAGE_MGR_API_GetTSBStatus       "getTSBStatus"     	/*!< get TSB Status from the box*/
#define IARM_BUS_STORAGE_MGR_API_GetTSBMaxMinutes 	"getTSBMaxMinutes" 	/*!< Returns the number of minutes allowed for TSB storage */
#define IARM_BUS_STORAGE_MGR_API_SetTSBMaxMinutes 	"setTSBMaxMinutes" 	/*!< Set the number of minutes allowed for TSB storage */
#define IARM_BUS_STORAGE_MGR_API_IsTSBEnabled	 	"isTSBEnabled" 	   	/*!< Returns the TSB Enable status; bool true when TSB is enabled else false */
#define IARM_BUS_STORAGE_MGR_API_SetTSBEnabled	 	"setTSBEnabled"     /*!< Returns the TSB Enable status; bool true when TSB is enabled else false */
#define IARM_BUS_STORAGE_MGR_API_IsTSBCapable	 	"isTSBCapable"    	/*!< Returns the TSB Enable status; bool true when TSB is enabled else false */
#define IARM_BUS_STORAGE_MGR_API_GetSDcardStatus	"getSDCardStatus"    	/*!< Returns the SDCard status; bool true when Ready else false */
#define IARM_BUS_STORAGE_MGR_API_GetSDcardPropertyInfo	"getSDCardPropertyInfo" /*!< Returns the SDCard property; bool true when Ready else false */
#define IARM_BUS_STORAGE_MGR_API_GetMountPath		"getMountPath" 		/*!< Returns the Mount Directory Path */




/** This enumeration defines a set of Device Status Codes from connected devices
     *  (e.g. SD, Hard disk and Usb)standard status code types.
     *
     */

typedef enum _smDeviceStatusCode_t {
    SM_DEV_STATUS_OK,                                            /**@<  0 : The device is functioning normally                                  */
    SM_DEV_STATUS_READ_ONLY,                                     /**@<  1 : The device is a read only device or has write protection locked     */
    SM_DEV_STATUS_NOT_PRESENT,                                   /**@<  2 : The storage device is not present or has been removed               */
    SM_DEV_STATUS_NOT_QUALIFIED     =       4,                   /**@<  The device is not a qualified device.  For example, a class two \
                                                                          SDHC card is inserted into a device that requires a class ten card.   */
    SM_DEV_STATUS_DISK_FULL         =       8,                 	 /**@<  The disk is full.  This is different from a disk that is READ_ONLY.     */
    SM_DEV_STATUS_READ_FAILURE      =       16,              	 /**@<  An error occurred during reading                                        */
    SM_DEV_STATUS_WRITE_FAILURE     =       32,                  /**@<  An error occurred during writing                                        */
    SM_DEV_STATUS_UNKNOWN           =       64,                  /**@<  An unknown error occurred                                               */
    SM_DEV_STATUS_READY             =       128,                 /**@<  SDCard is ready.                                               */
    SM_DEV_STATUS_UMOUNT						                 /**@<  SDCard is ready to umount.                                               */
} smDeviceStatusCode_t;


/** This enumeration defines a type of one of 0-Hard disk, 1-SD card, 2-USB drive *
  *
  */
typedef enum _smDeviceType_t {
    SM_DEV_HARD_DISK,                                           /**@<  0 : Hard disk   */
    SM_DEV_SD_MMC,                                              /**@<  1 : SD                  */
    SM_DEV_USB_DRIVE,                                           /**@<  2 : USB drive   */
} smDeviceType_t;


typedef struct _sm_Events
{
    unsigned char deviceId[MAX_BUF];                  			/**@<   Fired when a device changes status.
                                                        	For example,
                                                          		an SD card could be removed and then re-inserted with read-only protection enabled;
                                                                	this would result in two different events firing.  One for the device's removal and another
                                                                	for the insertion where the status codes would be 2 and 1 respectively. */
    unsigned short status;                            			/**@<   The cause is indicated by the status which can represent multiple values, for example,
                                                                        a write error (32) due to a full disk (8) would result in a status of 40.*/
    unsigned char description[MAX_BUF];                           /**@<   An optional description of the error can also be provided.*/
} sm_Events;



typedef struct _STRMgr_DeviceInfo_Param_t {
    char devid[DEV_ID_BUF];                             	/*!< [in/out] Device id       */
    smDeviceType_t type;                            	/*!< [out] One of 0-Hard disk, 1-SD card, 2-USB drive      */
    unsigned long long capacity;                        /*!< [out] Capacity in bytes                               */
    unsigned long long free_space;                      /*!< [out] Capacity in bytes                               */
    unsigned int status;        	                    /*!< [out] An integer representing one or more device status values*/
    unsigned char mountPartition[MAX_BUF];              /*!< [out] The location of this storage device's partition*/
    unsigned char format[MAX_BUF];                      /*!< [out] The formatting used by this storage device      */
    char manufacturer[MAX_BUF];                   		/*!< [out] The manufacturer of this storage device         */
    bool isTSBSupported;                        		/*!< [out] true if this device is used for TSB storage     */
    bool isDVRSupported;                        		/*!< [out] true if this device is used for DVR storage     */
    int fileSysflag;						/*!< [out] ync with the definitions in <sys/mount.h>.  */

} strMgrDeviceInfoParam_t;

typedef union _uSDCardParamValue {
    unsigned char uchVal[MAX_BUF];
    unsigned int ui32Val;
    int iVal;
    bool bVal;
} uSDCard_Param_Val;

typedef enum _eSD_PROPERTY_Type {
    SD_Capacity,
    SD_CardFailed,
    SD_LifeElapsed,
    SD_LotID,
    SD_Manufacturer,
    SD_Model,
    SD_ReadOnly,
    SD_SerialNumber,
    SD_TSBQualified,
    SD_Status
} eSD_ParamPropertyType;

typedef struct _strMgrSDcardPropParam_t {
    uSDCard_Param_Val sdCardProp;
    eSD_ParamPropertyType eSDPropType;
} strMgrSDcardPropParam_t;


typedef struct _IARM_Bus_STRMgr_Param_t {
    union {
        struct _DEVICE_ID_DATA {
            char devId[DEV_ID_BUF];
        } deviceID;
        strMgrDeviceInfoParam_t deviceInfo;
        struct _TSB_STATUS_CODE {
            char tsbStatus;
        } tsbStatusCode;
        struct _TSB_MAX_MINUTE {
            int tsbMaxMin;
        } tsbMaxMin;
        struct _TSB_ENABLE_DATA {
            bool tsbEnabled;
        } tsbEnableData;
        struct _TSB_CAPABLE_DATA {
            bool istsbCapable;
        } tsbCapableData;
        struct _SDCARD_STATUS_DATA {
            bool isSDCardReady;
        } sdCardStatusData;
        struct _DEV_MOUNT_PATH {
            char mountDir[MOUNT_PATH_BUF];
        } mountPath;
        strMgrSDcardPropParam_t stSDCardParams;
    } data;
    int status;
} IARM_Bus_STRMgr_Param_t;



/*! Event Data associated with Stoarge Managers */
typedef struct _IARM_BUS_STRMgr_EventData_t {
    union {
        struct _CARD_STATUS_DATA {
            char deviceId[DEV_ID_BUF];
            smDeviceStatusCode_t status;
            char mountDir[MOUNT_PATH_BUF];
        } cardStatus;
        struct _CARD_TSB_ERROR_DATA {
            char deviceId[DEV_ID_BUF];
            smDeviceStatusCode_t status;
            char description[MAX_DESC_BUF];
        } tsbErrStatus;
    } data;
} IARM_Bus_STRMgr_EventData_t;


/*! Events published from Storage Manager */
typedef enum _IARM_Bus_STR_Mgr_EventId_t {
    IARM_BUS_STORAGE_MGR_EVENT_OnDeviceStatusChanged = 1,     		/*!< Insert Event in case of plugin SD, USB and Hard disk (onDeviceStatusChanged)  */
    /*!< Remove Event for unplugging SD, USB and Hard disk  */
    IARM_BUS_STORAGE_MGR_EVENT_TSB_ERROR, 		/*!< Fired when a TSB error occurs  */
    IARM_BUS_STORAGE_MGR_EVENT_MAX,           		/*!< Maximum event id*/
} IARM_Bus_STRMgr_Mgr_EventId_t;




#endif //_STORAGE_MGR_H_


/* End of IARM_BUS_STORAGE_LIB_API doxygen group */
/**
 * @}
 */


/** @} */
/** @} */
