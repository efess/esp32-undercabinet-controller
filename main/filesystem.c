#include "esp_log.h"
#include "esp_vfs_semihost.h"
#include "esp_littlefs.h"
#include "./filesystem.h"

static const char *TAG = "fs";

#if CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST

esp_err_t init_fs(void)
{
    esp_err_t ret = esp_vfs_semihost_register(CONFIG_CABINET_LIGHTING_WEB_MOUNT_POINT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register semihost driver (%s)!", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    return ESP_OK;
}

#else

esp_err_t init_fs(void)
{
    esp_vfs_littlefs_conf_t  conf = {
        .base_path = CONFIG_CABINET_LIGHTING_WEB_MOUNT_POINT,
        .partition_label = "storage",
        .dont_mount = false,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        ret =  esp_littlefs_format(conf.partition_label);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to format LittleFS partition (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        }
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

#endif

esp_err_t fs_read_file(const char *path, char **buf, size_t *len)
{
    char fixedPath[256];
    snprintf(fixedPath, sizeof(fixedPath), "%s/%s", CONFIG_CABINET_LIGHTING_WEB_MOUNT_POINT, path);

    FILE *f = fopen(fixedPath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buf = malloc(*len);
    if (*buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer for file");
        fclose(f);
        return ESP_FAIL;
    }

    if (fread(*buf, 1, *len, f) != *len) {
        ESP_LOGE(TAG, "Failed to read file");
        fclose(f);
        free(*buf);
        return ESP_FAIL;
    }

    fclose(f);
    return ESP_OK;
}

esp_err_t fs_write_file (const char *path, const char *buf, size_t len)
{
    char fixedPath[256];
    snprintf(fixedPath, sizeof(fixedPath), "%s/%s", CONFIG_CABINET_LIGHTING_WEB_MOUNT_POINT, path);
    
    FILE *f = fopen(fixedPath, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    if (fwrite(buf, 1, len, f) != len) {
        ESP_LOGE(TAG, "Failed to write file");
        fclose(f);
        return ESP_FAIL;
    }

    fclose(f);
    return ESP_OK;
}