/* mqtt_sensor_wifi
*/

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "mqtt_sensor_wifi.h"

#define ESP_RETURN_ERROR(x)                  \
    do                                       \
    {                                        \
        esp_err_t __err_rc = (x);            \
        if (__err_rc != ESP_OK)              \
        {                                    \
            ESP_LOGE(TAG, "unexpected error occurred"); \
            mqtt_sensor_wifi_disconnect_to_sta(); \
            return ESP_FAIL;                 \
        }                                    \
    } while (0)

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* Created netif handler when connecting*/
static esp_netif_t *handler = NULL;

/* Configured wifi config*/
static mqtt_sensor_wifi_config_t *configured_mqtt_sensor_wifi_config;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define ESP_MAXIMUM_RETRY 0

static const char *TAG = "mqtt_sensor_wifi";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t mqtt_sensor_wifi_connect_to_sta(mqtt_sensor_wifi_config_t *mqtt_sensor_wifi_config)
{
    uint16_t number_of_scanned_ap = 1;
    esp_err_t status = ESP_FAIL;
    wifi_ap_record_t ap_info[number_of_scanned_ap];
    uint16_t ap_count = 0;
    nvs_stats_t nvs_stats;

    configured_mqtt_sensor_wifi_config = mqtt_sensor_wifi_config;

    if (nvs_get_stats(NULL, &nvs_stats) == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGE(TAG, "NVS needs to be initalized!");
        return status;
    }

    s_wifi_event_group = xEventGroupCreate();

    ESP_RETURN_ERROR(esp_netif_init());
    ESP_RETURN_ERROR(esp_event_loop_create_default());
    handler = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ERROR(esp_wifi_init(&cfg));

    memset(ap_info, 0, sizeof(ap_info));

    wifi_scan_config_t scan_config = {0};
    scan_config.ssid = configured_mqtt_sensor_wifi_config->ssid;
    scan_config.channel = configured_mqtt_sensor_wifi_config->channel;
    scan_config.scan_time.active.max = 10;
    scan_config.scan_time.passive = 10;

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = {0},
            .password = {0},
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .channel = configured_mqtt_sensor_wifi_config->channel,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    memcpy(&wifi_config.sta.ssid, &configured_mqtt_sensor_wifi_config->ssid, sizeof(configured_mqtt_sensor_wifi_config->ssid));
    memcpy(&wifi_config.sta.password, &configured_mqtt_sensor_wifi_config->password, sizeof(configured_mqtt_sensor_wifi_config->password));

    ESP_RETURN_ERROR(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_RETURN_ERROR(esp_wifi_start());
    ESP_RETURN_ERROR(esp_wifi_scan_start(&scan_config, true));
    ESP_RETURN_ERROR(esp_wifi_scan_get_ap_records(&number_of_scanned_ap, ap_info));
    ESP_RETURN_ERROR(esp_wifi_scan_get_ap_num(&ap_count));

    if (ap_count == 0)
    {
        ESP_LOGI(TAG, "AP not available.");
        return status;
    }

    ESP_RETURN_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_RETURN_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_RETURN_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    esp_wifi_connect();

    ESP_RETURN_ERROR(esp_wifi_start());

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s",
                 configured_mqtt_sensor_wifi_config->ssid);
        status = ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s",
                 configured_mqtt_sensor_wifi_config->ssid);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_RETURN_ERROR(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_RETURN_ERROR(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);

    return status;
}

esp_err_t mqtt_sensor_wifi_disconnect_to_sta()
{
    nvs_stats_t nvs_stats;

    if (nvs_get_stats(NULL, &nvs_stats) == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGE(TAG, "NVS needs to be initalized!");
        return ESP_OK;
    }

    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_loop_delete_default();

    if (handler != NULL)
    {
        esp_netif_destroy(handler);
    }

    return ESP_OK;
}
