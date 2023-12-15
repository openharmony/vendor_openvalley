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

#include "bluetooth_heart_rate_example.h"
#include "cmsis_os2.h"
#include "ohos_run.h"

/// Declare the static function
static void GattsProfileAEventHandler(GattsBleCallbackEvent event, GattInterfaceType gattsIf, BleGattsParam *param);

static uint8_t char1_str[] = {0x11, 0x22, 0x33};
static PrepareEnvType g_prepareWriteEnv;
uint8_t directAddr[6] = {0x44, 0x17, 0x93, 0xb4, 0x0b, 0xe2}; // 44:17:93:b4:0b:e2

static BleAttrValue gatts_demo_char1_val = {
    .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
    .attr_len = sizeof(char1_str),
    .attr_value = char1_str,
};

static uint8_t g_advConfigDone = 0;
#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
#define INDEX0 0
#define INDEX1 0
#define INDEX2 0
#define INDEX3 0
#define INDEX4 0
#define INDEX5 0
#define HEART_RATE_MAX 92
#define HEART_RATE_MIN 60
#define STACK_SIZE (1024 * 4)
#define DELAY_TICKS 200
#define FLAG_RECV_TURE 1
#define FLAG_RECV_FALSE 0

int g_heartbeat = HEART_RATE_MIN; // 心率最高93
uint8_t g_gattsIfs = 0;           // Gatt接口类型
uint8_t g_connIds = 0;            // 连接id
uint8_t g_charHandles = 0;
uint8_t g_flagRec = FLAG_RECV_FALSE; // 校验成功标志位

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {0x02, 0x01, 0x06, 0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd};
static uint8_t raw_scan_rsp_data[] = {0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41,
                                      0x54, 0x54, 0x53, 0x5f, 0x44, 0x45, 0x4d, 0x4f};
#else

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    // second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

// The length of adv data must be less than 31 bytes
// adv data
static BleAdvData adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, // BleSlave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // BleSlave connection max interval, Time = max_interval * 1.25 msec
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

#endif /* CONFIG_SET_RAW_ADV_DATA */

static BleAdvParams2 adv_params = {
    .adv_int_min = 0x20, // 广播间隔小值
    .adv_int_max = 0x40, // 广播间隔大值
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* 基于gatt的配置文件 */
static struct GattsProfileInst g_profileTab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] =
        {
            .gatts_cb = GattsProfileAEventHandler,
            .gatts_if = OHOS_GATT_IF_NONE, /* 没有获得gatt_if，所以初始值是OHOS_GATT_IF_NONE */
        },
};

void HeartRateTask(void)
{
    BT_LOGE("HeartRateTask enter\n");
    while (1) {
        if (g_flagRec) {
            // 数据生成规则
            if (g_heartbeat > HEART_RATE_MAX) {
                g_heartbeat = HEART_RATE_MIN;
            }
            ++g_heartbeat;
            uint8_t write_char_data[] = {0x06, 0x03, g_heartbeat};
            GattsSendParam param = {
                .gattsIf = g_gattsIfs,
                .connId = g_connIds,
                .attrHandle = g_charHandles,
                .valueLen = sizeof(write_char_data),
                .value = write_char_data,
                .needConfirm = false,
            };
            BleGattsSendIndication(&param); // 发送数据
            osDelay(DELAY_TICKS);
        } else {
            break;
        }
    }
}

static void StartHeartRateTask(void)
{
    BT_LOGE("StartHeartRateTask enter\n");
    osThreadAttr_t attr;
    attr.name = "HeartRateTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)HeartRateTask, NULL, &attr) == NULL) {
        printf("Falied to create HeartRateTask!\n");
    }
}

