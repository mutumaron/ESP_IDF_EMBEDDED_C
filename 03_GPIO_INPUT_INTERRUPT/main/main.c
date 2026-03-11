#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "esp_log.h"

#define INTERRUPT_PIN GPIO_NUM_5
uint16_t interrupt_count = 0;
bool button_state = false;

static void IRAM_ATTR gpio_isr_handler(void * arg)
{
    interrupt_count ++;
    button_state = true;

    gpio_isr_handler_add(INTERRUPT_PIN,gpio_isr_handler,NULL);
    gpio_intr_enable(INTERRUPT_PIN);

 
}

void app_main(void)
{
    gpio_reset_pin(INTERRUPT_PIN);
    gpio_set_direction(INTERRUPT_PIN, GPIO_MODE_INPUT);

    gpio_set_pull_mode(INTERRUPT_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(INTERRUPT_PIN,GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(INTERRUPT_PIN,gpio_isr_handler,NULL);

    gpio_intr_enable(INTERRUPT_PIN);

    while(1)
    {
        if(button_state == true)
        {
            printf("Interrupt count: %d\n", interrupt_count);
            button_state = false;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
