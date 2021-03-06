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
#include "rdkStorageMgr.h"
#include "rdkStorageMgrTypes.h"
#include "rdkStorageMgrLogger.h"
#include "rdkStorageMgr_iarm_private.h"

void stmgr_eventCallback (eSTMGREventMessage events)
{
    eSTMGREventMessage eventData;
    memcpy (&eventData, &events, sizeof(eSTMGREventMessage));

    if (eventData.m_eventType == RDK_STMGR_EVENT_STATUS_CHANGED)
    {
        STMGRLOG_WARN ("Post RDK_STMGR_EVENT_STATUS_CHANGED update\n");
        IARM_Bus_BroadcastEvent(IARM_BUS_STMGR_NAME, (IARM_EventId_t) RDK_STMGR_IARM_EVENT_STATUS_CHANGED, (void *)&eventData, sizeof(eventData));
    }
    else if (eventData.m_eventType == RDK_STMGR_EVENT_TSB_ERROR)
    {
        STMGRLOG_WARN ("Post RDK_STMGR_EVENT_TSB_ERROR event\n");
        IARM_Bus_BroadcastEvent(IARM_BUS_STMGR_NAME, (IARM_EventId_t) RDK_STMGR_IARM_EVENT_TSB_ERROR, (void *)&eventData, sizeof(eventData));
    }
    else if (eventData.m_eventType == RDK_STMGR_EVENT_HEALTH_WARNING)
    {
        STMGRLOG_WARN ("Post RDK_STMGR_EVENT_HEALTH_WARNING event\n");
        IARM_Bus_BroadcastEvent(IARM_BUS_STMGR_NAME, (IARM_EventId_t) RDK_STMGR_IARM_EVENT_HEALTH_WARNING, (void *)&eventData, sizeof(eventData));
    }
    else if (eventData.m_eventType == RDK_STMGR_EVENT_DEVICE_FAILURE)
    {
        STMGRLOG_WARN ("Post RDK_STMGR_EVENT_DEVICE_FAILURE event\n");
        IARM_Bus_BroadcastEvent(IARM_BUS_STMGR_NAME, (IARM_EventId_t) RDK_STMGR_IARM_EVENT_DEVICE_FAILURE, (void *)&eventData, sizeof(eventData));
    }

    return;
}


