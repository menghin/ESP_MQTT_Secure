/* test_mqtt_sensor_data.c: Implementation of a mqtt_sensor_data component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <limits.h>
#include "unity.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "mqtt_sensor_wifi.h"
#include "mqtt_sensor_mqtt.h"
#include <esp_wifi.h>
#include <esp_system.h>

#define USE_SECRET_LOCAL

#ifdef USE_SECRET_LOCAL
#include "mqtt_sensor_wifi_secret_local.h"
#include "mqtt_sensor_mqtt_secret_local.h"
#else
/* Used WIFI SSID*/
#define ESP_WIFI_SSID "WIFI_SSID"
/* Used WIFI password*/
#define ESP_WIFI_PASS "WIFI_PASS"
/* Used WIFI Channel*/
#define ESP_WIFI_CHANNEL 0

#define LOCATION "location"
#define HOSTNAME LOCATION "hostname"

const char *MQTT_URI = "URI";
const char *MQTT_USER = "user";    // leave blank if no credentials used
const char *MQTT_PASS = "pasword"; // leave blank if no credentials used

#endif

TEST_CASE("mqtt_sensor_mqtt_connect is able to connect", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL};

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_connect wrong uri", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://127.0.0.2:8883",
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_connect wrong user", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = "wrongUser",
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_connect wrong password", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = "wrongPassword"};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_publish_result", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    struct sensor_data results;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_publish_result(&results));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_publish_result invalid input data", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    struct sensor_data results;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    for (int i = 0; i < 50; i++)
    {
        esp_fill_random((void *)&results, sizeof(struct sensor_data));
        mqtt_sensor_mqtt_publish_result(&results);
    }

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_publish_result input data pointer is NULL", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_mqtt_publish_result(NULL));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_publish_message", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    char *message = "The test message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_publish_message(message));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_mqtt_publish_result input data pointer is NULL", "[mqtt_sensor_mqtt]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message));

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_mqtt_publish_message(NULL));

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_mqtt_disconnect());

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}
