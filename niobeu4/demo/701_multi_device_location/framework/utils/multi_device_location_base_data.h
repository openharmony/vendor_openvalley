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

#ifndef MULTI_DEVICE_LOCATION_BASE_DATA_H
#define MULTI_DEVICE_LOCATION_BASE_DATA_H
#define MAX_DEVICE_ID_LENTH 32
#include <stdint.h>
typedef struct {
    float x;
    float y;
} Location;

typedef struct {
    char deviceId[MAX_DEVICE_ID_LENTH + 1];
    int32_t rssi;
} DeviceRssiInfo;

#endif