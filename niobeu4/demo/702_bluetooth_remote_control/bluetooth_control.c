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
#include "ohos_run.h"
#include "cmsis_os2.h"
#include "stdbool.h"
#include "ohos_bt_def.h"
#include "ohos_bt_gatt.h"
#include "blemasterkits.h"
#include "securec.h"
#include "gpio_types.h"
#include "gpio_if.h"

#define REMOTE_SERVICE_UUID 0x00FF
#define REMOTE_NOTIFY_CHAR_UUID 0xFF01
#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define INVALID_HANDLE 0

#define RSSI_MSG_SIZE 256
#define RSSI_MSG_COUNT 5
#define MTU_SIZE 500
#define WRITE_DATA_SIZE 4
#define RED_LED_PIN_INDEX 0
#define PWR_SW_PIN_INDEX 2 // VCC_3.3 电源使能
#define ON 1
#define OFF 0
#define SYS_DELAY_TICKS 200

static void GpioInit(void)
{
    GpioSetDir(PWR_SW_PIN_INDEX, GPIO_DIR_OUT); // 设置VCC_3.3电源使能
    GpioWrite(PWR_SW_PIN_INDEX, ON);

    GpioSetDir(RED_LED_PIN_INDEX, GPIO_DIR_OUT);
}

static void GpioMessageHandler(char message)
{
    if (message == '1') {
        GpioWrite(RED_LED_PIN_INDEX, ON);
    } else {
        GpioWrite(RED_LED_PIN_INDEX, OFF);
    }

    osDelay(SYS_DELAY_TICKS);
}

static osMessageQueueId_t g_RssiStatusMsg = NULL;

struct GattcProfileInst {
    GattcBleCallback gattcCb;
    uint16_t gattcIf;
    uint16_t appId;
    uint16_t connId;
    uint16_t serviceStartHandle;
    uint16_t serviceEndHandle;
    uint16_t charHandle;
    BdAddrs remoteBda;
};

uint32_t GetDeviceRssiByBle(DeviceRssiInfo *deviceRssiInfos, size_t len)
{
    // open ble device for scan

    BleStartScan();
    for (int i = 0; i < len; i++) {
        esp_bd_addr_t remote_addr = {0};
        memcpy_s(remote_addr, ESP_BD_ADDR_LEN, deviceRssiInfos[i].deviceId, sizeof(remote_addr));
        ReadRssiFromDevice(remote_addr);
    }

    // block read from message queue
    DeviceRssiInfo deviceRssiInfo = {0};
    if (g_RssiStatusMsg != NULL) {
        osStatus_t status = osMessageQueueGet(g_RssiStatusMsg, (void *)&deviceRssiInfo, NULL, osWaitForever);
    } else {
        BleStopScan();
        return MDL_OS_ERROR;
    }
    BleStopScan();

    return MDL_SUCCESS;
}

int32_t ReadRssiFromDevice(esp_bd_addr_t remote_addr)
{
    return esp_ble_gap_read_rssi(remote_addr);
}

static const char REMOTE_DEVICE_NAME[] = "BLE_DEMO";
static bool g_connect = false;
static bool g_getServer = false;
static BleGattcCharElem *char_elem_result = NULL;
static BleGattcDescrElem *descr_elem_result = NULL;

/* Declare static functions */
static void gap_cb(GapSearchEvent event, BleGapParam *param);
static void gattc_cb(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param);
static void gattc_profile_event_handler(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param);

uint8_t g_directAddr[6] = {0x3c, 0x61, 0x05, 0x7d, 0xdf, 0x66};

static BtUuid remote_filter_service_uuid = {
    .uuidLen = 2,
    .uuid = REMOTE_SERVICE_UUID,
};

static BtUuids remote_filter_char_uuid = {
    .len = 2,
    .uuid = {
        .uuid16 = REMOTE_NOTIFY_CHAR_UUID,
    },
};

