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
#include <esp_wifi.h>

#define USE_SECRET_LOCAL

#ifdef USE_SECRET_LOCAL
#include "mqtt_sensor_wifi_secret_local.h"
#else
/* Used WIFI SSID*/
#define ESP_WIFI_SSID "WIFI_SSID"
/* Used WIFI password*/
#define ESP_WIFI_PASS "WIFI_PASS"
/* Used WIFI Channel*/
#define ESP_WIFI_CHANNEL 0
#endif

TEST_CASE("mqtt_sensor_wifi_connect_to_sta is able to connect", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_wifi_connect_to_sta NVS is not initalized", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();
}

TEST_CASE("mqtt_sensor_wifi_connect_to_sta is able to reconnect", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_wifi_connect_to_sta wrong ssid", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = "WRONG_SSID",
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL,
    };

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_wifi_connect_to_sta wrong password", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = "WRONG_PASS",
        .channel = ESP_WIFI_CHANNEL,
    };

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}

TEST_CASE("mqtt_sensor_wifi_connect_to_sta wrong channel", "[mqtt_sensor_wifi]")
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL + 1,
    };

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_wifi_connect_to_sta(wifi_config));

    mqtt_sensor_wifi_disconnect_to_sta();

    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_deinit());
}