static void GapEventHandler(GapBleCallbackEvent event, BleGapParam *param)
{
    switch (event) {
        case OHOS_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            g_advConfigDone &= (~ADV_CONFIG_FLAG);
            if (g_advConfigDone == 0) {
                BleGattsStartAdvertising(&adv_params);
            }
            break;
        case OHOS_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            g_advConfigDone &= (~SCAN_RSP_CONFIG_FLAG);
            if (g_advConfigDone == 0) {
                BleGattsStartAdvertising(&adv_params);
            }
            break;
        case OHOS_GAP_BLE_ADV_START_COMPLETE_EVT:
            // advertising start complete event to indicate advertising start successfully or failed
            if (param->adv_start_cmpl.status != BT_SUCCESS) {
                BT_LOGE("Advertising start failed\n");
            }
            break;
        case OHOS_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != BT_SUCCESS) {
                BT_LOGE("Advertising stop failed\n");
            } else {
                BT_LOGE("Stop adv successfully\n");
            }
            break;
        case OHOS_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            BT_LOGE("update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, "
                    "timeout = %d",
                    param->update_conn_params.status, param->update_conn_params.min_int,
                    param->update_conn_params.max_int, param->update_conn_params.conn_int,
                    param->update_conn_params.latency, param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}

void ExampleWriteEventEnv(GattInterfaceType gattsIf, PrepareEnvType *prepareWriteEnv, BleGattsParam *param)
{
    GattStatus status = OHOS_GATT_SUCCESS;
    if (param->write.need_rsp) {
        if (param->write.is_prep) {
            if (prepareWriteEnv->prepareBuf == NULL) {
                prepareWriteEnv->prepareBuf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
                prepareWriteEnv->prepareLen = 0;
            }
            if (prepareWriteEnv->prepareBuf == NULL) {
                BT_LOGE("Gatt_server prep no mem\n");
                status = OHOS_GATT_INVALID_ATTRIBUTE_VALUE_LENGTH;
            }
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                status = OHOS_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                status = OHOS_GATT_INVALID_ATTRIBUTE_VALUE_LENGTH;
            }

            BleGattRsp *gatt_rsp = (BleGattRsp *)malloc(sizeof(BleGattRsp));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = OHOS_BLE_AUTH_NO_BOND;
            memcpy_s(gatt_rsp->attr_value.value, gatt_rsp->attr_value.len, param->write.value, param->write.len);
            BtError response_err =
                BleGattsSendResponse(gattsIf, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != BT_SUCCESS) {
                BT_LOGE("Send response error\n");
            }
            free(gatt_rsp);
            if (status != OHOS_GATT_SUCCESS) {
                return;
            }
            memcpy_s(prepareWriteEnv->prepareBuf + param->write.offset,
                     PREPARE_BUF_MAX_SIZE - prepareWriteEnv->prepareLen,
                     param->write.value, param->write.len);
            prepareWriteEnv->prepareLen += param->write.len;
        } else {
            BleGattsSendResponse(gattsIf, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

void ExampleExecWriteEventEnv(PrepareEnvType *prepareWriteEnv, BleGattsParam *param)
{
    if (param->exec_write.exec_write_flag == OHOS_GATT_PREP_WRITE_EXEC) {
        printf("exec_write_flag is OHOS_GATT_PREP_WRITE_EXEC !\n");
    } else {
        BT_LOGE("OHOS_GATT_PREP_WRITE_EXEC");
    }
    if (prepareWriteEnv->prepareBuf) {
        free(prepareWriteEnv->prepareBuf);
        prepareWriteEnv->prepareBuf = NULL;
    }
    prepareWriteEnv->prepareLen = 0;
}

static void CreateService(GattInterfaceType gattsIf)
{
    g_profileTab[PROFILE_A_APP_ID].service_id.is_primary = true;
    g_profileTab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00; // instance id
    g_profileTab[PROFILE_A_APP_ID].service_id.id.uuid.len =
        sizeof(g_profileTab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16); // uuid长度
    g_profileTab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;

    BtError set_dev_name_ret = SetLocalName(TEST_DEVICE_NAME, sizeof(TEST_DEVICE_NAME));
    if (set_dev_name_ret) {
        BT_LOGE("set device name failed, error code = %x", set_dev_name_ret);
    }
    // config adv data
    BtError ret = BleGapConfigAdvData(&adv_data);
    if (ret) {
        BT_LOGE("config adv data failed, error code = %x", ret);
    }
    g_advConfigDone |= ADV_CONFIG_FLAG;
    // config scan response data
    ret = BleGapConfigAdvData(&scan_rsp_data);
    if (ret) {
        BT_LOGE("config scan response data failed, error code = %x", ret);
    }
    g_advConfigDone |= SCAN_RSP_CONFIG_FLAG;
    BleGattsCreateService(gattsIf, &g_profileTab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
}

static void Addcharacteristics(BleGattsParam *param)
{
    if (param == NULL) {
        BT_LOGE("Addcharacteristics param null");
        return;
    }
    g_profileTab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
    g_profileTab[PROFILE_A_APP_ID].char_uuid.len = sizeof(g_profileTab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16);
    g_profileTab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;

    BleGattsStartService(g_profileTab[PROFILE_A_APP_ID].conn_id, g_profileTab[PROFILE_A_APP_ID].service_handle);
    GattsChar gttsChar = {
        .charUuid = &g_profileTab[PROFILE_A_APP_ID].char_uuid,
        .perm = OHOS_GATT_PERMISSION_READ | OHOS_GATT_PERMISSION_WRITE,
        .property = OHOS_GATT_CHARACTER_PROPERTY_BIT_READ | OHOS_GATT_CHARACTER_PROPERTY_BIT_WRITE |
                    OHOS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY,
        .charVal = &gatts_demo_char1_val,
        .control = NULL,
    };
    BtError add_char_ret = BleGattsAddChar(g_profileTab[PROFILE_A_APP_ID].service_handle, &gttsChar);
    if (add_char_ret) {
        BT_LOGE("add char failed, error code =%x", add_char_ret);
    }
}

static void AddcharacteristicsDesc(BleGattsParam *param)
{
    if (param == NULL) {
        BT_LOGE("AddcharacteristicsDesc param null");
        return;
    }
    uint16_t length = 0;
    const uint8_t *prf_char;
    g_profileTab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
    g_profileTab[PROFILE_A_APP_ID].descr_uuid.len =
        sizeof(g_profileTab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16);
    g_profileTab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = OHOS_GATT_UUID_CHAR_CLIENT_CONFIG;
    BtError get_attr_ret = BleGattsGetAttrValue(param->add_char.attr_handle, &length, &prf_char);
    if (get_attr_ret == BT_ERROR) {
        BT_LOGE("ILLEGAL HANDLE");
    }

    BT_LOGE("the gatts demo char length = %x\n", length);
    for (int i = 0; i < length; i++) {
        BT_LOGE("prf_char[%x] =%x\n", i, prf_char[i]);
    }
    BtError add_descr_ret = BleGattsAddCharDescr(
        g_profileTab[PROFILE_A_APP_ID].service_handle, &g_profileTab[PROFILE_A_APP_ID].descr_uuid,
        OHOS_GATT_PERMISSION_READ | OHOS_GATT_PERMISSION_WRITE, NULL, NULL);
    if (add_descr_ret) {
        BT_LOGE("add char descr failed, error code =%x", add_descr_ret);
    }
}

static void UpdateConnectInfo(BleGattsParam *param)
{
    if (param == NULL) {
        BT_LOGE("UpdateConnectInfo param null");
        return;
    }
    BleConnUpdateParams conn_params = {0};
    memcpy_s(conn_params.bda, sizeof(BdAddr), param->connect.remote_bda, sizeof(BdAddr));
    /* For the IOS system, please reference the apple official documents about the ble connection parameters
        * restrictions. */
    conn_params.latency = 0;
    conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
    conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
    conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
    BT_LOGE("OHOS_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:", param->connect.conn_id,
            param->connect.remote_bda[INDEX0], param->connect.remote_bda[INDEX1],
            param->connect.remote_bda[INDEX2], param->connect.remote_bda[INDEX3],
            param->connect.remote_bda[INDEX4], param->connect.remote_bda[INDEX5]);
    g_profileTab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
    // start sent the update connection parameters to the peer device.
    BleGapUpdateConnParams(&conn_params);
}

static void GattsProfileAEventHandler(GattsBleCallbackEvent event, GattInterfaceType gattsIf, BleGattsParam *param)
{
    switch (event) {
        case OHOS_GATTS_REG_EVT:
            BT_LOGE("REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
            CreateService(gattsIf);
            break;
        case OHOS_GATTS_READ_EVT:
            BT_LOGE("GATT_READ_EVT, conn_id %d, handle %d\n", param->read.conn_id, param->read.handle);
            break;
        case OHOS_GATTS_WRITE_EVT: { // 1、发送接收入口
            printf("\n receive A MergeRequestDeviceType data len: %d \n", param->write.len);
            if (g_flagRec == FLAG_RECV_FALSE) {
                g_flagRec = FLAG_RECV_TURE;
                g_gattsIfs = gattsIf;            // Gatt接口类型
                g_connIds = param->write.conn_id; // 连接id
                g_charHandles = g_profileTab[PROFILE_A_APP_ID].char_handle;
                StartHeartRateTask(); // 启动定时发送任务
            }
            ExampleWriteEventEnv(gattsIf, &g_prepareWriteEnv, param);
        }
            break;
        case OHOS_GATTS_EXEC_WRITE_EVT:
            BT_LOGE("OHOS_GATTS_EXEC_WRITE_EVT");
            BleGattsSendResponse(gattsIf, param->write.conn_id, param->write.trans_id, OHOS_GATT_SUCCESS, NULL);
            ExampleExecWriteEventEnv(&g_prepareWriteEnv, param);
            break;
        case OHOS_GATTS_CREATE_EVT:
            Addcharacteristics(param);
            break;
        case OHOS_GATTS_ADD_CHAR_EVT:
            BT_LOGE("ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n", param->add_char.status,
                    param->add_char.attr_handle, param->add_char.service_handle);
            AddcharacteristicsDesc(param);
            break;
        case OHOS_GATTS_ADD_CHAR_DESCR_EVT:
            g_profileTab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
            break;
        case OHOS_GATTS_CONNECT_EVT:
            UpdateConnectInfo(param);
            break;
        case OHOS_GATTS_DISCONNECT_EVT:
            BT_LOGE("OHOS_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
            g_flagRec = FLAG_RECV_FALSE;
            BleGattsStartAdvertising(&adv_params);
            break;
        case OHOS_GATTS_CONF_EVT:
            BT_LOGE("OHOS_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
            break;
        default:
            break;
    }
}

static void GattsEventHandler(GattsBleCallbackEvent event, GattInterfaceType gattsIf, BleGattsParam *param)
{
    /* If event is register event, store the gattsIf for each profile */
    if (event == OHOS_GATTS_REG_EVT) {
        if (param->reg.status == OHOS_GATT_SUCCESS) {
            g_profileTab[param->reg.app_id].gatts_if = gattsIf;
        } else {
            BT_LOGE("Reg app failed, app_id %04x, status %d\n", param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* If the gattsIf equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if ((gattsIf == OHOS_GATT_IF_NONE || gattsIf == g_profileTab[idx].gatts_if) && g_profileTab[idx].gatts_cb) {
                g_profileTab[idx].gatts_cb(event, gattsIf, param);
            }
        }
    } while (0);
}

void AppHearRateMain(void)
{
    EnableBle();
    BtGattServerCallbacks btGattServerCallbacks = {
        .gapCallback = GapEventHandler,
        .gattsCallback = GattsEventHandler,
        .profileAppId = PROFILE_A_APP_ID,
    };
    BleGattsRegisterCallbacks(btGattServerCallbacks);
    return;
}

/* 声明测试函数入口 */
OHOS_APP_RUN(AppHearRateMain);