static BtUuids notify_descr_uuid = {
    .len = 2,
    .uuid = {
        .uuid16 = OHOS_GATT_UUID_CHAR_CLIENT_CONFIG,
    },
};

static BleScanParams ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x0500,
    .scan_window = 0x0050,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

/* One gatt-based profile one app_id and one gattc_if,
this array will store the gattc_if returned by OHOS_GATTS_REG_EVT */
static struct GattcProfileInst g_profileTab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattcCb = gattc_event_handler,
        .gattcIf = OHOS_GATT_IF_NONE, /* Not get the gatt_if, so initial is OHOS_GATT_IF_NONE */
    },
};

static void gattc_reg_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    BT_LOGE("REG_EVT");
    BtError scan_ret = BleGatSetScanParams(&ble_scan_params);
    if (scan_ret) {
        BT_LOGE("set scan params error, error code = %x", scan_ret);
    }
}

static void open_evt(BleGattcParam *p_data, BleGattcParam *param)
{
    if (param->open.status != BT_SUCCESS) {
        BT_LOGE("open failed, status %d", p_data->open.status);
        return;
    }
    BT_LOGE("open success");
}

static void connect_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    BT_LOGE("OHOS_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
    g_profileTab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
    memcpy_s(g_profileTab[PROFILE_A_APP_ID].remote_bda.addr,
             sizeof(g_profileTab[PROFILE_A_APP_ID].remote_bda.addr),
             p_data->connect.remote_bda, sizeof(p_data->connect.remote_bda));
    BT_LOGE("REMOTE BDA:");
    BtError mtu_ret = BleGattcSendMtuReq(gattc_if, p_data->connect.conn_id);
    if (mtu_ret) {
        BT_LOGE("config MTU error, error code = %x", mtu_ret);
    }
}

