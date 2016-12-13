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
#source $WORK_DIR/../build_scripts/setBCMenv.sh
source $WORK_DIR/../build_scripts/setBCMenv.sh

export APPLIBS_PATH=$APPLIBS_TARGET_DIR/usr/local/lib
export APPLIBS_INCLUDE=$APPLIBS_TARGET_DIR/usr/local/include
export IARM_PATH=$COMBINED_DIR/iarmbus
export IARM_MGRS=$COMBINED_DIR/iarmmgrs
export FUSION_DALE=$COMBINED_DIR/opensource/lib

#echo $DS_PATH
export CXX=mipsel-linux-g++
export CC=mipsel-linux-gcc
export AR=mipsel-linux-ar
export LD=mipsel-linux-ld
export NM=mipsel-linux-nm
export RANLIB=mipsel-linux-ranlib
export STRIP=mipsel-linux-strip

export PLATFORM_SDK=$REFSW_TOP/uclinux-rootfs
export ROOTFS=$WORK_DIR/rootfs

export LDFLAGS="-L. \
-L$PREFIX_FOLDER/lib \
-L${PLATFORM_SDK}/lib/uClibc \
-L${ROOTFS}/lib \
-L${ROOTFS}/usr/lib \
-L${ROOTFS}/usr/local/lib \
-L${PLATFORM_SDK}/usr/lib \
-L${PLATFORM_SDK}/usr/local/lib \
-L${PLATFORM_SDK}/lib \
-L$APP_PATH/lib  \
-L${PLATFORM_SDK}/lib/uClibc \
-lm -lintl -lz  -ldl"
# INCLUDE PATH FOR RDK COMPONENTS
export OPENSOURCE_INCLUDE="-I$COMBINED_DIR/opensource/include -I$COMBINED_DIR/opensource/include"
export IARM_INCLUDE="-I$IARM_PATH/core -I$IARM_PATH/core/include -I$DFB_ROOT/usr/local/include/directfb"
export GLIB_INCLUDE="-I$WORK_DIR/rootfs/usr/local/include/"
export REFS_INCLUDE="-I$WORK_DIR/Refsw/linux/include"

export CFLAGS+="$OPENSOURCE_INCLUDE $IARM_INCLUDE -I$APPLIBS_INCLUDE $GLIB_INCLUDE $REFS_INCLUDE"

export LDFLAGS="$LDFLAGS"

export CPPFLAGS="$CFLAGS `${ROOTFS}/usr/local/bin/libgcrypt-config --cflags` -UDEBUG_LOGS"
export LDFLAGS="$LDFLAGS `${ROOTFS}/usr/local/bin/libgcrypt-config --libs`"

export PKG_CONFIG_PATH="$OPENSOURCE_INCLUDE/../lib/pkgconfig/:$DFB_LIB/pkgconfig:$APPLIBS_TARGET_DIR/usr/local/lib/pkgconfig:${APPLIBS_DIR}/opensource/directfb/build/1.4.17/97428_linuxuser/lib/pkgconfig"

