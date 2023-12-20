# 蓝牙获取电量示例

## 简介

本案例为蓝牙获取电量示例代码。该代码实现了蓝牙低能耗(BLE)通用属性(GATTS)服务端，创建一个自定义服务，发送广播，允许客户端扫描连接，读取电量。

## 编译调试

- 进入//kernel/liteos_m目录, 执行`make menuconfig`，在menuconfig配置中进入如下选项:
  `(Top) → Platform → Board Selection → select board niobeu4 → use openvalley niobeu4 application → niobeu4 application choose`
- 选择 `602_bluetooth_battery`
- 回到sdk根目录，执行`hb build`脚本进行编译。

## 案例程序
```c
#define GATTS_SERVICE_UUID_TEST_A 0x00FF    //设置蓝牙设备服务UUID
#define GATTS_CHAR_UUID_TEST_A 0xFF01       //设置读写UUID
#define TEST_DEVICE_NAME "NiobeU4_Battery"  //设置蓝牙设备名称
#define GATTS_NUM_HANDLE_TEST_A 4           //设置服务请求的句柄数
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40    //当前属性值长度
#define PROFILE_NUM 1                       //设置Profile数量
#define PROFILE_A_APP_ID 0                  //设置Profile对应的App ID
```
GattsProfile配置结构体
```c
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
```
### 主入口
程序入口点是bluetooth_battery_example()函数
```C
void bluetooth_battery_example(void)
{
    BT_LOGE("bluetooth_broadcast_example run\n");
    EnableBle();
    BtGattServerCallbacks btGattServerCallbacks = {
        .gap_callback = gap_cb,
        .gatts_callback = gatts_event_handler,
        .profileAppId = PROFILE_A_APP_ID,
    };
    BleGattsRegisterCallbacks(btGattServerCallbacks);
    return;
}
```
在这个入口函数中执行了打开蓝牙，注册gatts服务端回调函数的操作
`gap_cb`事件回调函数，用于GAP协议触发设备对应的访问模式、连接和广播事件
`gatts_event_handler`事件回调函数，gatts事件处理，应用Profile存储在一个数组中，遍历数组通过gatts_if调用gatts_cb


