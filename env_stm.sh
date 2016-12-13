#!/bin/sh
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
export RDK_RAMDISK_PATH=$RDK_PROJECT_ROOT_PATH/sdk/fsroot/ramdisk
#setup sdk environment variables
export TOOLCHAIN_NAME=`find ${RDK_TOOLCHAIN_PATH} -name environment-setup-* | sed -r 's#.*environment-setup-##'`
source $RDK_TOOLCHAIN_PATH/environment-setup-${TOOLCHAIN_NAME}

export ARCH=armv7
TARGETFS=$OECORE_TARGET_SYSROOT
export TARGETFS

export RDK_SYSROOTS_PATH="${RDK_TOOLCHAIN_PATH}/sysroots"
export TOOLCHAIN_NAME=`find ${RDK_TOOLCHAIN_PATH} -name environment-setup-* | sed -r 's#.*environment-setup-##'`
export HOSTBINDIR=`find ${RDK_SYSROOTS_PATH}/x86_64-oesdk-linux/usr/bin -maxdepth 1 -name "*linux-gnueabi*" -print`
export PATH=${RDK_SYSROOTS_PATH}/x86_64-oesdk-linux/usr/bin:${HOSTBINDIR}:$PATH

# INCLUDE PATH FOR RDK COMPONENTS
export RDK_TARGET_SYSROOT_DIR=${RDK_SYSROOTS_PATH}/${TOOLCHAIN_NAME}
export RDK_FSROOT_PATH=$RDK_TARGET_SYSROOT_DIR
export RDK_RAMDISK_PATH=$RDK_PROJECT_ROOT_PATH/sdk/fsroot/ramdisk
TARGETFS=$OECORE_TARGET_SYSROOT
export TARGETFS
export IARM_PATH=$COMBINED_DIR/iarmbus
export IARM_MGRS=$COMBINED_DIR/iarmmgrs
export DFB_ROOT=$RDK_FSROOT_PATH

export OPENSOURCE_INCLUDE="-I$COMBINED_DIR/opensource/include -I$COMBINED_DIR/opensource/include"
export IARM_INCLUDE="-I$IARM_PATH/core -I$IARM_PATH/core/include -I$DFB_ROOT/usr/include/directfb"
export GLIB_INCLUDE="-I${TARGETFS}/usr/local/include -I${TARGETFS}/usr/include -I${RDK_RAMDISK_PATH}/include -I${RDK_RAMDISK_PATH}/usr/include"
export GLIB_LDFLAGS="-L${RDK_FSROOT_PATH}/usr/lib -L${RDK_FSROOT_PATH}/usr/local/lib -L${RDK_RAMDISK_PATH}/usr/lib -L${RDK_RAMDISK_PATH}/usr/local/lib"
export ROOTFS=${RDK_RAMDISK_PATH}

export CFLAGS+=" $OPENSOURCE_INCLUDE $IARM_INCLUDE $GLIB_INCLUDE"

export LDFLAGS="$LDFLAGS `$(shell pkg-config --libs gthread-2.0)`"

export CPPFLAGS="$CFLAGS `${ROOTFS}/usr/local/bin/libgcrypt-config --cflags` -UDEBUG_LOGS"
export LDFLAGS="$LDFLAGS -L${RDK_FSROOT_PATH}/usr/lib -L${RDK_FSROOT_PATH}/usr/local/lib -L${RDK_RAMDISK_PATH}/usr/lib -L${RDK_RAMDISK_PATH}/usr/local/lib -ldbus-1"

