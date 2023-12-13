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

#ifndef PACKAGE_REQUEST_DATA_H
#define PACKAGE_REQUEST_DATA_H
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "cmsis_os2.h"
#include "ohos_init.h"
#include "ohos_run.h"
#include "string.h"
#include "http.h"
#include "http_application_api.h"
#include "wifi_service.h"
#include "multi_device_location_base_data.h"

#define DEVICE_LOCATION_ROW_NUMBER 3
#define DEVICE_LOCATION_COL_NUMBER 64

// 组包RSSI元数据
// 入参为已知DeviceRssiInfo的数据信息，返回值值为组包字符串，申请内存需要手动释放
char *PackageRssiPostMeteData(DeviceRssiInfo *deviceRssiInfo, size_t deviceSize);

// post请求
int PostRequestTask(char *postbuf, size_t bufsize, char *reponseBuf, size_t reponseBufSize, const char *url);

// 解析reponse数据，获取rssi值 入参reponse数据和大小
int ParseReponseRssiData(char *buf, size_t size, Location *result);

// 组包获取目标设备设备, 发送本地设备ID，获取配置列表
char *PackageTargertDeviceId(char *localDeviceId);

// 解包获取目标设备
char **ParseTargetDeviceId(char *buf, size_t size);

#endif