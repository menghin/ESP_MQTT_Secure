#include <WiFi.h>
//#include <WiFiClientSecure.h>  //included WiFiClientSecure does not work!
#include "src/dependencies/WiFiClientSecure/WiFiClientSecure.h" //using older WiFiClientSecure
#include <time.h>
#include <MQTT.h>
#include "secrets_local.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include <CircularBuffer.h>

// Defines

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10       /* Time ESP32 will go to sleep (in seconds) */

#define SERIAL_LOG 1            /* Serial log is active or not */

#ifndef SECRET
  const char ssid[] = "WiFiSSID";
  const char pass[] = "WiFiPassword";

  #define LOCATION "home"
  #define HOSTNAME LOCATION "_0"

  const char *MQTT_HOST = "xxx.yyy.zzz";
  const int MQTT_PORT = 8883;
  const char *MQTT_USER = ""; // leave blank if no credentials used
  const char *MQTT_PASS = ""; // leave blank if no credentials used

  const char* local_root_ca =
    "-----BEGIN CERTIFICATE-----\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
    "-----END CERTIFICATE-----";

#endif

const char MQTT_SUB_TOPIC[] = LOCATION "/" HOSTNAME "/in";
const char MQTT_PUB_TOPIC_TEMP[] = LOCATION "/" HOSTNAME "/out/temperature";
const char MQTT_PUB_TOPIC_HUM[] = LOCATION "/" HOSTNAME "/out/humidity";
const char MQTT_PUB_TOPIC_PRES[] = LOCATION "/" HOSTNAME "/out/pressure";
const char MQTT_PUB_TOPIC_RES[] = LOCATION "/" HOSTNAME "/out/gasResistance";

// Structs

struct sensor_data 
{
  float temperature;
  float humidity;
  float pressure;
  float gasResistance;
};

// Global variables

WiFiClientSecure net;
MQTTClient client;
Adafruit_BME680 bme; // I2C
CircularBuffer<sensor_data,100> sensor_data_buffer; 

time_t now;

// Internal functions

#if (SERIAL_LOG == 1)
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup_serial(){
  Serial.begin(115200);
}

void print_serial(String msg){
  Serial.println(msg);
}
#else
#define print_wakeup_reason()
#define print_serial(msg)
#define setup_serial()
#endif

void get_BME680_readings()
{
  sensor_data sensor_data;

  print_serial("--\nget_BME680_readings()\n--");

  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    print_serial("Failed to begin reading :(");
    return;
  }
  if (!bme.endReading()) {
    print_serial("Failed to complete reading :(");
    return;
  }
  sensor_data.temperature = bme.temperature;
  sensor_data.pressure = bme.pressure / 100.0;
  sensor_data.humidity = bme.humidity;
  sensor_data.gasResistance = bme.gas_resistance / 1000.0;

  Serial.printf("- Temperature = %.2f ºC \n", sensor_data.temperature);
  Serial.printf("- Humidity = %.2f Percent \n", sensor_data.humidity);
  Serial.printf("- Pressure = %.2f hPa \n", sensor_data.pressure);
  Serial.printf("- Gas Resistance = %.2f KOhm \n", sensor_data.gasResistance);  

  sensor_data_buffer.push(sensor_data);
}

void mqtt_connect()
{
  print_serial("- MQTT connecting");
  while (!client.connect(HOSTNAME, MQTT_USER, MQTT_PASS))
  {
    print_serial("- .");
    delay(1000);
  }
  print_serial("- MQTT connected");
  client.subscribe(MQTT_SUB_TOPIC);
}

void messageReceived(String &topic, String &payload)
{
  print_serial("- Received [" + topic + "]: " + payload);
}

void setup()
{
  setCpuFrequencyMhz(80);

  setup_serial();

  print_serial("First initialisation");  

  print_serial("Attempting to connect to SSID: " + String(ssid));
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }
  print_serial("Connected to " + String(ssid));

  print_serial("Setting time using SNTP ");
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < 1510592825) {
    delay(500);
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  print_serial("Current time: " + String(ctime(&now)));

  client.begin(MQTT_HOST, MQTT_PORT, net);
  client.onMessage(messageReceived);
  mqtt_connect();

  // Init BME680 sensor
  if (!bme.begin()) {
    print_serial(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms  
}

void send_sensor_data()
{
  uint8_t retryCounter = 10;
  uint8_t number_of_sensor_data = sensor_data_buffer.size();

  print_serial("--\nsend_sensor_data(sensor_data_buffer.size=" + String(number_of_sensor_data) + ")\n--");

  if (WiFi.status() != WL_CONNECTED)
  {
    print_serial("- Checking Wifi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      delay(10);
      retryCounter--;
      if (retryCounter==0)
      {
        throw "Connection to Wifi failed";
      }
    }
    print_serial("- Wifi connected");
  }

  // Check mqtt connection otherwise try to connect
  if (!client.connected())
  {
    mqtt_connect();

    if (!client.connected())
    {
      throw "Connection to MQTT failed";
    }
  }
  else
  {
    client.loop();
  }

  // Send the data
  for (byte i = 0; i < number_of_sensor_data; i++) {
    sensor_data sensor_data = sensor_data_buffer.pop();
    client.publish(MQTT_PUB_TOPIC_TEMP, String(sensor_data.temperature).c_str(), false, 0);
    client.publish(MQTT_PUB_TOPIC_HUM, String(sensor_data.humidity).c_str(), false, 0);
    client.publish(MQTT_PUB_TOPIC_PRES, String(sensor_data.pressure).c_str(), false, 0);
    client.publish(MQTT_PUB_TOPIC_RES, String(sensor_data.gasResistance).c_str(), false, 0);
  }
}

void loop()
{
  // Print the wakeup reason for ESP32 
  print_wakeup_reason(); 

  // Get the current time
  now = time(nullptr); 

  try
  {
    // Get the sensor data
    get_BME680_readings();

    // Send the data (all on the buffer)
    send_sensor_data();
  }
  catch(const std::exception& e)
  {
    print_serial("[E] - " + String(e.what()));
  }

  // Go back to sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // ESP32 wakes up every 60 seconds
  print_serial("Going to light-sleep now");
  Serial.flush(); 
  esp_light_sleep_start();  
}
