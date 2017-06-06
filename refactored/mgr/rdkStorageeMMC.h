#ifndef __RDK_STMGR_EMMC_H__
#define __RDK_STMGR_EMMC_H__
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


typedef struct _eMMC_Health_Status {
    uint16_t tsb_id;
    char man_date[6];
    uint8_t used;
    uint8_t used_user_area;
    uint8_t used_spare_block;
    char customer_name[32];
} eSTMGReMMCHSInfo;

typedef enum {
    NO_EMMC_TSB_SUPPORT = -1,
    EMMC_TSB_SUPPORT_WITH_HEALTH_MONITORING = 1,
    EMMC_TSB_SUPPORT_WITHOUT_HEALTH_MONITORING = 2
} eTsbeMMCHMStatus;

#define DEFULT_DATARATE_PER_SEC 18 /* MBPS*/
#define DEFULT_TSB_MAX_MINUTE  60
#define DEFULT_TSB_MIN_MINUTE  25

class rStorageeMMC : public rStorageMedia
{
    string m_scr;
    string m_date;
    string m_oemid;
    eTsbeMMCHMStatus m_tsbHM;
    bool m_isMounted;
    string m_devMount;
    struct udev* m_pUDeveMMC;
    pthread_mutex_t m_mountLock = PTHREAD_MUTEX_INITIALIZER;

    bool readeMMCSysAttributes(struct udev_device *);
    bool check_HealthStatus(eSTMGReMMCHSInfo *);
    bool iseMMCTSBSupported();
    bool doMounteMMC();
    bool doUmounteMMC();
    bool get_eMMCPropertiesStatvfs();

public:
    rStorageeMMC(std::string devicePath);
    ~rStorageeMMC();

    /* Populates all the device data base */
    eSTMGRReturns populateDeviceDetails();

    /* Queries the Device for the health info */
    eSTMGRReturns doDeviceHealthQuery(void);

    eSTMGRReturns getHealth(eSTMGRHealthInfo* pHealthInfo);
};

#endif /* __RDK_STMGR_SDCARD_H__ */

