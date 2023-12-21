/*
 * Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
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
#include "bluetooth_battery_example.h"
#include <stdio.h>
#include <stdlib.h>
#include "ohos_run.h"
#include "cmsis_os2.h"

#define TAG "BLE_BATTERY"

static uint8_t char1_str[] = { 0x11, 0x22, 0x33 };
static uint8_t a_property = 0;
static BleAttrValue gatts_demo_char1_val = {
    .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
    .attr_len = sizeof(char1_str),
    .attr_value = char1_str,
};

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    // second uuid, 32bit
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
// adv data
static BleAdvData adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, // connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // &test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (OHOS_BLE_ADV_FLAG_GEN_DISC | OHOS_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static BleAdvData scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // &test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (OHOS_BLE_ADV_FLAG_GEN_DISC | OHOS_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


/* Declare static functions */
static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
static void gatts_profile_a_event_handler(GattsBleCallbackEvent event,
    GattInterfaceType gatts_if, BleGattsParam* param);
/* 基于gatt的配置文件 */
static struct GattsProfileInst g_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = OHOS_GATT_IF_NONE, /* 没有获得gatt_if，所以初始值是OHOS_GATT_IF_NONE */
    },
};

static BleAdvParams adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t get_fake_battery_level()
{
    return 0x21;
}

