#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "LCD_SYSTEM";

#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0 //I2C PORT 0
#define I2C_MASTER_FREQ_HZ 100000 //100KHZ -STANDARD
#define I2C_MASTER_TX_BUF_DISABLE 0 //MASTER DOESNT NEED A BUFFER
#define I2C_MASTER_RX_BUF_DISABLE 0
#define LCD_ADDR 0x27

#define POT_ADC_CHANNEL ADC_CHANNEL_3
#define BOOT_BUTTON GPIO_NUM_0

typedef struct {
    int  raw_adc;
    uint32_t voltage_mv;
} sensor_data_t;

QueueHandle_t lcd_queue;

void lcd_send_cmd(char cmd) {
    char data_u,data_l;
    uint8_t data_t[4];
    data_u = (cmd & 0xf0);
    data_l = ((cmd << 4) & 0xf0);
    data_t[0] = data_u | 0x0C;
    
}



void i2c_master_init(){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = I2C_MASTER_FREQ_HZ },
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode,0,0,0);
}

void scanner_task(void *pvParameters){
    i2c_master_init();
    ESP_LOGI(TAG,"I2C Scanner started on SDA:21, SCL:22");

    while(1){
        int devices_found = 0;
        for(int i=1; 1<127; i++){
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd,(i<<1) | I2C_MASTER_WRITE,true);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM,cmd,pdMS_TO_TICKS(10));
            i2c_cmd_link_delete(cmd);

            if(ret == ESP_OK){
                ESP_LOGW(TAG,"Found device at address: 0x%02X",i);
                devices_found++;
            }
        }
        if(devices_found == 0) ESP_LOGE(TAG,"NO I2C devices found!");

        vTaskDelay(pdMS_TO_TICKS(50000)); // scan every 5 seconds
    }
}

void app_main(void)
{
    xTaskCreate(scanner_task,"I2C_SCAN",4096,NULL,5,NULL);
}