static IARM_Result_t _GetDeviceIDs(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRDeviceIDs* pDeviceIDs = (eSTMGRDeviceIDs*) arg;

    if (!pDeviceIDs)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getDeviceIds(pDeviceIDs);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetDeviceInfo(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRDeviceInfo_iarm_t *pDeviceInfo = (eSTMGRDeviceInfo_iarm_t*) arg;

    if (!pDeviceInfo)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getDeviceInfo(pDeviceInfo->m_deviceID, &pDeviceInfo->m_deviceInfo);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetDeviceInfoList(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRDeviceInfoList *pDeviceInfoList = (eSTMGRDeviceInfoList*) arg;

    if (!pDeviceInfoList)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getDeviceInfoList(pDeviceInfoList);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetPartitionInfo(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRPartitionInfo_iarm_t *pPartitionInfo = (eSTMGRPartitionInfo_iarm_t*) arg;

    if (!pPartitionInfo)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getPartitionInfo (pPartitionInfo->m_deviceID, pPartitionInfo->m_partitionID, &pPartitionInfo->m_partitionInfo);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBStatus(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRTSBStatus *pTSBStatus = (eSTMGRTSBStatus*) arg;

    if (!pTSBStatus)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBStatus (pTSBStatus);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _SetTSBMaxMinutes(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned int *pMinutes = (unsigned int*) arg;

    if (!pMinutes)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_setTSBMaxMinutes (*pMinutes);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBMaxMinutes(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned int *pMinutes = (unsigned int*) arg;

    if (!pMinutes)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBMaxMinutes (pMinutes);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBCapacityInMinutes(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned int *pMinutes = (unsigned int*) arg;

    if (!pMinutes)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBCapacityMinutes(pMinutes);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBCapacity(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned long long *pCapacity = (unsigned long long*) arg;

    if (!pCapacity)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBCapacity(pCapacity);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBFreeSpace(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned long long *pFreeSpace = (unsigned long long*) arg;

    if (!pFreeSpace)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBFreeSpace(pFreeSpace);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetDVRCapacity(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned long long *pCapacity = (unsigned long long*) arg;

    if (!pCapacity)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getDVRCapacity(pCapacity);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetDVRFreeSpace(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    unsigned long long *pFreeSpace = (unsigned long long*) arg;

    if (!pFreeSpace)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getDVRFreeSpace(pFreeSpace);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetIsTSBEnabled(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    bool *pIsEnabled = (bool*) arg;

    if (!pIsEnabled)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        *pIsEnabled = rdkStorage_isTSBEnabled();
    }
    return rc;
}

static IARM_Result_t _SetIsTSBEnabled(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    bool *pIsEnabled = (bool*) arg;

    if (!pIsEnabled)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_setTSBEnabled (*pIsEnabled);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetIsDVREnabled(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    bool *pIsEnabled = (bool*) arg;

    if (!pIsEnabled)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        *pIsEnabled = rdkStorage_isDVREnabled();
    }
    return rc;
}

static IARM_Result_t _SetIsDVREnabled(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    bool *pIsEnabled = (bool*) arg;

    if (!pIsEnabled)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_setDVREnabled (*pIsEnabled);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetHealthInfo(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRHealthInfo_iarm_t *pHealthInfo = (eSTMGRHealthInfo_iarm_t*) arg;

    if (!pHealthInfo)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getHealth (pHealthInfo->m_deviceID, &pHealthInfo->m_healthInfo);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _GetTSBPartitionMountPath (void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    char *pMountPath = (char*) arg;

    if (!pMountPath)
    {
        rc = IARM_RESULT_INVALID_PARAM;
        STMGRLOG_ERROR ("%s : Invalid input passed\n", __FUNCTION__);
    }
    else
    {
        eSTMGRReturns retCode = rdkStorage_getTSBPartitionMountPath(pMountPath);
        if (retCode != RDK_STMGR_RETURN_SUCCESS)
        {
            STMGRLOG_ERROR ("%s : failed\n", __FUNCTION__);
            rc = IARM_RESULT_IPCCORE_FAIL; /* We do not have other IARM Error code to describe this. */
        }
    }
    return rc;
}

static IARM_Result_t _NotifyMGRAboutFailure(void *arg)
{
    IARM_Result_t rc = IARM_RESULT_SUCCESS;
    eSTMGRErrorEvent *pFailEvent = (eSTMGRErrorEvent*) arg;

    if (!pFailEvent)
    {
        rdkStorage_notifyMGRAboutFailure (*pFailEvent);
    }
    return rc;
}

void stmgr_BeginIARMMode()
{
    static bool m_isInited = false;
    if (!m_isInited)
    {
        m_isInited = true;
        IARM_Bus_Init(IARM_BUS_STMGR_NAME);
        IARM_Bus_Connect();

        STMGRLOG_INFO ("Entering %s\n", __FUNCTION__);

        IARM_Bus_RegisterCall ("GetDeviceIDs", _GetDeviceIDs);
        IARM_Bus_RegisterCall ("GetDeviceInfo", _GetDeviceInfo);
        IARM_Bus_RegisterCall ("GetDeviceInfoList", _GetDeviceInfoList);
        IARM_Bus_RegisterCall ("GetPartitionInfo", _GetPartitionInfo);
        IARM_Bus_RegisterCall ("GetTSBStatus", _GetTSBStatus);
        IARM_Bus_RegisterCall ("SetTSBMaxMinutes", _SetTSBMaxMinutes);
        IARM_Bus_RegisterCall ("GetTSBMaxMinutes", _GetTSBMaxMinutes);
        IARM_Bus_RegisterCall ("GetTSBCapacityInMinutes", _GetTSBCapacityInMinutes);
        IARM_Bus_RegisterCall ("GetTSBCapacity", _GetTSBCapacity);
        IARM_Bus_RegisterCall ("GetTSBFreeSpace", _GetTSBFreeSpace);
        IARM_Bus_RegisterCall ("GetDVRCapacity", _GetDVRCapacity);
        IARM_Bus_RegisterCall ("GetDVRFreeSpace", _GetDVRFreeSpace);
        IARM_Bus_RegisterCall ("GetIsTSBEnabled", _GetIsTSBEnabled);
        IARM_Bus_RegisterCall ("SetIsTSBEnabled", _SetIsTSBEnabled);
        IARM_Bus_RegisterCall ("GetIsDVREnabled", _GetIsDVREnabled);
        IARM_Bus_RegisterCall ("SetIsDVREnabled", _SetIsDVREnabled);
        IARM_Bus_RegisterCall ("GetHealthInfo", _GetHealthInfo);
        IARM_Bus_RegisterCall ("GetTSBPartitionMountPath", _GetTSBPartitionMountPath);
        IARM_Bus_RegisterCall ("NotifyMGRAboutFailure", _NotifyMGRAboutFailure);

        IARM_Bus_RegisterEvent(RDK_STMGR_IARM_EVENT_MAX);

        /* Register callback */
        rdkStorage_RegisterEventCallback(stmgr_eventCallback);

        STMGRLOG_INFO ("IARM Interface Inited Successfully\n");
    }
    else
        STMGRLOG_INFO ("IARM Interface Already Inited\n");

    return;
}
