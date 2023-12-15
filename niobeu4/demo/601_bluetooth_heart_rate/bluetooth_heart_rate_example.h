/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BLUETOOTH_HEART_RATE_EXAMPLE_H
#define BLUETOOTH_HEART_RATE_EXAMPLE_H
#include "stdbool.h"
#include "ohos_bt_def.h"
#include "ohos_bt_gatt.h"
#include "bluetooth_service.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define GATTS_SERVICE_UUID_TEST_A 0x00FF
#define GATTS_CHAR_UUID_TEST_A 0xFF01
#define GATTS_DESCR_UUID_TEST_A 0x3333
#define GATTS_NUM_HANDLE_TEST_A 4
#define TEST_DEVICE_NAME "ble_heart_rate"
#define TEST_MANUFACTURER_DATA_LEN 17
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40
#define PREPARE_BUF_MAX_SIZE 1024
#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0

struct GattsProfileInst {
    GattsBleCallback gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    GattSrvcId service_id;
    uint16_t char_handle;
    BtUuids char_uuid;
    uint16_t perm;
    uint8_t property;
    uint16_t descr_handle;
    BtUuids descr_uuid;
};

typedef struct {
    uint8_t *prepareBuf;
    int prepareLen;
} PrepareEnvType;

#endif /* BLUETOOTH_HEART_RATE_EXAMPLE_H */
