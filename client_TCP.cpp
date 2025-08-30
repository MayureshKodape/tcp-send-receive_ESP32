
// CLIENT 
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "esp_timer.h"

#define WIFI_SSID       "Mayuresh"
#define WIFI_PASS       "12345678"
#define SERVER_IP       "10.180.196.150"  // <-- static server IP
#define SERVER_PORT     3333

static const char *TAG = "TCP_CLIENT";
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/* WiFi event handler */
static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
         

    


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi SSID:%s ...", WIFI_SSID);
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

/* TCP Client Task */
static void tcp_client_task(void *pv) {
    while (1) {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket() failed: errno=%d", errno);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        struct sockaddr_in srv = {0};
        srv.sin_family = AF_INET;
        srv.sin_port = htons(SERVER_PORT);
        srv.sin_addr.s_addr = inet_addr(SERVER_IP);

        ESP_LOGI(TAG, "Connecting to %s:%d ...", SERVER_IP, SERVER_PORT);
        if (connect(sock, (struct sockaddr *)&srv, sizeof(srv)) != 0) {
            ESP_LOGE(TAG, "connect() failed: errno=%d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        ESP_LOGI(TAG, "Connected! Starting RTT test...");

        while (1) {
            uint32_t t1 = (uint32_t)(esp_timer_get_time() / 1000); // ms
            send(sock, &t1, sizeof(t1), 0);

            uint32_t t2;
            int len = recv(sock, &t2, sizeof(t2), 0);
            if (len <= 0) break;

            uint32_t t3 = (uint32_t)(esp_timer_get_time() / 1000); // ms
            uint32_t rtt = t3 - t1;

            ESP_LOGI(TAG, "RTT=%u ms | server_time=%u | client_sent=%u", rtt, t2, t1);
            vTaskDelay(pdMS_TO_TICKS(1000)); // 1s interval
        }

        shutdown(sock, 0);
        close(sock);
        ESP_LOGW(TAG, "Disconnected. Reconnecting soon...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

extern "C" void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
}
