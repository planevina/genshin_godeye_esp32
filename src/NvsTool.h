#ifndef NVS_TOOL_H
#define NVS_TOOL_H

#include "nvs.h"
#include "esp_system.h"
#include "nvs_flash.h"

#define STORAGE_NAMESPACE "storage"

static uint8_t getNvsValueU8(const char *key, uint8_t default_value = 0)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    uint8_t v = default_value;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_get_u8(st_handle, key, &v);
        if (err == ESP_OK)
        {
        }
        else if (err == 0x1102)
        {
            printf("[NVS]: Key '%s' not found,use default value:'%d'\n", key ,default_value);
        }
        else
        {
            printf("get err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return v;
}

static uint32_t getNvsValueU32(const char *key, uint32_t default_value = 0)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    uint32_t v = default_value;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_get_u32(st_handle, key, &v);
        if (err == ESP_OK)
        {
        }
        else if (err == 0x1102)
        {
            printf("[NVS]: Key '%s' not found,use default value:'%d'\n", key ,default_value);
        }
        else
        {
            printf("get err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return v;
}

static uint64_t getNvsValueU64(const char *key, uint64_t default_value = 0)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    uint64_t v = default_value;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_get_u64(st_handle, key, &v);
        if (err == ESP_OK)
        {
        }
        else if (err == 0x1102)
        {
            printf("[NVS]: Key '%s' not found,use default value:'%d'\n", key ,default_value);
        }
        else
        {
            printf("get err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return v;
}

static bool getNvsValueStr(const char *key, char *v, size_t *len)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    bool r = false;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_get_str(st_handle, key, v, len);
        if (err == ESP_OK)
        {
            r = true;
        }
        else if (err == 0x1102)
        {
            printf("[NVS]: Key '%s' not found\n", key);
        }
        else
        {
            printf("get err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return r;
}

static bool setNvsValueU8(const char *key, uint8_t v)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    bool r = false;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_set_u8(st_handle, key, v);
        if (err == ESP_OK)
        {
            err = nvs_commit(st_handle);
            if (err == ESP_OK)
            {
                r = true;
            }
            else
            {
                printf("nvs_commit Failed!\n");
            }
        }
        else
        {
            printf("set err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return r;
}

static bool setNvsValueStr(const char *key, const char *v)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    bool r = false;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_set_str(st_handle, key, v);
        if (err == ESP_OK)
        {
            err = nvs_commit(st_handle);
            if (err == ESP_OK)
            {
                r = true;
            }
            else
            {
                printf("nvs_commit Failed!\n");
            }
        }
        else
        {
            printf("set err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return r;
}

static bool setNvsValueU32(const char *key, uint32_t v)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    bool r = false;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_set_u32(st_handle, key, v);
        if (err == ESP_OK)
        {
            err = nvs_commit(st_handle);
            if (err == ESP_OK)
            {
                r = true;
            }
            else
            {
                printf("nvs_commit Failed!\n");
            }
        }
        else
        {
            printf("set err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return r;
}

static bool setNvsValueU64(const char *key, uint64_t v)
{
    esp_err_t err;
    nvs_handle_t st_handle;
    bool r = false;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &st_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_set_u64(st_handle, key, v);
        if (err == ESP_OK)
        {
            err = nvs_commit(st_handle);
            if (err == ESP_OK)
            {
                r = true;
            }
            else
            {
                printf("nvs_commit Failed!\n");
            }
        }
        else
        {
            printf("set err =0x%x\n", err);
        }
    }
    nvs_close(st_handle);
    return r;
}

#endif