#include <Arduino.h>
#include <WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

#define WIFI_SSID "Bundita 2.4G"
#define WIFI_PASS "0816216971"
#define MQTT_SERVER "192.168.1.241"
#define MQTT_USER "mqtt"
#define MQTT_PASSWORD "9696"
#define lockPin 3

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

IPAddress local_IP(192, 168, 1, 249);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* temperatureTopic = "home/office/temperature";
const char* humidityTopic = "home/office/humid";
bool newstart = 0;

const unsigned long interval = 6UL * 60UL * 60UL * 1000UL;
unsigned long previousMillis = 0;
unsigned long previousMillisInterval = 0;
const long intervalSend = 300000;

const int dry = 210;
const int wet = 510;

void setup_wifi();
void reconnect();
void hard_restart();
void rebootCountdown();
void checkConnection();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  Serial.println();
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  Serial.println("Connecting to WiFi");
  dht.setup(3, DHTesp::DHT11);
 
}



void setup_wifi() {
  delay(10);
  Serial.print("Connecting");
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempt++;
    if (attempt > 40) {
      // Serial.println("Failed to connect to WiFi will reset");
      hard_restart();
    }
  }
  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
}

void reconnect() {
  int mqttAttempts = 0;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Generate a random client ID
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      mqttAttempts++;
      delay(5000);
      if(mqttAttempts > 20){
        Serial.println("Failed to connect to MQTT will reset");
        hard_restart();
      }
    }
  }
}

void hard_restart() {
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while (true);
}

void rebootCountdown() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Serial.println("Rebooting...");
    hard_restart();
  }
}

void checkConnection(){
  if (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    setup_wifi();
  }

  if (!client.connected()) {
    delay(5000);
    reconnect();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String messageTemp;
  for(unsigned int i = 0; i < length; i++){
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);
  
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  unsigned long currentMillis = millis();
  if(currentMillis -previousMillis >= intervalSend){
    previousMillis = currentMillis;
    temperature = temperature - 3.5;
    String temperatureSTR = String(temperature,2);
    client.publish(temperatureTopic, temperatureSTR.c_str());
    Serial.println("Temperature");
    Serial.println(temperature);
    humidity = humidity + 2.0;
    String humiditySTR = String(humidity,2);
    client.publish(humidityTopic, humiditySTR.c_str());
    Serial.println("Humidity");
    Serial.println(humidity);
    delay(1000);
  }
 

  delay(1000);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(1000);
  newstart = 0;
  checkConnection();
  rebootCountdown();
  client.loop();
  delay(10);
  
}
