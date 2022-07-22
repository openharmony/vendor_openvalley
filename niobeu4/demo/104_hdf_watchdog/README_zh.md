# NiobeU4开发板WATCHDOG使用说明文档
本示例将演示如何在Niobeu4开发板上使用WATCHDOG接口进行任务看门狗开发

## 接口说明

```
1. Watchdog Open打开看门狗函数:int32_t WatchdogOpen(int16_t wdtId,  DevHandle *handle);
    参数说明: 
        wdtId:    看门狗ID
        handle:   设备句柄
        return:   如果操作成功，返回指向看门狗设备句柄的指针

2. Watchdog close关闭看门狗函数:void WatchdogClose(DevHandle handle);
    参数说明: 
        handle:   设备句柄
        
3. Watchdog feed喂狗函数:int32_t WatchdogFeed(DevHandle handle);
    参数说明: 
        handle:   设备句柄
        return:   返回0,表示喂狗成功，否则喂狗失败
        
4. Watchdog getStatus获取看门狗状态函数:int32_t WatchdogGetStatus(DevHandle handle, int32_t *status);
    参数说明: 
        handle:   设备句柄
        status:   看门狗状态指针
        return:   返回0,表示获得看门狗状态成功，否则获得看门狗状态失败

5. Watchdog start启动看门狗函数:int32_t WatchdogStart(DevHandle handle);
    参数说明: 
        handle:   设备句柄
        return:   返回0,表示启动看门狗成功，否则启动看门狗失败    
        
6. Watchdog stop停止看门狗函数:int32_t WatchdogStop(DevHandle handle);
    参数说明: 
        handle:   设备句柄
        return:   返回0,表示关闭看门狗成功，否则关闭看门狗失败 

7. Watchdog setTimeout设置看门狗超时时间函数:int32_t WatchdogSetTimeout(DevHandle handle, uint32_t seconds);
    参数说明: 
        handle:   设备句柄
        seconds:  超时时间
        return:   返回0,表示设置看门狗超时时间成功，否则设置看门狗超时时间失败 
        
8. Watchdog getTimeout获取看门狗超时时间函数:int32_t WatchdogGetTimeout(DevHandle handle, uint32_t *seconds);
    参数说明: 
        handle:   设备句柄
        seconds:  超时时间指针，单位为秒
        return:   返回0,表示获取看门狗超时时间函数成功，否则获取看门狗超时时间函数失败 
```

## 编译调试

- 进入//kernel/liteos_m目录, 在menuconfig配置中进入如下选项:
  `(Top) → Platform → Board Selection → select board niobeu4 → use openvalley niobeu4 application → niobeu4 application choose`
- 选择 `104_hdf_watchdog`
- 回到sdk根目录，执行`hb build`脚本进行编译。

## hcs配置

hcs配置文件在device/board/openvalley/niobeu4/liteos_m/hdf_config/hdf.hcs目录下，需要进行如下设置：

```
watchdog_config {
    watchdog0_config {
        match_attr = "watchdog0";
        id = 0;         // 看门狗ID
        timeout = 1;    // 看门狗超时时间，单位s
    }
  }
```
