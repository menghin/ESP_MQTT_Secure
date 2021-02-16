/* mqtt_sensor_wifi
*/

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include "esp_tls.h"
#include "cJSON.h"

#include <mqtt_sensor_mqtt.h>

static char *mqtt_pub_topic_to_subscribe;

static const char *TAG = "mqtt_sensor_mqtt";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_mqtt_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - error occurred
 * - message was published
 * - we got disconnected */
#define MQTT_CONNECTED_BIT BIT0
#define MQTT_ERROR_BIT BIT1
#define MQTT_PUBLISHED_BIT BIT2
#define MQTT_DISCONNECTED_BIT BIT3

static esp_mqtt_client_handle_t client;

static esp_err_t mqtt_sensor_mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, mqtt_pub_topic_to_subscribe, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupSetBits(s_mqtt_event_group, MQTT_DISCONNECTED_BIT);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        xEventGroupSetBits(s_mqtt_event_group, MQTT_PUBLISHED_BIT);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS)
        {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
        }
        else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
        {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        }
        else
        {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        xEventGroupSetBits(s_mqtt_event_group, MQTT_ERROR_BIT);
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_sensor_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_sensor_mqtt_event_handler_cb(event_data);
}

esp_err_t mqtt_sensor_mqtt_connect(esp_mqtt_client_config_t mqtt_cfg, char *mqtt_pub_topic)
{
    esp_err_t status = ESP_FAIL;

    mqtt_pub_topic_to_subscribe = mqtt_pub_topic;

    s_mqtt_event_group = xEventGroupCreate();

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_sensor_mqtt_event_handler, client) == ESP_OK)
    {
        if (esp_mqtt_client_start(client) == ESP_OK)
        {
            /* Waiting until either the connection is established (MQTT_CONNECTED_BIT) or connection failed (MQTT_ERROR_BIT). The bits are set by event_handler() (see above) */
            EventBits_t bits = xEventGroupWaitBits(s_mqtt_event_group,
                                                   MQTT_CONNECTED_BIT | MQTT_ERROR_BIT,
                                                   pdFALSE,
                                                   pdFALSE,
                                                   150000);

            /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
            if (bits & MQTT_CONNECTED_BIT)
            {
                status = ESP_OK;
            }
        }
    }

    return status;
}

esp_err_t mqtt_sensor_mqtt_disconnect(void)
{
    esp_err_t status = ESP_OK;

    esp_mqtt_client_destroy(client);

    return status;
}

esp_err_t mqtt_sensor_mqtt_publish(struct sensor_data *results)
{
    esp_err_t status = ESP_FAIL;

    if (results == NULL)
    {
        return status;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", results->timestamp);
    cJSON_AddNumberToObject(root, "temperature", results->temperature);
    cJSON_AddNumberToObject(root, "humidity", results->humidity);
    cJSON_AddNumberToObject(root, "pressure", results->pressure);
    cJSON_AddNumberToObject(root, "gasResistance", results->gasResistance);
    const char *payload = cJSON_Print(root);

    if (esp_mqtt_client_publish(client, mqtt_pub_topic_to_subscribe, payload, 0, 0, 0) != -1)
    {
        ESP_LOGI(TAG, "sent publish successful");

        /* Waiting until either the message was published (MQTT_PUBLISHED_BIT) or connection failed (MQTT_ERROR_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_mqtt_event_group,
                                               MQTT_PUBLISHED_BIT | MQTT_ERROR_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               150000);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
        if (bits & MQTT_PUBLISHED_BIT)
        {
            status = ESP_OK;
        }
    }

    return status;
}
