#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mqtt_client.h"

#define WIFI_SSID      "MANJU_WIFI"
#define WIFI_PASS      "1234567890"

static const char *TAG = "MQTT_APP";

/* Global MQTT client handle */
static esp_mqtt_client_handle_t global_client = NULL;
static int sequence = 0;

/* ---------- MQTT EVENT HANDLER ---------- */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client,
            "edgepulse/manjunatha144/device_001/cmd", 0);

        ESP_LOGI(TAG, "Subscribed to CMD topic, msg_id=%d", msg_id);

        global_client = client;
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

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

/* ---------- MAIN ---------- */
void app_main(void)
{
    ESP_LOGI(TAG, "Starting MQTT example");

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Start WiFi */
    wifi_init();

    /* Wait for WiFi connection */
    vTaskDelay(pdMS_TO_TICKS(5000));

    /* MQTT Configuration */
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);

    esp_mqtt_client_start(client);

    /* Publish every 5 seconds */
    while (1) {

        if (global_client != NULL) {

            char payload[128];

            sequence++;

            sprintf(payload,
                "{\"device_id\":\"device_001\",\"seq\":%d,\"uptime_ms\":%ld,\"temp\":27.5}",
                sequence,
                esp_log_timestamp());

            esp_mqtt_client_publish(global_client,
                "edgepulse/manjunatha144/device_001/telemetry",
                payload,
                0, 0, 0);

            ESP_LOGI(TAG, "Telemetry sent: %s", payload);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}