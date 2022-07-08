/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
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
#include "gattc_example.h"
#include "ohos_run.h"
#include "cmsis_os2.h"
static const char remote_device_name[] = "BLUETOOTH";
static bool connect = false;
static bool get_server = false;
static BleGattcCharElem *char_elem_result = NULL;
static BleGattcDescrElem *descr_elem_result = NULL;

/* Declare static functions */
static void gap_cb(GapSearchEvent event, BleGapParam *param);
static void gattc_cb(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param);
static void gattc_profile_event_handler(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param);

uint8_t directAddr[6] = {0x3c, 0x61, 0x05, 0x7d, 0xdf, 0x66}; // 0x3c, 0x61, 0x05, 0x7d, 0xdf, 0x66 / 0x44, 0x17, 0x93, 0xb4, 0x0b, 0xe2 / 0xd5, 0xef, 0xda, 0x6b, 0x5b, 0x25

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
    .scan_interval = 0x0010,
    .scan_window = 0x0050,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by OHOS_GATTS_REG_EVT */
static struct GattcProfileInst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = OHOS_GATT_IF_NONE, /* Not get the gatt_if, so initial is OHOS_GATT_IF_NONE */
    },
};

static void gattc_profile_event_handler(GattcBleCallbackEvent event, GattInterfaceType gattc_if, BleGattcParam *param)
{
    BleGattcParam *p_data = (BleGattcParam *)param;

    switch (event) {
    case OHOS_GATTC_REG_EVT:
        BT_LOGE("REG_EVT");
        BtError scan_ret = BleGatSetScanParams(&ble_scan_params);
        if (scan_ret) {
            BT_LOGE("set scan params error, error code = %x", scan_ret);
        }
        break;
    case OHOS_GATTC_CONNECT_EVT: {
        BT_LOGE("OHOS_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(BdAddrs));
        BT_LOGE("REMOTE BDA:");
        BtError mtu_ret = BleGattcSendMtuReq(gattc_if, p_data->connect.conn_id);
        if (mtu_ret) {
            BT_LOGE("config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case OHOS_GATTC_OPEN_EVT:
        if (param->open.status != BT_SUCCESS) {
            BT_LOGE("open failed, status %d", p_data->open.status);
            break;
        }
        BT_LOGE("open success");
        break;
    case OHOS_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != BT_SUCCESS) {
            BT_LOGE("discover service failed, status %d", param->dis_srvc_cmpl.status);
            break;
        }
        BT_LOGE("discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        BleGattcSearchServices(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case OHOS_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != BT_SUCCESS) {
            BT_LOGE("config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        BT_LOGE("OHOS_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        break;
    case OHOS_GATTC_SEARCH_RES_EVT: {
        BT_LOGE("SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        BT_LOGE("start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == 2 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
            BT_LOGE("service found");
            get_server = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
            BT_LOGE("UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
        }
        break;
    }
    case OHOS_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != BT_SUCCESS) {
            BT_LOGE("search service failed, error status = %x", p_data->search_cmpl.status);
            break;
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
            uint16_t count = 0;
            GattStatus status = BleGattcGetAttrCount(gattc_if,
                                                                p_data->search_cmpl.conn_id,
                                                                OHOS_GATT_DB_CHARACTERISTIC,
                                                                gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
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
                    status = BleGattcGetCharByUuid(gattc_if,
                                                            p_data->search_cmpl.conn_id,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                            remote_filter_char_uuid,
                                                            char_elem_result,
                                                            &count);
                    if (status != BT_SUCCESS) {
                        BT_LOGE("BleGattcGetCharByUuid error");
                    }

                    /*  Every service have only one char in our 'OHOS_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result[0].properties & OHOS_GATT_CHAR_PROP_BIT_NOTIFY)) {
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result[0].char_handle;
                        BleGattcRegisterForNotify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result[0].char_handle);
                    }
                }
                /* free char_elem_result */
                free(char_elem_result);
            } else {
                BT_LOGE("no char found");
            }
        }
        break;
    case OHOS_GATTC_REG_FOR_NOTIFY_EVT: {
        BT_LOGE("OHOS_GATTC_REG_FOR_NOTIFY_EVT");
        if (p_data->reg_for_notify.status != BT_SUCCESS) {
            BT_LOGE("REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
        } else {
            uint16_t count = 0;
            uint16_t notify_en = 1;
            GattStatus ret_status = BleGattcGetAttrCount(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        OHOS_GATT_DB_DESCRIPTOR,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                        &count);
            if (ret_status != BT_SUCCESS) {
                BT_LOGE("BleGattcGetAttrCount error");
            }
            if (count > 0) {
                descr_elem_result = malloc(sizeof(BleGattcDescrElem) * count);
                if (!descr_elem_result) {
                    BT_LOGE("malloc error, gattc no mem");
                } else {
                    ret_status = BleGattcGetDescrByCharHandle(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        p_data->reg_for_notify.handle,
                                                                        notify_descr_uuid,
                                                                        descr_elem_result,
                                                                        &count);
                    if (ret_status != BT_SUCCESS) {
                        BT_LOGE("BleGattcGetDescrByCharHandle error");
                    }
                    /* Every char has only one descriptor in our 'OHOS_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == 2 && descr_elem_result[0].uuid.uuid.uuid16 == OHOS_GATT_UUID_CHAR_CLIENT_CONFIG) {
                        ret_status = BleGattcWriteCharDescr(gattc_if,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                    descr_elem_result[0].handle,
                                                                    sizeof(notify_en),
                                                                    (uint8_t *)&notify_en,
                                                                    OHOS_GATT_WRITE_TYPE_NO_RSP,
                                                                    OHOS_BLE_AUTH_NO_BOND);
                    }

                    if (ret_status != BT_SUCCESS) {
                        BT_LOGE("BleGattcWriteCharDescr error");
                    }

                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            } else {
                BT_LOGE("decsr not found");
            }
        }
        break;
    }
    case OHOS_GATTC_NOTIFY_EVT: 
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
        }
        printf("\n");
        break;
    case OHOS_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != BT_SUCCESS) {
            BT_LOGE("write descr failed, error status = %x", p_data->write.status);
            break;
        }
        BT_LOGE("write descr success ");
        uint8_t write_char_data[4] = {0x00, 0x01, 0x02, 0x03}; 
        BleGattcWriteCharacteristic(gattc_if,
                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                    gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                    sizeof(write_char_data),
                                    write_char_data,
                                    OHOS_GATT_WRITE_TYPE_RSP,
                                    OHOS_BLE_AUTH_NO_BOND);
        break;
    case OHOS_GATTC_SRVC_CHG_EVT: {
        BdAddrs bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(BdAddrs));
        BT_LOGE("OHOS_GATTC_SRVC_CHG_EVT, bd_addr:");
        break;
    }
    case OHOS_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != BT_SUCCESS) {
            BT_LOGE("write char failed, error status = %x", p_data->write.status);
            break;
        }
        BT_LOGE("write char success ");
        break;
    case OHOS_GATTC_DISCONNECT_EVT:
        connect = false;
        get_server = false;
        BT_LOGE("OHOS_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
        BT_LOGE("GATTC RECONNECT ! \n");
        BleGattcConnect(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, NULL, (unsigned char *)directAddr, true, BLE_ADDR_TYPE_PUBLIC); //根据mac地址连接蓝牙
        break;
    default:
        break;
    }
}

static void gap_cb(GapSearchEvent event, BleGapParam *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {
    case OHOS_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        BleStartScan();
        break;
    }
    case OHOS_GAP_BLE_SCAN_RESULT_EVT: {
        BleGapParam *scan_result = (BleGapParam *)param;
        switch (scan_result->scan_rst.search_evt) {
        case OHOS_GAP_SEARCH_INQ_RES_EVT:
            BT_LOGE("searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = BleResolveAdvData(scan_result->scan_rst.ble_adv,
                                                OHOS_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            if (adv_name_len) {
                BT_LOGE("searched Device Name Len %d", adv_name_len);
                BtLogBufferChar(GATTC_TAG, adv_name, adv_name_len);

                BT_LOGE("\n");
            }

            if (adv_name != NULL) {
                if (strlen(remote_device_name) == adv_name_len && strncmp((char *)adv_name, remote_device_name, adv_name_len) == 0) {
                    BT_LOGE("searched device %s\n", remote_device_name);
                    if (connect == false) {
                        connect = true;
                        BT_LOGE("connect to the remote device.");
                        //蓝牙直连
                        BleStopScan();
                        memcpy(directAddr, scan_result->scan_rst.bda, sizeof(directAddr));
                        BleGattcConnect(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, NULL, scan_result->scan_rst.bda, true, BLE_ADDR_TYPE_PUBLIC);
                    }
                }
            }
            break;
        case OHOS_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
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
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            BT_LOGE("reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == OHOS_GATT_IF_NONE || /* OHOS_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

void bluetooth_example(void)
{
    EnableBle();
    BleGattcConfigureMtuSize(500);
    BtGattClientCallbacks btGattClientCallbacks = {
        .gap_callback = gap_cb,
        .gattc_callback = gattc_cb,
        .PROFILE_APP_ID = PROFILE_A_APP_ID,
    };
    BleGattcRegister(btGattClientCallbacks);
}

/* 声明测试函数入口 */
OHOS_APP_RUN(bluetooth_example);