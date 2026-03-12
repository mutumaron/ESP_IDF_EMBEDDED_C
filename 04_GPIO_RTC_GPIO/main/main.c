#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_15
#define BUZZER_GPIO GPIO_NUM_2
#define GREEN_LED_GPIO GPIO_NUM_4
#define RED_LED_GPIO GPIO_NUM_5



void app_main(void)
{
    //SETUP GPIO

    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO,GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_GPIO); // Enable pull-up resistor for the button GPIO ....pressed=0,not pressed=1


    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO,GPIO_MODE_OUTPUT);

    gpio_reset_pin(GREEN_LED_GPIO);
    gpio_set_direction(GREEN_LED_GPIO,GPIO_MODE_OUTPUT);

    gpio_reset_pin(RED_LED_GPIO);
    gpio_set_direction(RED_LED_GPIO,GPIO_MODE_OUTPUT);

    printf("Vault System Initialized. Hold button for 3s to unlock...\n");

    while(1)
    {
        if(gpio_get_level(BUTTON_GPIO) == 0) {
            uint8_t counter = 0;
            bool success = false;

            while(gpio_get_level(BUTTON_GPIO) == 0){
                vTaskDelay(pdMS_TO_TICKS(1000)); //WAIT FOR 1 SECOND
                counter++;
                
                if(counter < 3){
                    gpio_set_level(RED_LED_GPIO,1);
                    gpio_set_level(BUZZER_GPIO,1);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    gpio_set_level(RED_LED_GPIO,0);
                    gpio_set_level(BUZZER_GPIO,0);
                    printf("Holding... %d seconds\n", counter);
                }
                else if(counter == 3){
                    success = true;
                    break;
                }
            }
            if(success){
                gpio_set_level(GREEN_LED_GPIO,1);
                printf("Vault unlocked!\n");

                //success melody
                for(int i=0; i<3; i++){
                    gpio_set_level(BUZZER_GPIO,1);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    gpio_set_level(BUZZER_GPIO,0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            } else {
                printf("Button released too soon. Try again.\n");
                gpio_set_level(RED_LED_GPIO,1);
                gpio_set_level(BUZZER_GPIO,1);
                vTaskDelay(pdMS_TO_TICKS(500));
                gpio_set_level(RED_LED_GPIO,0);
                gpio_set_level(BUZZER_GPIO,0);
            }
            while(gpio_get_level(BUTTON_GPIO) == 0){vTaskDelay(pdMS_TO_TICKS(10));}

        }

      vTaskDelay(pdMS_TO_TICKS(100)); // Debounce delay
    }  

}
