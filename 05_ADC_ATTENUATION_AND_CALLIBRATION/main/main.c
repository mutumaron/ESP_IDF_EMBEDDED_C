#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "ADC APP";

#define LED_PIN GPIO_NUM_2
//GPIO 15 IS CHANNEL 3 ON ADC_UNIT_2
#define POT_ADC_UNIT ADC_UNIT_2
#define POT_ADC_CHANNEL ADC_CHANNEL_3

void app_main(void)
{
    //USE EXPLICIT BIT-WIDTH TYPES FOR GPIO SETUP
    const gpio_num_t led_pin = LED_PIN;

    gpio_reset_pin(led_pin);
    gpio_set_direction(led_pin,GPIO_MODE_OUTPUT);

    //ADC HANDLE AND CONFIGURATION
    //unit
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = POT_ADC_UNIT,
    };

    adc_oneshot_new_unit(&init_config2, &adc2_handle);

    //bitwidth and attenuation
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12, //ATTENUATION CONFIGURATION FOR ADC CHANNEL 
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
    };
    //channel configuration
    adc_oneshot_config_channel(adc2_handle,POT_ADC_CHANNEL, &config);

    int raw_val = 0;
    ESP_LOGI(TAG,"System optimized. ADC2 Initialized on GPIO 15.");

    while(1) {
        //Read raw value (voltage from potentiometer)
        adc_oneshot_read(adc2_handle,POT_ADC_CHANNEL, &raw_val);

        uint32_t voltage_mv = (uint32_t)(raw_val * 3300)/4095;
        uint32_t delay_ms = 500 - (raw_val * 480 / 4095);

        ESP_LOGI(TAG,"Raw: %d | Voltage: %dlu mV | Status: %s",
             raw_val,
             voltage_mv,
              voltage_mv >2500 ? "High" : "Low");
        
        gpio_set_level(led_pin,1);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        gpio_set_level(led_pin,0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

    }


}
