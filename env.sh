#!/bin/bash
##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#
#export COMCAST_PLATFORM=XI3

echo "WORK_DIR: =>" $WORK_DIR
# Set Broadcom Environment Variables
source $WORK_DIR/../build_scripts/setBCMenv.sh

#echo $DS_PATH
export CXX=mipsel-linux-g++
export CC=mipsel-linux-gcc
export AR=mipsel-linux-ar
export LD=mipsel-linux-ld
export NM=mipsel-linux-nm
export RANLIB=mipsel-linux-ranlib
export STRIP=mipsel-linux-strip