static void gatts_reg_event(GattInterfaceType gatts_if, BleGattsParam* param)
{
    BT_LOGE("REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        g_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
        g_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
        g_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = 0x02;
        g_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;
        BtError ret = SetLocalName(TEST_DEVICE_NAME, sizeof(TEST_DEVICE_NAME));
    if (ret) {
        BT_LOGE("set device name failed. error code:%x\n", ret);
    }

    ret = BleGapConfigAdvData(&adv_data);
    if (ret) {
        BT_LOGE("config scan response data failed, error code = %x", ret);
    }
    BleGattsCreateService(gatts_if, &g_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
}

static void gatts_read_event(GattInterfaceType gatts_if, BleGattsParam* param)
{
    BT_LOGE("GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n",
        param->read.conn_id, param->read.trans_id, param->read.handle);
    BleGattRsp rsp;
    memset_s(&rsp, sizeof(BleGattRsp), 0, sizeof(BleGattRsp));
    rsp.attr_value.handle = param->read.handle;
    rsp.attr_value.len = 1;
    rsp.attr_value.value[0] = get_fake_battery_level();
    BtError ret = BleGattsSendResponse(gatts_if, param->read.conn_id, param->read.trans_id,
    OHOS_GATT_SUCCESS, &rsp);
    if (ret) {
        BT_LOGE("send response error code:%x\n", ret);
    }
}

static void gatts_create_event(GattInterfaceType gatts_if, BleGattsParam* param)
{
    BT_LOGE("CREATE_SERVICE_EVT, status %d, service_handle %d\n", param->create.status, param->create.service_handle);
    g_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
    g_profile_tab[PROFILE_A_APP_ID].char_uuid.len = 0x02;
    g_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;
            
    BleGattsStartService(g_profile_tab[PROFILE_A_APP_ID].conn_id, g_profile_tab[PROFILE_A_APP_ID].service_handle);
    a_property = OHOS_GATT_CHARACTER_PROPERTY_BIT_READ |
            OHOS_GATT_CHARACTER_PROPERTY_BIT_WRITE | OHOS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY;
    BtError add_char_ret = BleGattsAddChar(g_profile_tab[PROFILE_A_APP_ID].service_handle,
        &g_profile_tab[PROFILE_A_APP_ID].char_uuid,
        OHOS_GATT_PERMISSION_READ | OHOS_GATT_PERMISSION_WRITE,
        a_property,
        &gatts_demo_char1_val, NULL);
    if (add_char_ret) {
        BT_LOGE("add char failed, error code =%x", add_char_ret);
    }
}

static void gatts_add_char_event(GattInterfaceType gatts_if, BleGattsParam* param)
{
    uint16_t length = 0;
    const uint8_t* prf_char;

    BT_LOGE("ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d\n",
        param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
    g_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
    g_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = 0x02;
    g_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = OHOS_GATT_UUID_CHAR_CLIENT_CONFIG;
    BtError get_attr_ret = BleGattsGetAttrValue(param->add_char.attr_handle, &length, &prf_char);
    if (get_attr_ret == BT_ERROR) {
        BT_LOGE("ILLEGAL HANDLE");
    }

    BT_LOGE("the gatts demo char length = %x\n", length);
    for (int i = 0; i < length; i++) {
        BT_LOGE("prf_char[%x] =%x\n", i, prf_char[i]);
    }
    BtError add_descr_ret = BleGattsAddCharDescr(g_profile_tab[PROFILE_A_APP_ID].service_handle,
        &g_profile_tab[PROFILE_A_APP_ID].descr_uuid,
        OHOS_GATT_PERMISSION_READ | OHOS_GATT_PERMISSION_WRITE, NULL, NULL);
    if (add_descr_ret) {
        BT_LOGE("add char descr failed, error code =%x", add_descr_ret);
    }
}

static void gatts_profile_a_event_handler(GattsBleCallbackEvent event,
    GattInterfaceType gatts_if, BleGattsParam* param)
{
    switch (event) {
        case OHOS_GATTS_REG_EVT:
            gatts_reg_event(gatts_if, param);
            break;
        case OHOS_GATTS_READ_EVT:
            gatts_read_event(gatts_if, param);
            break;
        case OHOS_GATTS_WRITE_EVT:
            BT_LOGE("GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d",
                param->write.conn_id, param->write.trans_id, param->write.handle);
            break;
        case OHOS_GATTS_EXEC_WRITE_EVT:
            BleGattsSendResponse(gatts_if, param->write.conn_id, param->write.trans_id, OHOS_GATT_SUCCESS, NULL);
            break;
        case OHOS_GATTS_MTU_EVT:
            BT_LOGE("OHOS_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case OHOS_GATTS_CREATE_EVT:
            gatts_create_event(gatts_if, param);
            break;
        case OHOS_GATTS_ADD_CHAR_EVT:
            gatts_add_char_event(gatts_if, param);
            break;
        case OHOS_GATTS_ADD_CHAR_DESCR_EVT:
            g_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
            BT_LOGE("ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
            break;
        case OHOS_GATTS_START_EVT:
            BT_LOGE("SERVICE_START_EVT, status %d, service_handle %d\n",
                param->start.status, param->start.service_handle);
            break;
        case OHOS_GATTS_DISCONNECT_EVT:
            BT_LOGE("OHOS_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
            BleGattsStartAdvertising(&adv_params);
            break;
        case OHOS_GATTS_CONF_EVT:
            BT_LOGE("OHOS_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
            if (param->conf.status != OHOS_GATT_SUCCESS) {
                BT_LOGE("status is not OHOS_GATT_SUCCESS!\n");
            }
            break;
        default:
            break;
    }
}

static void gatts_event_handler(GattsBleCallbackEvent event, GattInterfaceType gatts_if, BleGattsParam* param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == OHOS_GATTS_REG_EVT) {
        if (param->reg.status == OHOS_GATT_SUCCESS) {
            g_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            BT_LOGE("Reg app failed, app_id %04x, status %d\n",
                param->reg.app_id,
                param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    int idx;
    for (idx = 0; idx < PROFILE_NUM; idx++) {
        if (gatts_if == OHOS_GATT_IF_NONE || gatts_if == g_profile_tab[idx].gatts_if) {
            if (g_profile_tab[idx].gatts_cb) {
                g_profile_tab[idx].gatts_cb(event, gatts_if, param);
            }
        }
    }
}

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    BT_LOGE("gap_cb event:%d\n", event);
    switch (event) {
        case OHOS_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            BleGattsStartAdvertising(&adv_params);
            break;
        case OHOS_GAP_BLE_ADV_START_COMPLETE_EVT:
            // adv start complete event to indicate adv start successfully or failed
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                BT_LOGE("adv start failed! \n");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                BT_LOGE("adv stop failed! \n");
            } else {
                BT_LOGE("adv stop successed!\n");
            }
            break;
        case OHOS_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            BT_LOGE("update connection params status = %d, min_int = %d",
                param->update_conn_params.status,
                param->update_conn_params.min_int);
            BT_LOGE("max_int = %d, conn_int = %d,latency = %d, timeout = %d",
                param->update_conn_params.max_int,
                param->update_conn_params.conn_int,
                param->update_conn_params.latency,
                param->update_conn_params.timeout);
            break;
        case OHOS_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            BleGattsStartAdvertising(&adv_params);
            break;
        default:
            break;
    }
}

void BluetoothBatteryExample(void)
{
    EnableBle();
    BtGattServerCallbacks btGattServerCallbacks = {
        .gap_callback = gap_cb,
        .gatts_callback = gatts_event_handler,
        .profileAppId = PROFILE_A_APP_ID,
    };
    BleGattsRegisterCallbacks(btGattServerCallbacks);
    return;
}

OHOS_APP_RUN(BluetoothBatteryExample);