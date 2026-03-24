#include <stdio.h>
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "WIFI_CONFIG";

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void* arg,esp_event_base_t event_base,
    int32_t even_id, void* event_data){
        if(event_base == WIFI_EVENT && even_id == WIFI_EVENT_STA_START){
            esp_wifi_connect();
        }else if(event_base == IP_EVENT && even_id == IP_EVENT_STA_GOT_IP){
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGW(TAG,"GOT IP: " IPSTR,IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(s_wifi_event_group,WIFI_CONNECTED_BIT);
        }
}

void wifi_init_sta(void){
    s_wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,NULL);
    esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&wifi_event_handler,NULL,NULL);

    wifi_config_t wifi_config ={
        .sta = {
            .ssid = "Ronny",
            .password ="Ronny2527",
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA,&wifi_config);
    esp_wifi_start();

}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES){
        nvs_flash_erase();
        ret = nvs_flash_init();
    }

    wifi_init_sta();

    xEventGroupWaitBits(s_wifi_event_group,WIFI_CONNECTED_BIT,pdFALSE,pdFALSE,portMAX_DELAY);
    ESP_LOGI(TAG,"ESP32 is Online.Ready for API Requests");

}
