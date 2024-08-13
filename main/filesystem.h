#include <esp_err.h>

esp_err_t init_fs(void);

esp_err_t fs_write_file (const char *path, const char *buf, size_t len);
esp_err_t fs_read_file(const char *path, char **buf, size_t *len);