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
#define ledPin1 6
#define switchPin1 2
#define ledPin2 7
#define switchPin2 3
#define ledPin3 8
#define switchPin3 1 


WiFiClient espClient;
PubSubClient client(espClient);

IPAddress local_IP(192, 168, 1, 245);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ledCommandTopic1 = "home/meeting/led1/set";
const char* ledStateTopic1 = "home/meeting/led1";
const char* ledAvailabilityTopic1 = "home/meeting/led1/available";

const char* ledCommandTopic2 = "home/meeting/led2/set";
const char* ledStateTopic2 = "home/meeting/led2";
const char* ledAvailabilityTopic2 = "home/meeting/led2/available";

const char* ledCommandTopic3 = "home/meeting/led3/set";
const char* ledStateTopic3 = "home/meeting/led3";
const char* ledAvailabilityTopic3 = "home/meeting/led3/available";

bool newstart = 0;
bool lastSwitchState1 = HIGH;
bool lastSwitchState2 = HIGH;
bool lastSwitchState3 = HIGH;

const unsigned long interval = 24UL * 60UL * 60UL * 1000UL;
unsigned long previousMillis = 0;
const long intervalSend = 1000000;

void setup_wifi();
void reconnect();
void hard_restart();
void rebootCountdown();
void checkConnection();
void checkSwitches();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  newstart = 1;
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(switchPin1, INPUT);
  pinMode(switchPin2, INPUT);
  pinMode(switchPin3, INPUT);
  // digitalWrite(ledPin1, LOW);
  // digitalWrite(ledPin2, LOW);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void setup_wifi(){
  delay(10);
  Serial.print("Connecting");
  if(!WiFi.config(local_IP, gateway, subnet)){
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempt = 0;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    attempt++;
    if(attempt > 30){
      Serial.println("Failed to connect to WiFi will reset");
      hard_restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
  int mqttAttempts = 0;
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if(client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)){
      Serial.println("connected");
      client.subscribe(ledCommandTopic1);
      client.subscribe(ledCommandTopic2);
      client.subscribe(ledCommandTopic3);
      client.publish(ledAvailabilityTopic1, "online");
      client.publish(ledAvailabilityTopic2, "online");
      client.publish(ledAvailabilityTopic3, "online");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      mqttAttempts++;
      if(mqttAttempts > 20){
        Serial.println("MQTT connection failed. Restarting...");
        hard_restart();
      }
    }
  }
}

void hard_restart(){
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while(true);
}

void rebootCountdown(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    Serial.println("Rebooting...");
    hard_restart();
  }
}

void checkConnection(){
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi disconnected! Reconnecting...");
    setup_wifi();
  }
  if(!client.connected()){
    reconnect();
  }
}

void checkSwitches(){
  bool currentSwitchState1 = digitalRead(switchPin1);
  if(lastSwitchState1 != currentSwitchState1){
    delay(500);
    if(currentSwitchState1 == LOW){  //led is on
      client.publish(ledStateTopic1,"ON");
      // Serial.println("CheckSW: LED1 ON");
    } else if(currentSwitchState1 == HIGH){ //led is off
      client.publish(ledStateTopic1, "OFF");
      // Serial.println("CheckSW: LED1 OFF");
     
    }
    lastSwitchState1 = currentSwitchState1;
  }

  bool currentSwitchState2 = digitalRead(switchPin2);
  if(lastSwitchState2 != currentSwitchState2){
    delay(500);
    if(currentSwitchState2 == LOW){  //led is on
      client.publish(ledStateTopic2,"ON");
      // Serial.println("CheckSW: LED2 ON");
    } else if(currentSwitchState2 == HIGH){ //led is off
      client.publish(ledStateTopic2, "OFF");
      // Serial.println("CheckSW: LED2 OFF");
     
    }
    lastSwitchState2 = currentSwitchState2;
  }

  bool currentSwitchState3 = digitalRead(switchPin3);
  if(lastSwitchState3 != currentSwitchState3){
    delay(500);
    if(currentSwitchState3 == LOW){  //led is on
      client.publish(ledStateTopic3,"ON");
      // Serial.println("CheckSW: LED1 ON");
    } else if(currentSwitchState3 == HIGH){ //led is off
      client.publish(ledStateTopic3, "OFF");
      // Serial.println("CheckSW: LED1 OFF");
     
    }
    lastSwitchState3 = currentSwitchState3;
  }
  
}



void callback(char* topic, byte* payload, unsigned int length){
  String messageTemp;
  for(unsigned int i = 0; i < length; i++){
    messageTemp += (char)payload[i];
  }

  if(String(topic) == ledCommandTopic1){
    if(messageTemp == "ON"){
      digitalWrite(ledPin1, LOW);
      client.publish(ledStateTopic1, "ON");
      Serial.println("Received message:LED1 ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(ledPin1, HIGH);
      client.publish(ledStateTopic1, "OFF");
      Serial.println("Received message:LED1 OFF");
    }
  }

  if(String(topic) == ledCommandTopic2){
    if(messageTemp == "ON"){
      digitalWrite(ledPin2,LOW);
      client.publish(ledStateTopic2, "ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(ledPin2, HIGH);
      client.publish(ledStateTopic2, "OFF");
    }
  }

   if(String(topic) == ledCommandTopic3){
    if(messageTemp == "ON"){
      digitalWrite(ledPin3,LOW);
      client.publish(ledStateTopic3, "ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(ledPin3, HIGH);
      client.publish(ledStateTopic3, "OFF");
    }
  }
  
}

void loop() {

  newstart = 0;
  checkConnection();
  checkSwitches();
  rebootCountdown();
  client.loop();
  delay(100);
}
