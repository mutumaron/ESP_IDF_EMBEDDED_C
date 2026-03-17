#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_log.h"


static const char *TAG = "ISR_LAB";

#define BUTTON_GPIO GPIO_NUM_5
#define LED_PIN GPIO_NUM_2
#define BUZZER_GPIO GPIO_NUM_4

SemaphoreHandle_t xInterruptSemaphore;

// 1. This is the Interrupt Service Routine (ISR)
// This function runs in a "special state" outside of normal tasks
static void IRAM_ATTR gpio_isr_handler(void* arg){
    // We use a special "FromISR" version of the semaphore give function
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xInterruptSemaphore,&xHigherPriorityTaskWoken);

    // If giving the semaphore woke a higher priority task, we force a context switch
    if(xHigherPriorityTaskWoken){
        portYIELD_FROM_ISR();
    }


}

// 3. The Deferred Task - This handles the "Heavy" logic
void alarm_task(void* pvParameters){
    while(1){
        if(xSemaphoreTake(xInterruptSemaphore,portMAX_DELAY)){
            ESP_LOGI(TAG,"INTERUPT RECEIVED ! EXECUTIUNG PROGRAMME");

            //mathematical logic
            for(int i=0; i<3; i++){
                gpio_set_level(LED_PIN,1);
                gpio_set_level(BUZZER_GPIO,1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_PIN,0);
                gpio_set_level(BUZZER_GPIO,0);
                vTaskDelay(pdTICKS_TO_MS(100));
            }
            // Debounce: Ignore extra triggers for 500ms
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}


void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN,GPIO_MODE_OUTPUT);
    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO,GPIO_MODE_OUTPUT);

    //we create the semaphore
    xInterruptSemaphore = xSemaphoreCreateBinary();

    //configure the button with the interrupt
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE; //TRIGGER ON PRESSED
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // 4. Install the Global GPIO ISR Service
    // This allocates the CPU's interrupt resources
    gpio_install_isr_service(0);

    // 5. Hook the specific ISR handler for our button
    gpio_isr_handler_add(BUTTON_GPIO,gpio_isr_handler,NULL);
    
    xTaskCreate(alarm_task,"ALARM TASK",2048,NULL,10,NULL);
    

}
