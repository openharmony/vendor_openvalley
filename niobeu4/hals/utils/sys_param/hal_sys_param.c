/*
 * Copyright (c) 2022 OpenValley Digital Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "hal_sys_param.h"

static const char OHOS_DEVICE_TYPE[] = {"ESP32"};
static const char OHOS_DISPLAY_VERSION[] = {"01.00.00"};
static const char OHOS_MANUFACTURE[] = {"Openvalley"};
static const char OHOS_BRAND[] = {"Openvalley"};
static const char OHOS_MARKET_NAME[] = {"Niobeu4"};
static const char OHOS_PRODUCT_SERIES[] = {"Niobeu4"};
static const char OHOS_PRODUCT_MODEL[] = {"Niobeu4"};
static const char OHOS_SOFTWARE_MODEL[] = {"ESP32-IDF-OM"};
static const char OHOS_HARDWARE_MODEL[] = {"Niobeu4-OM"};
static const char OHOS_HARDWARE_PROFILE[] = {"wlan:true"};
static const char OHOS_BOOTLOADER_VERSION[] = {"bootloader"};
static const char OHOS_ABI_LIST[] = {"xtensa-liteos"};
static const char OHOS_SECURITY_PATCH_TAG[] = {"2022/3/18"};
static const char OHOS_SERIAL[] = {"1234567890"};
static const int OHOS_FIRST_API_VERSION = 1;

const char* HalGetSecurityPatchTag(void)
{
    return OHOS_SECURITY_PATCH_TAG;
}

const char *HalGetDeviceType(void)
{
    return OHOS_DEVICE_TYPE;
}

const char *HalGetManufacture(void)
{
    return OHOS_MANUFACTURE;
}

const char *HalGetBrand(void)
{
    return OHOS_BRAND;
}

const char *HalGetMarketName(void)
{
    return OHOS_MARKET_NAME;
}

const char *HalGetProductSeries(void)
{
    return OHOS_PRODUCT_SERIES;
}

const char *HalGetProductModel(void)
{
    return OHOS_PRODUCT_MODEL;
}

const char *HalGetSoftwareModel(void)
{
    return OHOS_SOFTWARE_MODEL;
}

const char *HalGetHardwareModel(void)
{
    return OHOS_HARDWARE_MODEL;
}

const char *HalGetHardwareProfile(void)
{
    return OHOS_HARDWARE_PROFILE;
}

const char *HalGetSerial(void)
{
    return OHOS_SERIAL;
}

const char *HalGetBootloaderVersion(void)
{
    return OHOS_BOOTLOADER_VERSION;
}

const char *HalGetAbiList(void)
{
    return OHOS_ABI_LIST;
}

const char *HalGetDisplayVersion(void)
{
    return OHOS_DISPLAY_VERSION;
}

const char *HalGetIncrementalVersion(void)
{
    return INCREMENTAL_VERSION;
}

const char *HalGetBuildType(void)
{
    return BUILD_TYPE;
}

const char *HalGetBuildUser(void)
{
    return BUILD_USER;
}

const char *HalGetBuildHost(void)
{
    return BUILD_HOST;
}

const char *HalGetBuildTime(void)
{
    return BUILD_TIME;
}

int HalGetFirstApiVersion(void)
{
    return OHOS_FIRST_API_VERSION;
}
