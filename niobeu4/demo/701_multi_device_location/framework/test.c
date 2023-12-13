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
#include <stdio.h>
#include "wifi_service.h"
#include "multi_device_location_api.h"
#include "cmsis_os2.h"
#include "ohos_run.h"
#include "ohos_init.h"
#include "ohos_types.h"

#define STACK_SIZE 4096

void TestTask(const char *arg)
{
    WaitStaInitFinish();
    printf(" get location test------------------ --------------------------\n");
    Location locationTest = {0};
    GetDeviceLocation(&locationTest);
}

static void GetDeviceLocationEntry(void)
{
    osThreadAttr_t attr;
    attr.name = "testTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)testTask, NULL, &attr) == NULL) {
        printf("[MainTask] Falied to create MainTask!\n");
    }
}

SYS_FEATURE_INIT(GetDeviceLocationEntry);