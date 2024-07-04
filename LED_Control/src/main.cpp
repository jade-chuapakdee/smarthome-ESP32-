#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#define WIFI_SSID "Bundita 2.4G"
#define WIFI_PASS "0816216971"
#define MQTT_SERVER "192.168.1.241"
#define MQTT_USER "mqtt"
#define MQTT_PASSWORD "9696"
#define wifiPin 23
#define ledPin1 4
#define ledPin2 5

WiFiClient espClient;
PubSubClient client(espClient);

const char* led1CommandTopic = "home/server/led1/set";
const char* led1StateTopic = "home/server/led1";
const char* led1AvailabilityTopic = "home/server/led1/available";

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  pinMode(wifiPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin1, LOW);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  // Serial.println("Connecting to WiFi");
}

void setup_wifi(){
  delay(10);
  // Serial.print("Connecting");
  
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    // Serial.print(".");
  }
  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
  digitalWrite(wifiPin, HIGH);

}

void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    if(client.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD)){
      // Serial.println("connected");
      client.subscribe(led1CommandTopic);
      client.publish(led1AvailabilityTopic, "online");
    } else {
      // Serial.print("failed, rc=");
      // Serial.print(client.state());
      // Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");

  String messageTemp;
  for(unsigned int i = 0; i < length; i++){
    messageTemp += (char)payload[i];
  }
  // Serial.println(messageTemp);

  if(String(topic) == led1CommandTopic){
    if(messageTemp == "ON"){
      digitalWrite(ledPin1, HIGH);
      client.publish(led1StateTopic, "ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(ledPin1,HIGH);
      client.publish(led1StateTopic,"ON");
      delay(3000);
      digitalWrite(ledPin1, LOW);
      client.publish(led1StateTopic, "OFF");
    }
  }
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  delay(500);
}