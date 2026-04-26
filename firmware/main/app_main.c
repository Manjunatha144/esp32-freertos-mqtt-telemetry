#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"

#include "mqtt_client.h"

#define WIFI_SSID      "MANJU_WIFI"
#define WIFI_PASS      "1234567890"

#define CMD_TOPIC       "edgepulse/manjunatha144/device_001/cmd"
#define TELEMETRY_TOPIC "edgepulse/manjunatha144/device_001/telemetry"

#define MIN_INTERVAL_MS 500
#define MAX_INTERVAL_MS 60000

static const char *TAG = "MQTT_APP";

/* Shared state */
static volatile bool mqtt_connected = false;
static volatile uint32_t interval_ms = 5000;
static esp_mqtt_client_handle_t global_client = NULL;

/* ---------- MQTT EVENT HANDLER ---------- */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        global_client = client;
        esp_mqtt_client_subscribe(client, CMD_TOPIC, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connected = false;
        global_client = NULL;
        break;

    case MQTT_EVENT_DATA: {
        char buffer[128];
        int len = event->data_len < sizeof(buffer) - 1 ? event->data_len : sizeof(buffer) - 1;
        memcpy(buffer, event->data, len);
        buffer[len] = '\0';

        ESP_LOGI(TAG, "CMD received: %s", buffer);

        char *pos = strstr(buffer, "interval_ms");
        if (pos) {
            char *colon = strchr(pos, ':');
            if (colon) {
                uint32_t new_interval = atoi(colon + 1);
                if (new_interval >= MIN_INTERVAL_MS && new_interval <= MAX_INTERVAL_MS) {
                    interval_ms = new_interval;
                    ESP_LOGI(TAG, "interval updated to %lu ms", interval_ms);
                } else {
                    ESP_LOGW(TAG, "Invalid interval value");
                }
            }
        }
        break;
    }

    default:
        break;
    }
}

/* ---------- WIFI INIT ---------- */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/* ---------- TELEMETRY TASK ---------- */
static void telemetry_task(void *arg)
{
    uint32_t seq = 0;

    while (1) {
        if (mqtt_connected && global_client != NULL) {
            seq++;

            uint32_t uptime_ms = esp_timer_get_time() / 1000;

            char payload[128];
            snprintf(payload, sizeof(payload),
                     "{\"device_id\":\"device_001\",\"seq\":%lu,\"uptime_ms\":%lu,\"temp\":27.5}",
                     seq, uptime_ms);

            esp_mqtt_client_publish(global_client,
                                    TELEMETRY_TOPIC,
                                    payload,
                                    0, 0, 0);

            ESP_LOGI(TAG, "Telemetry sent: %s", payload);
        }

        vTaskDelay(pdMS_TO_TICKS(interval_ms));
    }
}

/* ---------- MAIN ---------- */
void app_main(void)
{
    ESP_LOGI(TAG, "Starting MQTT Telemetry (FreeRTOS task version)");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();

    vTaskDelay(pdMS_TO_TICKS(5000));  // allow WiFi connect

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(client,
                                       ESP_EVENT_ANY_ID,
                                       mqtt_event_handler,
                                       NULL)
    );

    ESP_ERROR_CHECK(esp_mqtt_client_start(client));

    xTaskCreate(telemetry_task,
                "telemetry_task",
                4096,
                NULL,
                5,
                NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}