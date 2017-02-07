/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2017 RDK Management
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
* @brief SMART monitor tools executor API for health monitoring for hard disk.
*
* This API defines the operations used to execute and get the data from STORAGE Manager.
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

/** @defgroup
*   @ingroup
*
*/


/**
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/

#ifndef _SMARTMON_UTILES_H_
#define _SMARTMON_UTILES_H_

//#ifdef ENABLE_SMARTMONTOOL_SUPPORT

#include <iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include<fstream>
#include <vector>
#include <stdio.h>
#include <regex.h>
#include <cstring>
#include <map>

#define LINE_MAX_BUFFER 256

/* SMART MON TOOL PROPERTY STRINGS */
#define SMARTMON_HDD_ID_STR			"LU WWN Device Id"
#define SMARTMON_CAPACITY_STR		"User Capacity"
#define SMARTMON_MODEL_FAMILY_STR	"Model Family"
#define SMARTMON_MODEL_STR			"Device Model"
#define SMARTMON_SERIAL_NUMBER_STR	"Serial Number"
#define SMARTMON_FIRMWARE_VER_STR	"Firmware Version"
#define SMARTMON_ATA_STD_STR		"ATA Standard is"
#define SMARTMON_SUPPORT_STR		"SMART support is"
#define SMARTMON_OVERALL_HEALTH_STR	"SMART overall-health self-assessment test result"

class smartmonUtiles {

public:
    smartmonUtiles() {
        this->cmd_name_="";
        this->cmd_args_="";
//        LOG_TRACE("[%s]Inside constructor.\n", __FUNCTION__);
    }

    ~smartmonUtiles() {
//        LOG_TRACE("[%s]Inside destructor.\n", __FUNCTION__);
    }

    bool execute();
    /* Set string to execute: smartctl executable name with path and argument*/
    void setCmd(std::string& cmd_name, std::string& cmd_args) {
        this->cmd_name_ = cmd_name;
        this->cmd_args_ = cmd_args;
    }


    /* Device Info properties methods */
    bool matchSMARTSupportExpression(std::string &s);
    bool getAtaStandard(std::string& ata_standard);
    long getCapacity();
    bool getFirmwareVersion(std::string& fwVer);
    bool isSmartSupport();
    bool getManufacture(std::string& manufacture);
    bool getDeviceId(std::string& deviceId);
    bool getModel(std::string& deviceModel);
    bool getSerialNumber(std::string& srNum);
    bool isOverallHealthOkay();
    bool getDiagnosticAttributes(std::map<string, string>& attrMap);

private:
    std::string cmd_name_;
    std::string cmd_args_;
    std::vector<std::string> stdOutlist;			/* List of std output after Execution smartctl command*/

    bool reg_matches(const char *str, const char *pattern);
    std::string trim(std::string& str);
    /* smartctl parser methods*/
    bool parse_match_string(const char* pattern, std::string& val);
    bool parse_capacity_string(std::string& s, long& capacity);

    /* smartctl std output in vector */
    std::vector<std::string> getStdOutlist() {
        return this->stdOutlist;
    }
};

//#endif /* ENABLE_SMARTMONTOOL_SUPPORT*/

#endif
/* End of SMART_MONITOR_TOOL_COMMAND_API doxygen group */

/**
 * @}
 */


/** @} */
/** @} */