static void dis_srvccmpl_evt(GattInterfaceType gattc_if, BleGattcParam *param)
{
    if (param->dis_srvc_cmpl.status != BT_SUCCESS) {
        BT_LOGE("discover service failed, status %d", param->dis_srvc_cmpl.status);
        return;
    }
    BT_LOGE("discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
    BleGattcSearchServices(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
}

static void cfg_mtu_evt(BleGattcParam *param)
{
    if (param->cfg_mtu.status != BT_SUCCESS) {
        BT_LOGE("config mtu failed, error status = %x", param->cfg_mtu.status);
    }
    BT_LOGE("OHOS_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d",
            param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
}

static void search_res_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    BT_LOGE("SEARCH RES: conn_id = %x is primary service %d",
            p_data->search_res.conn_id, p_data->search_res.is_primary);
    BT_LOGE("start handle %d end handle %d current handle value %d",
            p_data->search_res.start_handle, p_data->search_res.end_handle,
            p_data->search_res.srvc_id.inst_id);
    if (p_data->search_res.srvc_id.uuid.len == UUID16_BIT &&
        p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
        BT_LOGE("service found");
        get_server = true;
        g_profileTab[PROFILE_A_APP_ID].serviceStartHandle = p_data->search_res.start_handle;
        g_profileTab[PROFILE_A_APP_ID].serviceEndHandle = p_data->search_res.end_handle;
        BT_LOGE("UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
    }
}

static void evt_get_server(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    uint16_t count = 0;
    GattcGetAttr get_attr = {
        .gattc_if = gattc_if,
        .conn_id = p_data->search_cmpl.conn_id,
        .type = OHOS_GATT_DB_CHARACTERISTIC,
        .start_handle = g_profileTab[PROFILE_A_APP_ID].serviceStartHandle,
        .end_handle = g_profileTab[PROFILE_A_APP_ID].serviceEndHandle,
    };
    GattStatus status = BleGattcGetAttrCount(get_attr,
                                             INVALID_HANDLE,
                                             &count);
    if (status != BT_SUCCESS) {
        BT_LOGE("GattcGetAttrCount error");
    }

    if (count > 0) {
        char_elem_result = (BleGattcDescrElem *)malloc(sizeof(BleGattcDescrElem) * count);
        if (!char_elem_result) {
            BT_LOGE("gattc no mem");
        } else {
            GattcGetChar get_char = {
                .gattc_if = gattc_if,
                .conn_id = p_data->search_cmpl.conn_id,
                .start_handle = g_profileTab[PROFILE_A_APP_ID].serviceStartHandle,
                .end_handle = g_profileTab[PROFILE_A_APP_ID].serviceEndHandle,
            };
            status = BleGattcGetCharByUuid(get_char,
                                           remote_filter_char_uuid,
                                           char_elem_result,
                                           &count);
            if (status != BT_SUCCESS) {
                BT_LOGE("BleGattcGetCharByUuid error");
            }

            if (count > 0 && (char_elem_result[0].properties & OHOS_GATT_CHAR_PROP_BIT_NOTIFY)) {
                g_profileTab[PROFILE_A_APP_ID].charHandle = char_elem_result[0].char_handle;
                BleGattcRegisterForNotify(gattc_if, g_profileTab[PROFILE_A_APP_ID].remoteBda,
                                          char_elem_result[0].char_handle);
            }
            free(char_elem_result);
            char_elem_result = NULL;
        }
    } else {
        BT_LOGE("no char found");
    }
}

static void search_cmpl_event(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    if (p_data->search_cmpl.status != BT_SUCCESS) {
        BT_LOGE("search service failed, error status = %x", p_data->search_cmpl.status);
        return;
    }
    if (p_data->search_cmpl.searched_service_source == OHOS_GATT_SERVICE_FROM_REMOTE_DEVICE) {
        BT_LOGE("Get service information from remote device");
    } else if (p_data->search_cmpl.searched_service_source == OHOS_GATT_SERVICE_FROM_NVS_FLASH) {
        BT_LOGE("Get service information from flash");
    } else {
        BT_LOGE("unknown service source");
    }
    BT_LOGE("OHOS_GATTC_SEARCH_CMPL_EVT");
    if (get_server) {
        evt_get_server(gattc_if, p_data);
    }
}

static GattStatus notify_handle_attr(GattInterfaceType gattc_if, uint16_t count)
{
    GattcGetAttr get_attr = {
        .gattc_if = gattc_if,
        .conn_id = g_profileTab[PROFILE_A_APP_ID].connId,
        .type = OHOS_GATT_DB_DESCRIPTOR,
        .start_handle = g_profileTab[PROFILE_A_APP_ID].serviceStartHandle,
        .end_handle = g_profileTab[PROFILE_A_APP_ID].serviceEndHandle,
    };
    return BleGattcGetAttrCount(get_attr,
                                g_profileTab[PROFILE_A_APP_ID].charHandle,
                                &count);
}

static void act_notify_handle(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    uint16_t count = 0;
    uint16_t notifyEn = 1;
    GattStatus ret_status = notify_handle_attr(gattc_if, count);
    if (ret_status != BT_SUCCESS) {
        BT_LOGE("BleGattcGetAttrCount error");
    }
    if (count > 0) {
        descr_elem_result = malloc(sizeof(BleGattcDescrElem) * count);
        if (!descr_elem_result) {
            BT_LOGE("malloc error, gattc no mem");
        } else {
            GattcGetDescr get_descr = {
                .gattc_if = gattc_if,
                .conn_id = g_profileTab[PROFILE_A_APP_ID].connId,
                .char_handle = p_data->reg_for_notify.handle,
                .descr_uuid = descr_elem_result,
            };
            ret_status = BleGattcGetDescrByCharHandle(get_descr,
                                                      descr_elem_result,
                                                      &count);
            if (ret_status != BT_SUCCESS) {
                BT_LOGE("BleGattcGetDescrByCharHandle error");
            }
            if (count > 0 && descr_elem_result[0].uuid.len == UUID16_BIT &&
                descr_elem_result[0].uuid.uuid.uuid16 == OHOS_GATT_UUID_CHAR_CLIENT_CONFIG) {
                GattcWriteChar write_char = {
                    .gattc_if = gattc_if,
                    .conn_id = g_profileTab[PROFILE_A_APP_ID].connId,
                    .handle = descr_elem_result[0].handle,
                    .value_len = sizeof(notifyEn),
                    .write_type = OHOS_GATT_WRITE_TYPE_NO_RSP,
                };
                ret_status = BleGattcWriteCharDescr(write_char,
                                                    (uint8_t *)&notifyEn,
                                                    OHOS_BLE_AUTH_NO_BOND);
            }
            if (ret_status != BT_SUCCESS) {
                BT_LOGE("BleGattcWriteCharDescr error");
            }
            free(descr_elem_result);
            descr_elem_result = NULL;
        }
    } else {
        BT_LOGE("decsr not found");
    }
}

static void reg_notify_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    BT_LOGE("OHOS_GATTC_REG_FOR_NOTIFY_EVT");
    if (p_data->reg_for_notify.status != BT_SUCCESS) {
        BT_LOGE("REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
    } else {
        act_notify_handle(gattc_if, p_data);
    }
}

static void notify_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    if (p_data->notify.is_notify) {
        BT_LOGE("OHOS_GATTC_NOTIFY_EVT, receive notify value:");
    } else {
        BT_LOGE("OHOS_GATTC_NOTIFY_EVT, receive indicate value:");
    }
    int dataLength = p_data->notify.value_len;
    printf("\n receive value length: %d \n", dataLength);
    printf("receive value: ");
    for (int i = 0; i < dataLength; i++) {
        printf("%x ", p_data->notify.value[i]);
        GpioMessageHandler(p_data->notify.value[i]);
    }
    printf("\n");
}

static void write_descr_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    if (p_data->write.status != BT_SUCCESS) {
        BT_LOGE("write descr failed, error status = %x", p_data->write.status);
        return;
    }
    BT_LOGE("write descr success ");
    uint8_t writeCharData[WRITE_DATA_SIZE] = {0x00, 0x01, 0x02, 0x03};
    GattcWriteChar write_char = {
        .gattc_if = gattc_if,
        .conn_id = g_profileTab[PROFILE_A_APP_ID].connId,
        .handle = descr_elem_result[0].handle,
        .value_len = sizeof(writeCharData),
        .write_type = OHOS_GATT_WRITE_TYPE_NO_RSP,
    };
    BleGattcWriteCharacteristic(write_char,
                                writeCharData,
                                OHOS_BLE_AUTH_NO_BOND);
}

static void srvc_chg_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    uint32_t bda[OHOS_BD_ADDR_LEN];
    memcpy_s(bda, sizeof(bda), p_data->srvc_chg.remote_bda, sizeof(p_data->srvc_chg.remote_bda));
    BT_LOGE("OHOS_GATTC_SRVC_CHG_EVT, bd_addr:");
}

