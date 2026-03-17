#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "UART_LAB";

#define LED_GPIO GPIO_NUM_2
#define UART_NUM UART_NUM_0 //USB SERIAL PORT
#define BUF_SIZE (1024)

QueueHandle_t uart_queue;

// --- TASK: The Receiver (Listens to UART) ---
void uart_rx_task(void *pvParameters){
    uart_config_t uart_config = {
        .baud_rate = 115300,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    //INSTALL DRIVER AND SET PINS
    uart_driver_install(UART_NUM,BUF_SIZE * 2,0,0,NULL,0);
    uart_param_config(UART_NUM,&uart_config);
    uart_set_pin(UART_NUM,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE);

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1){
        int len = uart_read_bytes(UART_NUM,data,BUF_SIZE,pdMS_TO_TICKS(100));
        if(len>0){
            data[len] = '\0';//NULL TERMINATE THE STRING
            ESP_LOGI(TAG,"RECEIVED: %s",(char *)data);

            char command = data[0];
            xQueueSend(uart_queue,&command,portMAX_DELAY);
        }
    }
    
}

void led_task(void *pvParameters){
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO,GPIO_MODE_OUTPUT);

    char received_cmd;
    while(1){
        if(xQueueReceive(uart_queue,&received_cmd,portMAX_DELAY)){
            if(received_cmd == '1'){
                gpio_set_level(LED_GPIO,1);
                ESP_LOGW(TAG,"LED TURNED ON VIA UART");
            }else if(received_cmd == '0'){
                gpio_set_level(LED_GPIO,0);
                ESP_LOGW(TAG,"LED TURNED OFF VIA UART");
            }
        }
    }
}

void app_main(void)
{
    uart_queue = xQueueCreate(10,sizeof(char));
    xTaskCreate(uart_rx_task,"UART_RX",4096,NULL,5,NULL);
    xTaskCreate(led_task,"LED_EXE",2048,NULL,5,NULL);
}
