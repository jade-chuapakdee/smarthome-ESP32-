#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

#define WIFI_SSID "Bundita 2.4G"
#define WIFI_PASS "0816216971"
#define MQTT_SERVER "192.168.1.241"
#define MQTT_USER "mqtt"
#define MQTT_PASSWORD "9696"
#define ledPin 3
#define lockPin 2
#define switchPin 6

WiFiClient espClient;
PubSubClient client(espClient);

IPAddress local_IP(192, 168, 1, 242);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ledCommandTopic = "home/lobby/ledUp/set";
const char* ledStateTopic = "home/lobby/ledUp";
const char* ledAvailabilityTopic = "home/lobby/ledUp/available";

const char* lockCommandTopic = "home/stairs/doorlock1/set";
const char* lockStateTopic = "home/stairs/doorlock1";
bool newstart = 0;
bool lastSwitchState = HIGH;

const unsigned long interval = 6UL * 60UL * 60UL * 1000UL;
unsigned long previousMillis = 0;

void setup_wifi();
void reconnect();
void hard_restart();
void rebootCountdown();
void checkConnection();
// void checkSwithces();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  newstart = 1;
  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
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
    // Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Generate a random client ID
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      // Serial.println("connected");
      client.subscribe(lockCommandTopic);
    } else {
      // Serial.print("failed, rc=");
      // Serial.print(client.state());
      // Serial.println(" try again in 5 seconds");
      mqttAttempts++;
      delay(5000);
      if(mqttAttempts > 20){
        // Serial.println("Failed to connect to MQTT will reset");
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

// void checkSwithces(){
//   bool currentSwitchState = digitalRead(switchPin);
//   if(currentSwitchState != lastSwitchState){
//     delay(500);
//     if(currentSwitchState == LOW){
//       client.publish(ledCommandTopic, "ON");
//     } else if (currentSwitchState == HIGH){
//       client.publish(ledCommandTopic, "OFF");
//     }
//     lastSwitchState = currentSwitchState;
//   }
// }


void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  if (String(topic) == lockCommandTopic) {
    if (messageTemp == "ON") {
      digitalWrite(lockPin, HIGH);
      client.publish(lockStateTopic, "ON");
    } else if (messageTemp == "OFF") {
      digitalWrite(lockPin, HIGH);
      client.publish(lockStateTopic, "ON");
      delay(3000);
      digitalWrite(lockPin, LOW);
      client.publish(lockStateTopic, "OFF");
    }
  }

  if(String(topic) == ledCommandTopic){
    if(messageTemp == "ON"){
      digitalWrite(ledPin, LOW);
      client.publish(ledStateTopic, "ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(ledPin, HIGH);
      client.publish(ledStateTopic, "OFF");
    }
  }
}

void loop() {
  newstart = 0;
  checkConnection();
  // checkSwithces();
  rebootCountdown();
  client.loop();
  delay(10);
  
}
