#include "i2c.h"

#include <string.h>
#include <stdio.h>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define I2C_SCL_IO          CONFIG_VE_I2C_SCL_IO
#define I2C_SDA_IO          CONFIG_VE_I2C_SDA_IO
#define I2C_PORT            I2C_NUM_0
#define I2C_FREQ_HZ         CONFIG_VE_I2C_FREQ_HZ
#define I2C_TIMEOUT_MS      50

static const char *TAG = "ve_i2c";

esp_err_t i2c_init(void)
{
    ESP_LOGI(TAG, "Bus initializing... SCL=%d SDA=%d FREQ=%dHz", I2C_SCL_IO, I2C_SDA_IO, I2C_FREQ_HZ);
    i2c_master_bus_config_t cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus = NULL;
    esp_err_t err = i2c_new_master_bus(&cfg, &bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Bus initialized successfully");
    ESP_LOGI(TAG, "I2C bus scan on SDA=%d SCL=%d", I2C_SDA_IO, I2C_SCL_IO);

    char line[128];

    // Header row
    strcpy(line, "    ");
    for (int i = 0; i < 16; i++) {
        char tmp[4];
        snprintf(tmp, sizeof(tmp), "%02X ", i);
        strncat(line, tmp, sizeof(line) - strlen(line) - 1);
    }
    ESP_LOGI(TAG, "%s", line);

    int found = 0;

    for (int high = 0; high < 8; high++) {
        snprintf(line, sizeof(line), "%02X: ", high << 4);

        for (int low = 0; low < 16; low++) {
            uint8_t addr = (high << 4) | low;
            char cell[4];

            if (addr < 0x03 || addr > 0x77) {
                snprintf(cell, sizeof(cell), "   ");
            } else {
                err = i2c_master_probe(bus, addr, I2C_TIMEOUT_MS);
                if (err == ESP_OK) {
                    snprintf(cell, sizeof(cell), "%02X ", addr);
                    found++;
                } else {
                    snprintf(cell, sizeof(cell), "-- ");
                }
            }

            strncat(line, cell, sizeof(line) - strlen(line) - 1);
        }

        ESP_LOGI(TAG, "%s", line);
    }

    ESP_LOGI(TAG, "Scan complete, found %d device(s)", found);

    err = i2c_del_master_bus(bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
