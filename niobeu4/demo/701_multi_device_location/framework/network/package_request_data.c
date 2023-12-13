/*
 * Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
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

#include "package_request_data.h"
#include "cJSON.h"
#include "http.h"
#include "multi_device_location_base_data.h"
#include "multi_device_location_error.h"
#include "securec.h"

#define LOCATION POST_URL "http://192.168.18.158:19501/mdclbs/v1/algorithm/three_points_location"
#define CONFIG_POST_URL "http://192.168.18.158:19501/mdclbs/v1/device/list_of_place"
#define REPONSE_BUF_SIZE 512

char g_deviceArray[DEVICE_LOCATION_ROW_NUMBER][DEVICE_LOCATION_COL_NUMBER] = {0};
char **g_array = NULL; // for
#define MAX_DEVICE_NUM 3
#define UNKOWN_DEVICE_ID 123456

// 获取的deviceRssi
char *PackageRssiPostMeteData(DeviceRssiInfo *deviceRssiInfo, size_t deviceSize)
{
    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        return NULL;
    }
    cJSON_AddNumberToObject(root, "unknow_device_id", UNKOWN_DEVICE_ID);

    cJSON *row = cJSON_CreateArray();
    for (int i = 0; i < MAX_DEVICE_NUM; i++) {
        cJSON *rowData = cJSON_CreateObject();
        cJSON_AddStringToObject(rowData, "know_device_id", deviceRssiInfo[i].deviceId);
        cJSON_AddNumberToObject(rowData, "rssi", deviceRssiInfo[i].rssi);
        cJSON_AddItemToArray(row, rowData);
    }

    cJSON_AddItemToObject(root, "point_list", row);
    char *buf = cJSON_Print(root);
    cJSON_Delete(root);

    return buf;
}

int PostRequestTask(char *postbuf, size_t bufsize, char *reponseBuf, size_t reponseBufSize, const char *url)
{
    http_client_t client = {
        .header = "Connection: close\r\n",
    };

    http_client_data_t clientData = {0};

    clientData.post_buf = postbuf;
    clientData.post_buf_len = bufsize;

    clientData.response_buf = reponseBuf;
    clientData.response_buf_len = reponseBufSize;

    char *contentType = "application/json";
    clientData.post_content_type = contentType;

    printf("%s, response_buf is %s-------------------------------\n", __func__, clientData.post_buf);
    printf("%s, url is %s---------------------------\n", __func__, url);
    int ret = http_client_post(&client, url, &clientData);
    if (ret != HTTP_SUCCESS) {
        printf("http_client_post failed, error code = %d\n", ret);
    }
    printf("reponse buf is %s\n", clientData.response_buf);

    return ret;
}

int ParseReponseRssiData(char *buf, size_t size, Location *result)
{
    cJSON *root = cJSON_Parse(buf);
    if (root == NULL) {
        printf("cjson parse error\n");
        return MDL_PARAM_ERROR;
    }

    cJSON *data = cJSON_GetObjectItem(root, "code");
    char *codeValue = data->valuestring;
    if (strcmp(codeValue, "0") == 0) {
        data = cJSON_GetObjectItem(root, "data");
        if (data == NULL) {
            printf("parse data error\n");

            return MDL_PARAM_ERROR;
        }
        cJSON *locationData = cJSON_GetObjectItem(data, "x");
        result->x = locationData->valuedouble;
        locationData = cJSON_GetObjectItem(data, "y");
        result->y = locationData->valuedouble;
    } else {
        // failed
    }

    cJSON_Delete(root);
    return MDL_SUCCESS;
}

char *PackageTargertDeviceId(char *localDeviceId)
{
    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        return NULL;
    }
    cJSON_AddStringToObject(root, "deviceId", localDeviceId);

    char *buf = cJSON_Print(root);
    cJSON_Delete(root);

    return buf;
}

// 解包获取目标设备
char **ParseTargetDeviceId(char *buf, size_t size)
{
    printf("ParseTargetDeviceId entry ---------------\n");
    cJSON *root = cJSON_Parse(buf);

    if (root == NULL) {
        printf("cjson parse error\n");
        return NULL;
    }

    cJSON *data = cJSON_GetObjectItem(root, "code");
    if (data == NULL) {
        printf("data is NULL------------\n");
        return NULL;
    }
    char *codeValue = data->valuestring;
    if (codeValue == NULL) {
        printf("code Value is NULL---------------\n");
        return NULL;
    }
    if (strcmp(codeValue, "0") == 0) {
        data = cJSON_GetObjectItem(root, "data");
        if (data == NULL) {
            printf("parse data error\n");
            return NULL;
        }
        cJSON *DeviceList = cJSON_GetObjectItem(data, "device_id_list");
        if (DeviceList == NULL) {
            printf("parse data error\n");
            return NULL;
        }
        int arrayNum = cJSON_GetArraySize(DeviceList);
        for (int i = 0; i < MAX_DEVICE_NUM && i < arrayNum; i++) {
            cJSON *rowData = cJSON_GetArrayItem(DeviceList, i);
            if (rowData == NULL) {
                break;
            }
            memcpy_s(g_deviceArray[i], DEVICE_LOCATION_COL_NUMBER, rowData->valuestring, strlen(rowData->valuestring));
        }
    } else {
        // failed
    }

    cJSON_Delete(root);
    if (g_array != NULL) {
        g_array = (char **)malloc(DEVICE_LOCATION_ROW_NUMBER);
        for (int i = 0; i < DEVICE_LOCATION_ROW_NUMBER; i++) {
            g_array[i] = g_deviceArray[i];
        }
    }
    for (int i = 0; i < DEVICE_LOCATION_ROW_NUMBER; i++) {
        if (g_array[i] != NULL) {
            printf("%s : g_deviceArray  i is %s\n", __func__, g_array[i]);
        }
    }

    return g_array;
}