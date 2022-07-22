

﻿# NiobeU4开发板GPIO使用说明文档

## 简介
 GPIO（General Purpose I/O Ports）是通用输入/输出端口，通俗地说，就是一些引脚，可以通过它们输出高低电平或者通过它们读入引脚的状态-是高电平或是低电平。本案例程序将演示GPIO控制LED灯相关程序。

## 接口说明

```
1. GPIO write写入引脚的电平值函数:int32_t GpioWrite(uint16_t gpio, uint16_t val);
    参数说明: 
        pin:    GPIO引脚ID
        *val:   写入电平状态值
        return: 返回0,表示写入成功，否则写入失败

2. GPIO read读取引脚电平值函数:int32_t GpioRead(uint16_t gpio, uint16_t *val);
    参数说明: 
        pin:    GPIO引脚ID
        *val:   写入电平状态值
        return: 返回0,表示读取成功，否则读取失败
        
3. GPIO setDir设置GPIO引脚的输入/输出方向函数:int32_t GpioSetDir(uint16_t gpio, uint16_t dir);
    参数说明: 
        pin:    GPIO引脚ID
        dir:    GPIO引脚方向
        return: 返回0,表示设置引脚方向成功，否则设置失败    
        
5. GPIO SetIrq设置GPIO引脚的中断功能函数:int32_t GpioSetIrq(uint16_t gpio, uint16_t mode, GpioIrqFunc func, void *arg);
    参数说明: 
        pin:    GPIO引脚ID
        mode:   GPIO中断类型
        func:   GPIO中断函数
        arg:    指向传递给ISR函数的参数的指针
        return: 返回0,表示设置中断功能成功，否则设置中断功能失败

6. Gpio EnableIrq使能GPIO引脚中断函数:int32_t GpioEnableIrq(uint16_t gpio);
    参数说明: 
        pin:    GPIO引脚ID
        return: 返回0,表示使能GPIO引脚中断成功，否则使能GPIO引脚中断失败    
        
7. Gpio UnsetIrq取消GPIO引脚的中断设置函数:int32_t GpioUnsetIrq(uint16_t gpio, void *arg);
    参数说明: 
        pin:    GPIO引脚ID
        arg:    指向传递给ISR函数的参数的指针
        return: 返回0,表示取消GPIO引脚的中断设置成功，否则取消GPIO引脚的中断设置失败 

8. Gpio DisableIrq禁用GPIO引脚中断函数:int32_t GpioDisableIrq(uint16_t gpio);
    参数说明: 
        pin:    GPIO引脚ID
        return: 返回0,表示禁用GPIO引脚中断成功，否则禁用GPIO引脚中断失败 
```

## 约束
GPIO组件使用C语言编写，目前支持Niobeu4开发板。

## 编译调试

- 进入//kernel/liteos_m目录, 在menuconfig配置中进入如下选项:
     `(Top) → Platform → Board Selection → select board niobeu4 → use openvalley niobeu4 application → niobeu4 application choose`
- 选择 `102_hdf_gpio`
- 回到sdk根目录，执行`hb build`脚本进行编译。

## hcs配置

hcs配置文件在device/board/openvalley/niobeu4/liteos_m/hdf_config/hdf.hcs目录下，需要进行如下设置：

```
    gpio_config {
            template default_config {
                match_attr = "default_config";
                /*         gpio_num_t,    gpio_pull_mode_t,    gpio_drive_cap_t       */
                /*          GPIO号           上下拉状态            驱动强度               */
                gpio_attr = ["GPIO_NUM_2", "GPIO_PULLUP_ONLY", "GPIO_DRIVE_CAP_DEFAULT"];
            }
            gpio_0 :: default_config {
                gpio_attr = ["GPIO_NUM_2", "GPIO_FLOATING", "GPIO_DRIVE_CAP_DEFAULT"];
            }
            gpio_1 :: default_config {
                gpio_attr = ["GPIO_NUM_27", "GPIO_FLOATING", "GPIO_DRIVE_CAP_DEFAULT"];
            }
            gpio_2 :: default_config {
                gpio_attr = ["GPIO_NUM_26", "GPIO_FLOATING", "GPIO_DRIVE_CAP_DEFAULT"];
            }
            gpio_3 :: default_config {
                gpio_attr = ["GPIO_NUM_39", "GPIO_FLOATING", "GPIO_DRIVE_CAP_DEFAULT"];
            }
        }
```

其中gpio_num_t、gpio_pull_mode_t以及gpio_drive_cap_t详细定义参考[device_soc_esp](https://gitee.com/openharmony-sig/device_soc_esp)仓库gpio_types.h代码

