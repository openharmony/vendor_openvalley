/*
 * Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "multi_device_location_api.h"
#include "multi_device_location_base_data.h"
#include "multi_device_location_ble_control.h"
#include "multi_device_location_error.h"
#include "cmsis_os2.h"
#include "ohos_run.h"
#include "securec.h"

#define RESPONSE_BUF_SIZE 256
#define DEVICE_LOCATION_MSG_COUNT 5
#define DEVICE_LOCATION_MSG_SIZE 16
#define HTTP_POST_RESPONSE_BUFF_SIZE 454
#define DEVICE_NAME_LENTH 6
#define HTTP_REQUEST_BUFF_SIZE 32
#define PER_SECOND_CONUT 1000000
#define DEVICE_NUM 3
#define STACK_SIZE 4096

#define LOCATION_POST_URL "http://ovc2c2.talkweb.com.cn/license-console/v1/algorithm/three_points_location"
#define CONFIG_POST_URL "http://ovc2c2.talkweb.com.cn/license-console/v1/device/list_of_place"

static osMessageQueueId_t g_DeviceLocationMsg = NULL;
static osMessageQueueId_t g_DeviceLocationCmdMsg = NULL;

uint32_t GetDeviceLocation(Location *location)
{
    printf("GetDeviceLocation entry-------------------------------\n");
    if (g_DeviceLocationCmdMsg == NULL) {
        return MDL_OS_ERROR;
    }
    int code = 0;
    osStatus_t status = osMessageQueuePut(g_DeviceLocationCmdMsg, &code, 0, osWaitForever);
    if (status != osOK) {
        printf("GetDeviceLocation osMessageQueuePut failed\n");
        return MDL_OS_ERROR;
    }
    if (g_DeviceLocationMsg == NULL) {
        return MDL_OS_ERROR;
    }
    osMessageQueueGet(g_DeviceLocationMsg, location, 0, 0);

    return MDL_SUCCESS;
}

uint32_t GetDeviceLocationImpl(Location *location)
{
    printf("GetDeviceLocationImpl entry-----------------------------\n");
    if (location == NULL) {
        return MDL_PARAM_ERROR;
    }

    // get device
    char locaDevice[DEVICE_NAME_LENTH + 1] = {'1', '0', '0', '1', '\0'};
    printf("before getLocalAddr ---------------\n");
    printf("GetLocalAddr success-------------\n");

    char *requestBuf = PackageTargertDeviceId(locaDevice);
    printf("PackageTargertDeviceId success----------------------\n");
    char *responseBuf = (char *)malloc(HTTP_POST_RESPONSE_BUFF_SIZE);
    if (responseBuf == NULL) {
        return MDL_OS_ERROR;
    }
    memset_s(responseBuf, HTTP_POST_RESPONSE_BUFF_SIZE, 0, HTTP_POST_RESPONSE_BUFF_SIZE);
    printf("memset success ---------------------\n");

    PostRequestTask(requestBuf, HTTP_REQUEST_BUFF_SIZE, responseBuf, HTTP_POST_RESPONSE_BUFF_SIZE, CONFIG_POST_URL);
    printf("PostRequestTask success------------------\n");
    char **deviceIdS = ParseTargetDeviceId(responseBuf, HTTP_POST_RESPONSE_BUFF_SIZE);

    if (deviceIdS == NULL) {
        printf("%s Deviceids is NULL\n", __func__);
        return MDL_DEVICE_ERROR;
    }
    // device control get correspond RSSI number
    DeviceRssiInfo deviceRssiInfo[DEVICE_NUM] = {0};
    for (int i = 0; i < DEVICE_NUM; i++) {
        memcpy_s(deviceRssiInfo[i].deviceId, MAX_DEVICE_ID_LENTH, deviceIdS[i], strlen(deviceIdS[i]));
    }

    GetDeviceRssiByBle(deviceRssiInfo, DEVICE_NUM);

    char *rssiResponseBuf = (char *)malloc(RESPONSE_BUF_SIZE);
    if (rssiResponseBuf == NULL) {
        return MDL_OS_ERROR;
    }
    memset_s(rssiResponseBuf, RESPONSE_BUF_SIZE, 0, RESPONSE_BUF_SIZE);

    char *rssiPostBuf = PackageRssiPostMeteData(deviceRssiInfo, DEVICE_NUM);
    PostRequestTask(rssiPostBuf, strlen(rssiPostBuf), rssiResponseBuf, RESPONSE_BUF_SIZE, LOCATION_POST_URL);

    ParseReponseRssiData(rssiResponseBuf, RESPONSE_BUF_SIZE, location);

    return MDL_SUCCESS;
}

void MainTask(const char *arg)
{
    printf("MainTask Entry\n");
    g_DeviceLocationMsg = osMessageQueueNew(DEVICE_LOCATION_MSG_COUNT, DEVICE_LOCATION_MSG_COUNT, NULL);
    if (g_DeviceLocationMsg == NULL) {
        printf("g_DeviceLocationMsg is nullptr\n");
        return;
    }
    g_DeviceLocationCmdMsg = osMessageQueueNew(DEVICE_LOCATION_MSG_COUNT, DEVICE_LOCATION_MSG_SIZE, NULL);
    if (g_DeviceLocationCmdMsg == NULL) {
        printf("g_DeviceLocationCmdMsg is nullptr\n");
        return;
    }

    while (1) {
        int code = 0;
        osStatus_t status = osMessageQueueGet(g_DeviceLocationCmdMsg, &code, NULL, osWaitForever);
        if (status != osOK) {
            printf("osMessageQueueGet failed, error code = %d\n", status);
            usleep(PER_SECOND_CONUT);
            continue;
        }
        Location location = {0};
        GetDeviceLocationImpl(&location);
        osMessageQueuePut(g_DeviceLocationMsg, (void *)&location, 0, 0);
    }
}

void GetDeviceLocationEntry(void)
{
    osThreadAttr_t attr;
    attr.name = "multi_device_location";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)MainTask, NULL, &attr) == NULL) {
        printf("[MainTask] Falied to create MainTask!\n");
    }
}

OHOS_APP_RUN(GetDeviceLocationEntry);