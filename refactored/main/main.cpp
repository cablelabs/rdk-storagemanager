#include <time.h>
#include <unistd.h>
#include "rdkStorageMgrLogger.h"
#include "rdkStorageMgr.h"

extern void stmgr_BeginIARMMode();

int main ()
{
    time_t curr;
    rdkStorage_init();
    stmgr_BeginIARMMode();
    while(1)
    {
        time(&curr);
        STMGRLOG_INFO("I-ARM STMgr Bus: HeartBeat at %s\n", ctime(&curr));
        sleep(10);
    }

    STMGRLOG_INFO ("I-ARM STMgr Bus: Quiting %s\n", ctime(&curr));

    return 0;
}
