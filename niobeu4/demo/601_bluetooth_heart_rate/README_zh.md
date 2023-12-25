# 蓝牙示例

## 简介

本案例为蓝牙示例代码。该代码实现了蓝牙低能耗(BLE)通用属性(GATT)服务端，它开启有个蓝牙广播。然后，外部客户端通过扫描连接该服务端。 
该示例可以注册一个应用程序Profile文件并初始化一系列事件，这些事件可用于配置GAP参数，并处理诸如广播、外部连接和读写特征值等事件。

## 编译调试

- 进入//kernel/liteos_m目录, 在menuconfig配置中进入如下选项:
  `(Top) → Platform → Board Selection → select board niobeu4 → use openvalley niobeu4 application → niobeu4 application choose`
- 选择 `701_bluetooth_heart_rate`
- 回到sdk根目录，执行`hb build`脚本进行编译。

## 案例程序

本例程位于`vendor/openvalley/niobeu4/demo/601_bluetooth_heart_rate`文件夹下。

```c
#define GATTS_SERVICE_UUID_TEST_A   0x00FF // 设置蓝牙设备服务UUID
#define GATTS_CHAR_UUID_TEST_A      0xFF01 //设置读写UUID
#define GATTS_DESCR_UUID_TEST_A     0x3333 //设置特征描述符句柄
#define GATTS_NUM_HANDLE_TEST_A     2      //设置服务请求的句柄数
#define TEST_DEVICE_NAME            "ble_heart_rate" //设置蓝牙设备名称
#define TEST_MANUFACTURER_DATA_LEN  17     //数据长度
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40   //当前属性值长度
#define PREPARE_BUF_MAX_SIZE 1024          //设置buff最大值
//应用配置文件
#define PROFILE_NUM 1                      //设置Profile数量
#define PROFILE_A_APP_ID 0                 //设置Profile对应的App ID

/* 声明静态回调函数 */
static void GattsProfileAEventHandler(GattsBleCallbackEvent event, GattInterfaceType gattsIf, BleGattsParam *param);
```



#### 主入口

程序的入口点是AppHearRateMain()函数:

```c
    EnableBle();
    BtGattServerCallbacks btGattServerCallbacks = {
        .gapCallback = GapEventHandler,
        .gattsCallback = GattsEventHandler,
        .profileAppId = PROFILE_A_APP_ID,
    };
    BleGattsRegisterCallbacks(btGattServerCallbacks);
    return;
```

注册回调函数。事件的两个主要管理器是GAP和GATT事件处理程序。应用Profile存储在一个数组中，并分配回调函数' gatts_profile_a_event_handler() 。GATT客户机上的不同应用程序使用不同的接口，由gatts_if参数表示:

```c
/* 基于gatt的配置文件 */
static struct GattsProfileInst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = OHOS_GATT_IF_NONE, /* 没有获得gatt_if，所以初始值是OHOS_GATT_IF_NONE */
    },
};
```

注册应用程序事件是程序生命周期中触发的第*一个事件，本例使用Profile A GATT事件句柄在注册时配置发布参数。这个例子可以选择使用标准的蓝牙核心规格的广告参数或定制的原始缓冲区。该选项可以通过' CONFIG_SET_RAW_ADV_DATA '定义来选择。原始广播数据可用于实现iBeacons, Eddystone或别的专有，以及自定义框架类型，如用于室内定位服务，不同于标准规范。

用于配置标准蓝牙规范广播参数的函数是`BleGapConfigAdvData()`，它接受一个指向`BleAdvData`结构的指针。广播数据的`BleAdvData`数据结构有如下定义:

```c
typedef struct {
    bool set_scan_rsp;            /*!< Set this advertising data as scan response or not*/
    bool include_name;            /*!< Advertising data include device name or not */
    bool include_txpower;         /*!< Advertising data include TX power */
    int min_interval;             /*!< Advertising data show BleSlave preferred connection min interval */
    int max_interval;             /*!< Advertising data show BleSlave preferred connection max interval */
    int appearance;               /*!< External appearance of device */
    uint16_t manufacturer_len;    /*!< Manufacturer data length */
    uint8_t *p_manufacturer_data; /*!< Manufacturer data point */
    uint16_t service_data_len;    /*!< Service data length */
    uint8_t *p_service_data;      /*!< Service data point */
    uint16_t service_uuid_len;    /*!< Service uuid length */
    uint8_t *p_service_uuid;      /*!< Service uuid array point */
    uint8_t flag;                 /*!< Advertising flag of discovery mode, see BLE_ADV_DATA_FLAG detail */
} BleAdvData;
```

使用`BleGattsSendIndication`函数发送数据：

```c
    case OHOS_GATTS_WRITE_EVT: {//1、发送接收入口
            ...
            uint8_t write_char_data[5]={0x00, 0x01, 0x02, 0x03, 0x04, 0x05};//设备类型应答数据
            BleGattsSendIndication(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle, sizeof(write_char_data),             write_char_data, false);
```

触发条件：
1. 手机在应用商店下载蓝牙助手
2. 搜索"ble_heart_rate", 连接蓝牙。
3. 设备会返回心率测试数据给手机

