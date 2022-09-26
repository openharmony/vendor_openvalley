# vendor_openvalley

## 介绍
该仓库托管湖南开鸿智谷数字产业发展有限公司Niobe系列开发板的示例程序、配置文件及文档。

## 目录结构
代码路径说明。

```
vendor/openvalley/
├── LICENSE
├── niobeu4            
│   ├── BUILD.gn
│   ├── config.json    # config文件
│   ├── demo           # 应用程序目录
│   ├── hals           # 适配层目录
│   ├── hdf_config     # hdf配置
│   ├── kernel_configs # 内核配置
│   ├── LICENSE
│   └── ohos.build
├── OAT.xml
└── README_zh.md

```

## 使用说明

参考[device_board_openvalley](https://gitee.com/openharmony/device_board_openvalley/blob/master/niobeu4/README_zh.md) 仓库下链接使用。


## 示例展示

开发套件SDK中提供了大量示例代码如下。

| 示例功能                  | 示例名称                                                     |
| ------------------------- | :----------------------------------------------------------- |
| 调试打印示例              | [001_quick_start](niobeu4/demo/001_quick_start/README_zh.md) |
| `LOS`接口线程使用示例     | [002_system_los_thread](niobeu4/demo/002_system_los_thread/README_zh.md) |
| `LOS`接口定时器使用示例   | [003_system_los_timer](niobeu4/demo/003_system_los_timer/README_zh.md) |
| `LOS`接口事件使用示例     | [004_system_los_event](niobeu4/demo/004_system_los_event/README_zh.md) |
| `LOS`接口互斥锁使用示例   | [005_system_los_mutex](niobeu4/demo/005_system_los_mutex/README_zh.md) |
| `LOS`接口信号量使用示例   | [006_system_los_semp](niobeu4/demo/006_system_los_semp/README_zh.md) |
| `LOS`接口消息队列使用示例 | [007_system_los_message](niobeu4/demo/007_system_los_message/README_zh.md) |
| `ADC`采集示例             | [101_hdf_adc](niobeu4/demo/101_hdf_adc/README_zh.md)         |
| `GPIO`使用示例            | [102_hdf_gpio](niobeu4/demo/102_hdf_gpio/README_zh.md)       |
| `UART`使用示例            | [103_hdf_uart](niobeu4/demo/103_hdf_uart/README_zh.md)       |
| `看门狗`使用示例          | [104_hdf_watchdog](niobeu4/demo/104_hdf_watchdog/README_zh.md) |
| `PWM`使用示例             | [105_hdf_pwm](niobeu4/demo/105_hdf_pwm/README_zh.md)         |
| `I2C`使用示例             | [106_hdf_i2c](niobeu4/demo/106_hdf_i2c/README_zh.md)         |
| `SPI`使用示例             | [107_hdf_spi](niobeu4/demo/107_hdf_spi/README_zh.md)         |
| `BLUETOOTH`使用示例       | [201_bluetooth](niobeu4/demo/201_bluetooth/README_zh.md)     |
| `WIFI`使用示例            | [202_wifi](niobeu4/demo/202_wifi/README_zh.md)               |

## 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request
