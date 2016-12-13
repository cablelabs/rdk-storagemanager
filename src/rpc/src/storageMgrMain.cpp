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


#include "storageMgrInternal.h"

char strMgr_ConfigProp_FilePath[100] = {'\0'};

static void STRMgr_SignalHandler (int sigNum);

void STRMgr_SignalHandler (int sigNum)
{
    LOG_WARN ("[%s:%d] sigNum = %d\n", __FUNCTION__, __LINE__, sigNum);

    signal(sigNum, SIG_DFL );
    kill(getpid(), sigNum );
}

int main(int argc, char *argv[])
{
    const char* debugConfigFile = NULL;

    for (int itr = 0; itr < argc; itr++)
    {
        if (strcmp (argv[itr], "--debugconfig") == 0)
        {
            if (++itr < argc)
            {
                debugConfigFile = argv[itr];
            }
            else
            {
                break;
            }
        }

        if (strcmp (argv[itr], "--configFilePath") == 0)
        {
            if (++itr < argc)
            {
                strcpy (strMgr_ConfigProp_FilePath, argv[itr]);
            }
            else
            {
                break;
            }
        }
    }

    bool b_rdk_logger_enabled = (rdk_logger_init (debugConfigFile) == 0);

    /* Signal handler */
    signal (SIGHUP, STRMgr_SignalHandler);
    signal (SIGINT, STRMgr_SignalHandler);
    signal (SIGQUIT, STRMgr_SignalHandler);
    signal (SIGILL, STRMgr_SignalHandler);
    signal (SIGABRT, STRMgr_SignalHandler);
    signal (SIGFPE, STRMgr_SignalHandler);
    signal (SIGSEGV , STRMgr_SignalHandler);
    signal (SIGTERM, STRMgr_SignalHandler);
    signal (SIGPIPE, SIG_IGN);

    storageManager_Start();

#if 0
// #ifdef ENABLE_SD_NOTIFY
    sd_notifyf(0, "READY=1\n"
              "STATUS=storageMgrMain is Successfully Initialized\n"
              "MAINPID=%lu",
              (unsigned long) getpid());
#endif
    storageManager_Loop();
    storageManager_Stop();
}



/** @} */
/** @} */
