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
* @defgroup storagemanager
* @{
* @defgroup src
* @{
**/

#ifdef ENABLE_SMARTMONTOOL_SUPPORT

#include "smartcmd_executor.h"

bool smartcmdexc::execute()
{
    FILE *stream;
    char buffer[LINE_MAX_BUFFER] = {'\0'};

    std::string cmd = this->cmd_name_ + " " + this->cmd_args_;
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (!stream) {
        return false;
    }

    while (!feof(stream)) {
        if (fgets(buffer, LINE_MAX_BUFFER, stream) != NULL) {
            this->stdOutlist.push_back(buffer);
        }
    }
    pclose(stream);
    return true;
}


bool smartcmdexc::parse_match_string(const char* pattern, std::string& val)
{
    std::string lstr;
    for(std::string& s:this->stdOutlist) {
        std::size_t found = s.find(pattern);
        if(found !=std::string::npos)	{
            lstr = s;
            break;
        }
    }

    size_t dpos=lstr.find_first_of(":");

    std::string value = lstr.substr(++dpos, std::string::npos);
    val = trim(value);
    return true;
}

std::string smartcmdexc::trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ')+1);
    return str;
}


bool smartcmdexc::reg_matches(const char *str, const char *pattern)
{
    regex_t re;

    if (regcomp(&re, pattern, REG_EXTENDED) != 0)
        return false;

    if(0 == regexec(&re, str, (size_t) 0, NULL, 0))
    {
        regfree(&re);
        return true;
    }

    return false;
}

bool smartcmdexc::isSMARTSupport(std::string &s)
{
    bool smart_enabled_ = false;
    const char *pattern1 = "SMART support is:[ \\t]*Enabled";
    const char *pattern2 = "SMART support is:[ \\t]*Disabled";

    if (reg_matches(s.c_str(), pattern1)) {
        smart_enabled_ = true;
    } else if (reg_matches(s.c_str(), pattern2)) {
        smart_enabled_ = false;
    }
    return smart_enabled_;
}

bool smartcmdexc::parse_capacity_string(std::string& s, long& capacity)
{
    size_t firstMatch=s.find_first_of("[");
    size_t lastMatch=s.find_first_of("]");
    ++firstMatch;
    std::string value = s.substr(firstMatch, (lastMatch - firstMatch));
    std::string::size_type sz;
    long hdd_capacity = std::stoi(value,&sz);
    if(hdd_capacity)
        capacity = (std::stoi(value,&sz))*1e+9;
    else
        return false;

    return true;
}


const std::string& smartcmdexc::getAtaStandard() {
    std::string ata_standard="";
    parse_match_string(SMARTMON_ATA_STD_STR , ata_standard);
    return ata_standard;
}

long smartcmdexc::getCapacity() {
    std::string capacity_str="";
    long capacity=0;

    if(parse_match_string(SMARTMON_CAPACITY_STR , capacity_str)) {
        parse_capacity_string(capacity_str, capacity);
    }
    return capacity;
}

const std::string& smartcmdexc::getFirmwareVersion() {
    std::string firmwareVersion="";
    parse_match_string(SMARTMON_FIRMWARE_VER_STR , firmwareVersion);
    return firmwareVersion;
}

bool smartcmdexc::isSmartSupport() {
    std::string smartSupported_str_val;
    bool smartSupport = false;
    if(parse_match_string(SMARTMON_SUPPORT_STR , smartSupported_str_val))
    {
        smartSupport = isSMARTSupport(smartSupported_str_val);
    }

    return smartSupport;
}

const std::string& smartcmdexc::getManufacture() {
    std::string manufacture="";
    parse_match_string(SMARTMON_MODEL_FAMILY_STR , manufacture);
    return manufacture;
}

const std::string& smartcmdexc::getModel() {
    std::string deviceModel= "";
    parse_match_string(SMARTMON_MODEL_STR , deviceModel);
    return deviceModel;
}

const std::string& smartcmdexc::getSerialNumber() {
    std::string serialNumber="";
    parse_match_string(SMARTMON_SERIAL_NUMBER_STR , serialNumber);
    return serialNumber;
}

#endif /* #ifdef ENABLE_SMARTMONTOOL_SUPPORT */