static void wrire_char_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    if (p_data->write.status != BT_SUCCESS) {
        BT_LOGE("write char failed, error status = %x", p_data->write.status);
        return;
    }
    BT_LOGE("write char success ");
}

static void gattc_disconnect_evt(GattInterfaceType gattc_if, BleGattcParam *p_data)
{
    connect = false;
    get_server = false;
    BT_LOGE("OHOS_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
    BT_LOGE("GATTC RECONNECT!\n");
    BleGattcConnect(g_profileTab[PROFILE_A_APP_ID].gattcIf,
                    NULL, (unsigned char *)g_directAddr,
                    true,
                    BLE_ADDR_TYPE_PUBLIC);
}

static void gattc_event_handler(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param)
{
    BleGattcParam *p_data = (BleGattcParam *)param;

    switch (event) {
        case OHOS_GATTC_REG_EVT:
            gattc_reg_evt(gattc_if, p_data);
            break;
        case OHOS_GATTC_CONNECT_EVT: {
            connect_evt(gattc_if, p_data);
            break;
        }
        case OHOS_GATTC_OPEN_EVT:
            open_evt(p_data, param);
            break;
        case OHOS_GATTC_DIS_SRVC_CMPL_EVT:
            dis_srvccmpl_evt(gattc_if, param);
            break;
        case OHOS_GATTC_CFG_MTU_EVT:
            cfg_mtu_evt(param);
            break;
        case OHOS_GATTC_SEARCH_RES_EVT: {
            search_res_evt(gattc_if, p_data);
            break;
        }
        case OHOS_GATTC_SEARCH_CMPL_EVT:
            search_cmpl_event(gattc_if, p_data);
            break;
        case OHOS_GATTC_REG_FOR_NOTIFY_EVT: {
            reg_notify_evt(gattc_if, p_data);
            break;
        }
        case OHOS_GATTC_NOTIFY_EVT:
            notify_evt(gattc_if, p_data);
            break;
        case OHOS_GATTC_WRITE_DESCR_EVT:
            write_descr_evt(gattc_if, p_data);
            break;
        case OHOS_GATTC_SRVC_CHG_EVT: {
            srvc_chg_evt(gattc_if, p_data);
            break;
        }
        case OHOS_GATTC_WRITE_CHAR_EVT:
            wrire_char_evt(gattc_if, p_data);
            break;
        case OHOS_GATTC_DISCONNECT_EVT:
            gattc_disconnect_evt(gattc_if, p_data);
            break;
        default:
            break;
    }
}

static void gap_cb(GapSearchEvent event, BleGapParam *param)
{
    uint8_t *advName = NULL;
    uint8_t advNameLen = 0;
    switch (event) {
        case OHOS_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            BleStartScan();
            break;
        }
        case OHOS_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT: {
            // to do read number from param, push to  message queue
            DeviceRssiInfo deviceRssiInfo = {0};
            deviceRssiInfo.rssi = param->read_rssi_cmpl.rssi;
            memcpy_s(deviceRssiInfo.deviceId, MAX_DEVICE_ID_LENTH,
                     param->read_rssi_cmpl.remote_addr, sizeof(param->read_rssi_cmpl.remote_addr));
            if (g_RssiStatusMsg != NULL) {
                osStatus_t status = osMessageQueuePut(g_RssiStatusMsg, (void *)&deviceRssiInfo, 0, osWaitForever);
            }
        }

        default:
            break;
    }
}

static void gattc_cb(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == OHOS_GATTC_REG_EVT) {
        if (param->reg.status == BT_SUCCESS) {
            g_profileTab[param->reg.app_id].gattcIf = gattc_if;
        } else {
            BT_LOGE("reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    int idx;
    for (idx = 0; idx < PROFILE_NUM; idx++) {
        /* OHOS_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
        if (gattc_if == OHOS_GATT_IF_NONE ||
            gattc_if == g_profileTab[idx].gattcIf) {
            if (g_profileTab[idx].gattcCb) {
                g_profileTab[idx].gattcCb(event, gattc_if, param);
            }
        }
    }
}

void GattcExample(void)
{
    g_RssiStatusMsg = osMessageQueueNew(RSSI_MSG_COUNT, RSSI_MSG_SIZE, NULL);
    GpioInit();
    EnableBle();
    BleGattcConfigureMtuSize(MTU_SIZE);
    BtGattClientCallbacks btGattClientCallbacks = {
        .gap_callback = gap_cb,
        .gattc_callback = gattc_cb,
        .PROFILE_APP_ID = PROFILE_A_APP_ID,
    };
    BleGattcRegister(btGattClientCallbacks);
}
