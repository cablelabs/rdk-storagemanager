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
        sleep(300);
    }

    STMGRLOG_INFO ("I-ARM STMgr Bus: Quiting %s\n", ctime(&curr));

    return 0;
}
