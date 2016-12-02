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


#ifndef UDEVUTILS_H_
#define UDEVUTILS_H_

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#define ADD_FILTER "add"
#define REMOVE_FILTER "remove"

/*For SD card */
#define SUBSYSTEM_FILTER_MMC "mmc"
#define ATTR_FILTER_MMC "MMC_TYPE"
#define ATTR_VALUE_SD "SD"
#define ATTR_VALUE_MMC "MMC"
#define DEVTYPE_FILTER "disk"
#define ATTR_ADDED_DISK "UDISKS_PARTITION_TABLE"

static struct udev* _udevDevInstance = NULL;

void *stMgrMonitorThead(void *);

void stMgr_udev_Init();
void stMgr_udev_Deinit();
bool getudevinfo();

#endif /* UDEVUTILS_H_ */


/** @} */
/** @} */
