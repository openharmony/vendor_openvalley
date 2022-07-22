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

#include <stdio.h>
#include <hdf_log.h>
#include <i2c_if.h>
#include "cmsis_os2.h"
#include "ohos_run.h"

#define HDF_I2C_STACK_SIZE 0x1000
#define HDF_I2C_TASK_NAME "hdf_i2c_test_task"
#define HDF_I2C_TASK_PRIORITY 25
#define HDF_I2C_DELAY_TICKS 100

#define I2C_SUCCESE 1
#define I2C_ERROR 0
#define DEV_SLAVE_ADDR 0x50     // 从机设备地址
#define REG_WR_ADDR 0X40        // write reg address
#define REG_RD_ADDR 0X80        // read reg address
#define I2C_NUM 0
#define BUFF_LEN 2


static int I2cWrite(DevHandle i2c_handle, unsigned char regAddr, unsigned char data)
{
    int ret = I2C_ERROR;
    unsigned char buf[BUFF_LEN] = {regAddr, data};
    struct I2cMsg msg = {
        .addr = DEV_SLAVE_ADDR,
        .buf = buf,
        .len = BUFF_LEN,
        .flags = 0,
    };
    ret = I2cTransfer(i2c_handle, &msg, 1);
    if (ret != 0) {
        printf("I2cWrite msg failed\n");
        return I2C_ERROR;
    }
    return I2C_SUCCESE;
}

static int I2cRead(DevHandle i2c_handle, unsigned char regAddr, unsigned char* data, int datalen)
{
    int ret = I2C_ERROR;
    struct I2cMsg writeMsg = {
        .addr = DEV_SLAVE_ADDR,
        .buf = &regAddr,
        .len = 1,
        .flags = 0,
    };
    ret = I2cTransfer(i2c_handle, &writeMsg, 1);
    if (ret != 0) {
        printf("I2cRead writeMsg failed\n");
        return I2C_ERROR;
    }

    struct I2cMsg readMsg = {
        .addr = DEV_SLAVE_ADDR,
        .buf = data,
        .len = datalen,
        .flags = I2C_FLAG_READ,
    };
    ret = I2cTransfer(i2c_handle, &readMsg, 1);
    if (ret != 0) {
        printf("I2cRead readMsg failed\n");
        return I2C_ERROR;
    }
    return I2C_SUCCESE;
}

static void* HdfI2CTestEntry(uint32_t arg)
{
    (void*)arg;
    int32_t ret;
    DevHandle i2c_handle = NULL;
    char data[BUFF_LEN] = {0};
    i2c_handle = I2cOpen(I2C_NUM);  // 打开 I2C
    if (i2c_handle == NULL) {
        printf("I2c %d Open fail!!\n", I2C_NUM);
        return NULL;
    }
    I2cRead(i2c_handle, REG_RD_ADDR, data, BUFF_LEN);   // 从寄存器地址读取两个字节

    I2cWrite(i2c_handle, REG_WR_ADDR, 0x66);            // 往寄存器地址中写入一个字节

    I2cClose(i2c_handle);
}

void StartHdfI2CTest(void)
{
    osThreadAttr_t attr;
    attr.name = HDF_I2C_TASK_NAME;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HDF_I2C_STACK_SIZE;
    attr.priority = HDF_I2C_TASK_PRIORITY;

    if (osThreadNew((osThreadFunc_t)HdfI2CTestEntry, NULL, &attr) == NULL) {
        printf("Failed to create %d!\n", HDF_I2C_TASK_NAME);
    }
}

OHOS_APP_RUN(StartHdfI2CTest);