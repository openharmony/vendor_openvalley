# vendor_openvalley

## 介绍
该仓库托管湖南开鸿智谷数字产业发展有限公司Niobe系列开发板的示例程序、配置文件及文档。

## 软件架构
代码路径说明

```
vendor/openvalley/
└──niobeu4              # niobeu4开发板
   ├── niobeu4/applications     # 应用程序目录
   ├── kernel_configs   # 内核配置
   └── hals
└──niobeu4_xts          # niobeu4_xts代码
   ├── hdf_config       # 开发板配置目录
   ├── kernel_configs   # 内核配置
   └── hals
```

## 使用说明

### 编译

```
进入https://gitee.com/openharmony-sig/device_board_openvalley/tree/master/niobeu4 目录参考REAMD.md文档进行编译 
```


## 示例展示

开发套件SDK中提供了大量示例代码：

| 示例功能                  | 示例名称                                                     |
| ------------------------- | :----------------------------------------------------------- |
| 调试打印示例              | [001_quick_start](niobeu4/applications/001_quick_start/README_zh.md) |
| `LOS`接口线程使用示例     | [002_system_los_thread](niobeu4/applications/002_system_los_thread/README_zh.md) |
| `LOS`接口定时器使用示例   | [003_system_los_timer](niobeu4/applications/003_system_los_timer/README_zh.md) |
| `LOS`接口事件使用示例     | [004_system_los_event](niobeu4/applications/004_system_los_event/README_zh.md) |
| `LOS`接口互斥锁使用示例   | [005_system_los_mutex](niobeu4/applications/005_system_los_mutex/README_zh.md) |
| `LOS`接口信号量使用示例   | [006_system_los_semp](niobeu4/applications/006_system_los_semp/README_zh.md) |
| `LOS`接口消息队列使用示例 | [007_system_los_message](niobeu4/applications/007_system_los_message/README_zh.md) |
| ADC采集示例               | [101_hdf_adc](niobeu4/applications/101_hdf_adc/README_zh.md) |

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
