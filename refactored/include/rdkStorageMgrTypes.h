#ifndef __RDK_STORAGE_MGR_TYPES_H__
#define __RDK_STORAGE_MGR_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Defines */
#define RDK_STMGR_MAX_DEVICES           10
#define RDK_STMGR_MAX_STRING_LENGTH     128
#define RDK_STMGR_PARTITION_LENGTH      256
#define RDK_STMGR_DIAGNOSTICS_LENGTH    256
#define RDK_STMGR_MAX_DIAGNOSTIC_ATTRIBUTES          20	/*!< Max Number of SMART diagnostics attributes. */

/* Structs & Enums */
typedef enum _stmgr_ReturnCode {
    RDK_STMGR_RETURN_SUCCESS = 0,
    RDK_STMGR_RETURN_GENERIC_FAILURE = -1,
    RDK_STMGR_RETURN_INIT_FAILURE = -2,
    RDK_STMGR_RETURN_INVALID_INPUT = -3,
    RDK_STMGR_RETURN_UNKNOWN_FAILURE = -4
}
eSTMGRReturns;

typedef enum _stmgr_DeviceType {
    RDK_STMGR_DEVICE_TYPE_HDD   = 0,
    RDK_STMGR_DEVICE_TYPE_SDCARD,
    RDK_STMGR_DEVICE_TYPE_USB,
    RDK_STMGR_DEVICE_TYPE_FLASH,
    RDK_STMGR_DEVICE_TYPE_NVRAM,
    RDK_STMGR_DEVICE_TYPE_MAX
} eSTMGRDeviceType;

typedef enum _stmgr_DeviceStatus {
    RDK_STMGR_DEVICE_STATUS_OK              = 0,
    RDK_STMGR_DEVICE_STATUS_READ_ONLY       = (1 << 0),
    RDK_STMGR_DEVICE_STATUS_NOT_PRESENT     = (1 << 1),
    RDK_STMGR_DEVICE_STATUS_NOT_QUALIFIED   = (1 << 2),
    RDK_STMGR_DEVICE_STATUS_DISK_FULL       = (1 << 3),
    RDK_STMGR_DEVICE_STATUS_READ_FAILURE    = (1 << 4),
    RDK_STMGR_DEVICE_STATUS_WRITE_FAILURE   = (1 << 5),
    RDK_STMGR_DEVICE_STATUS_UNKNOWN         = (1 << 6)
} eSTMGRDeviceStatus;

typedef enum _stmgr_TSBStatus {
    RDK_STMGR_TSB_STATUS_OK  = 1,
    RDK_STMGR_TSB_STATUS_DISABLED = 0,
    RDK_STMGR_TSB_STATUS_FAILED = -1
} eSTMGRTSBStatus;

typedef enum _stmgr_events {
    RDK_STMGR_EVENT_STATUS_CHANGED = 0,
    RDK_STMGR_EVENT_TSB_ERROR,
    RDK_STMGR_EVENT_HEALTH_WARNING,
    RDK_STMGR_EVENT_DEVICE_FAILURE
} eSTMGREvents;

typedef struct _stmgr_DeviceIds {
    unsigned short m_numOfDevices;
    char m_deviceIDs[RDK_STMGR_MAX_DEVICES][RDK_STMGR_MAX_STRING_LENGTH];
} eSTMGRDeviceIDs;

typedef struct _stmgr_DeviceInfo {
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRDeviceType m_type;
    unsigned long m_capacity;
    eSTMGRDeviceStatus m_status;
    char m_partitions[RDK_STMGR_PARTITION_LENGTH];
    char m_manufacturer[RDK_STMGR_MAX_STRING_LENGTH];
    char m_model[RDK_STMGR_MAX_STRING_LENGTH];
    char m_serialNumber[RDK_STMGR_MAX_STRING_LENGTH];
    char m_firmwareVersion[RDK_STMGR_MAX_STRING_LENGTH];
    char m_ifATAstandard[RDK_STMGR_MAX_STRING_LENGTH];
    bool m_hasSMARTSupport;
} eSTMGRDeviceInfo;

typedef struct _stmgr_DeviceInfos {
    unsigned short m_numOfDevices;
    eSTMGRDeviceInfo m_devices[RDK_STMGR_MAX_DEVICES];
} eSTMGRDeviceInfoList;

typedef struct _stmgr_PartitionInfo {
    char m_partitionId [RDK_STMGR_MAX_STRING_LENGTH];
    char m_format[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRDeviceStatus m_status;
    unsigned long m_capacity;
    unsigned long m_freeSpace;
    bool m_isTSBSupported;
    bool m_isDVRSupported;
} eSTMGRPartitionInfo;

typedef struct _stmgr_DiagnosticsAttributes {
    char m_name[RDK_STMGR_MAX_STRING_LENGTH];	/*!< Gives SMART diagnostics attributes name. */
    char m_value[RDK_STMGR_MAX_STRING_LENGTH];	/*!< Gives SMART diagnostics attributes value, Comma separated. */
} eSTMGRDiagAttributes;

typedef struct _stmgr_DiagnosticsAttributeList {
    unsigned short m_numOfAttributes;
    eSTMGRDiagAttributes m_diagnostics[RDK_STMGR_MAX_DIAGNOSTIC_ATTRIBUTES];
} eSTMGRDiagAttributesList;

typedef struct _stmgr_Health {
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRDeviceType m_deviceType;
    bool m_isOperational;
    bool m_isHealthy;
    eSTMGRDiagAttributesList m_diagnosticsList;
} eSTMGRHealthInfo;

typedef struct _stmgr_EventMessage {
    eSTMGREvents m_eventType;
    char m_deviceID[RDK_STMGR_MAX_STRING_LENGTH];
    eSTMGRDeviceType m_deviceType;
    eSTMGRDeviceStatus m_deviceStatus;
    char m_description[RDK_STMGR_MAX_STRING_LENGTH];
    char m_diagnostics[RDK_STMGR_DIAGNOSTICS_LENGTH];
} eSTMGREventMessage;

typedef void (*fnSTMGR_EventCallback)(eSTMGREventMessage);
#ifdef __cplusplus
}
#endif

#endif /* __RDK_STORAGE_MGR_TYPES_H__ */
