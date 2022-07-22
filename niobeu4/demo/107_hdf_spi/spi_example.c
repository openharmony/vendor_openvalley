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
#include <spi_if.h>
#include "cmsis_os2.h"
#include "ohos_run.h"

#define HDF_SPI_STACK_SIZE 0x1000
#define HDF_SPI_TASK_NAME "hdf_i2c_test_task"
#define HDF_SPI_TASK_PRIORITY 25
#define HDF_SPI_DELAY_TICKS 100

#define HDF_SPI_SUCCESS 1
#define HDF_SPI_FAIL    0

#define I2C_NUM 0
#define BUFF_SIZE 64

uint8_t rxBuffer[BUFF_SIZE] = { 0 };
uint8_t txBuffer[BUFF_SIZE] = "welcome to use OpenHarmony\n";

static void BufferRead(DevHandle spiHandle, uint8_t* buf, uint32_t size)
{
    int32_t ret = 0;
    uint8_t wbuf[4] = { 0x03, 0x00, 0x00, 0x00 };
    uint8_t rbuf[4] = { 0 };
    struct SpiMsg msg = { 0 };
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 1;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }
    uint8_t* wbuf1 = (uint8_t*)OsalMemAlloc(size);
    if (wbuf1 == NULL) {
        HDF_LOGE("OsalMemAlloc failed.\n");
        return;
    }
    memset_s(wbuf1, size, 0xff, size);
    msg.wbuf = wbuf1;
    msg.rbuf = buf;
    msg.len = size;
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }
    OsalMemFree(wbuf1);
}

static void BufferWrite(DevHandle spiHandle, const uint8_t* buf, uint32_t size)
{
    uint8_t wbuf[4] = { 0x02, 0x00, 0x00, 0x00 };
    uint8_t rbuf[4] = { 0 };
    uint8_t* rbuf1 = NULL;
    int32_t ret = 0;

    struct SpiMsg msg = { 0 };
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 1;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    rbuf1 = (uint8_t*)OsalMemAlloc(size);
    if (rbuf1 == NULL) {
        HDF_LOGE("OsalMemAlloc failed.\n");
        return;
    }

    memset_s(rbuf1, size, 0, size);
    msg.wbuf = buf;
    msg.rbuf = rbuf1;
    msg.len = size;
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }
    OsalMemFree(rbuf1);
}

static uint8_t BufferCmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--) {
        if (*pBuffer1 != *pBuffer2) {
            return HDF_SPI_FAIL;
        }
        pBuffer1++;
        pBuffer2++;
    }
    return HDF_SPI_SUCCESS;
}

static void* HdfSPITestEntry(uint32_t arg)
{
    (void*)arg;
    int32_t ret;

    struct SpiDevInfo spiDevinfo;
    spiDevinfo.busNum = 0;
    spiDevinfo.csNum = 0;
    DevHandle spiHandle = SpiOpen(&spiDevinfo);
    if (spiHandle == NULL) {
        HDF_LOGE("SpiOpen: failed\n");
        return NULL;
    }

    BufferWrite(spiHandle, txBuffer, strlen(txBuffer));
    HDF_LOGI("Write buffer is %s\n", txBuffer);

    BufferRead(spiHandle, rxBuffer, BUFF_SIZE);
    HDF_LOGI("Read buffer is %s\n", rxBuffer);

    if (BufferCmp(txBuffer, rxBuffer, strlen(txBuffer))) {
        HDF_LOGI("HDF Spi Write and Read Flash success!\n");
    } else {
        HDF_LOGI("HDF Spi Write and Read Flash failed!\n");
    }
    SpiClose(spiHandle);
    return NULL;
}

void StartHdfSPITest(void)
{
    osThreadAttr_t attr;
    attr.name = HDF_SPI_TASK_NAME;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HDF_SPI_STACK_SIZE;
    attr.priority = HDF_SPI_TASK_PRIORITY;

    if (osThreadNew((osThreadFunc_t)HdfSPITestEntry, NULL, &attr) == NULL) {
        printf("Failed to create %d!\n", HDF_SPI_TASK_NAME);
    }
}

OHOS_APP_RUN(StartHdfSPITest);