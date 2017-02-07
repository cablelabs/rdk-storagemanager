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

//#ifdef ENABLE_SMARTMONTOOL_SUPPORT

#include "smartmonUtiles.h"

bool smartmonUtiles::execute()
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


bool smartmonUtiles::parse_match_string(const char* pattern, std::string& val)
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

std::string smartmonUtiles::trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ')+1);
    return str;
}


bool smartmonUtiles::reg_matches(const char *str, const char *pattern)
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

bool smartmonUtiles::matchSMARTSupportExpression(std::string &s)
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

bool smartmonUtiles::parse_capacity_string(std::string& s, long& capacity)
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


bool smartmonUtiles::getAtaStandard(std::string& ata_standard) {
    return parse_match_string(SMARTMON_ATA_STD_STR , ata_standard);
}

long smartmonUtiles::getCapacity() {
    std::string capacity_str;
    long capacity=0;

    if(parse_match_string(SMARTMON_CAPACITY_STR , capacity_str)) {
        parse_capacity_string(capacity_str, capacity);
    }
    return capacity;
}

bool smartmonUtiles::getFirmwareVersion(std::string& firmwareVersion) {
    return parse_match_string(SMARTMON_FIRMWARE_VER_STR , firmwareVersion);
}

bool smartmonUtiles::isSmartSupport() {
    std::string smartSupported_str_val;
    bool smartSupport = false;
    if(parse_match_string(SMARTMON_SUPPORT_STR , smartSupported_str_val))
    {
        smartSupport = matchSMARTSupportExpression(smartSupported_str_val);
    }

    return smartSupport;
}

bool smartmonUtiles::getManufacture(std::string& manufacture) {
    return (parse_match_string(SMARTMON_MODEL_FAMILY_STR , manufacture));
}

bool smartmonUtiles::getDeviceId(std::string& deviceId) {
    return (parse_match_string(SMARTMON_HDD_ID_STR, deviceId));
}

bool smartmonUtiles::getModel(std::string& deviceModel) {
    return parse_match_string(SMARTMON_MODEL_STR , deviceModel);
}

bool smartmonUtiles::getSerialNumber(std::string& serialNumber) {
    return parse_match_string(SMARTMON_SERIAL_NUMBER_STR , serialNumber);
}


bool smartmonUtiles::isOverallHealthOkay() {

    std::string health_str;

    if(parse_match_string(SMARTMON_OVERALL_HEALTH_STR, health_str))
    {
        const char *temp = "PASSED";
        if(0 == strncasecmp(health_str.c_str(), temp, strlen(temp)))
        {
            return true;
        }
    }
    return false;
}

bool smartmonUtiles::getDiagnosticAttributes(std::map<std::string, std::string>& attrMap)
{
    int startIndex = 0;
    std::vector<std::string> list = getStdOutlist();

    if(list.empty())
    {
//		std::cout << "Empty Diagnostic Attributes." << std::endl;
        return false;
    }

    int cnt = 0;
    int len = 0;
    for (std::string& s:list)
    {
        if(s.length() > 1)
        {
            cnt++;
            std::size_t found = s.find("RAW_VALUE");
            if(found !=std::string::npos)	{
                startIndex = cnt;
            }

            if(cnt > startIndex && startIndex)
            {
                std::istringstream iss(s);
                int index = 0;
                std::string key;
                std::string value;
                while (iss) {
                    ++index;
                    std::string word;
                    iss >> word;
                    if(index > 1 && index == 2) {
                        if(!word.empty())
                            key = word;
                    }
                    else if (index > 1 && index != 2) {
                        if(value.empty())
                            value = word;
                        else
                            value = value + ";" + word;
                    }
                }
                attrMap.insert(std::make_pair(key, value));
            }
        }
    }
    return true;
}

//#endif /* #ifdef ENABLE_SMARTMONTOOL_SUPPORT */
