#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"


static const char *TAG = "FREE_RTOS_QUEUE_APP";

#define LED_GPIO GPIO_NUM_2
#define POT_ADC_UNIT ADC_UNIT_2
#define POT_ADC_CHANNEL ADC_CHANNEL_3

//creating a queue handle
QueueHandle_t delay_queue;

void sensor_task (void *pvParameters) {
   adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = POT_ADC_UNIT };
    adc_oneshot_new_unit(&init_config, &adc2_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc2_handle, POT_ADC_CHANNEL, &config);

    int raw_val = 0;
    while(1){
       adc_oneshot_read(adc2_handle, POT_ADC_CHANNEL, &raw_val);
        
        uint32_t data_to_send = (uint32_t)((raw_val * 1000) / 4095) + 50;

        xQueueSend(delay_queue, &data_to_send,portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//LEA=D & BUZZER TASK
void led_buzzer_task (void *pvParameters){
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO,GPIO_MODE_OUTPUT);

    uint32_t received_delay = 50;

    while(1){
        if(xQueueReceive(delay_queue,&received_delay,0) == pdPASS){
            ESP_LOGI(TAG,"New Delay received: %lu ms",received_delay);
        }
        gpio_set_level(LED_GPIO,1);
        vTaskDelay(pdMS_TO_TICKS(received_delay));
        gpio_set_direction(LED_GPIO,0);
        vTaskDelay(pdMS_TO_TICKS(received_delay));
    }
}

void app_main(void)
{
    delay_queue = xQueueCreate(10,sizeof(uint32_t));
    if(delay_queue != NULL){
        xTaskCreate(sensor_task,"SENSE",2046,NULL,5,NULL);
        xTaskCreate(led_buzzer_task,"BLINK",2046,NULL,5,NULL);
    }else{
        ESP_LOGI(TAG,"Failed to create QUEUE");
    }

}
