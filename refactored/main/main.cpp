#include <time.h>
#include <unistd.h>
#include "rdkStorageMgrLogger.h"
#include "rdkStorageMgr.h"

#ifdef ENABLE_SD_NOTIFY
#include <systemd/sd-daemon.h>
#endif

extern void stmgr_BeginIARMMode();

int main ()
{
    time_t curr;

#ifdef ENABLE_SD_NOTIFY
    sd_notifyf(0, "READY=1\n" "STATUS=storageMgrMain is Ready..\n" "MAINPID=%lu",  (unsigned long) getpid());
#endif

    /* Start the Module */
    stmgr_BeginIARMMode();
    rdkStorage_init();

    while(1)
    {
        time(&curr);
        STMGRLOG_INFO("I-ARM STMgr Bus: HeartBeat at %s", ctime(&curr));
        sleep(10);
    }

    STMGRLOG_INFO ("I-ARM STMgr Bus: Quiting %s\n", ctime(&curr));

    return 0;
}
