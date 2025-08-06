#include <Arduino.h>
unsigned long startTime;
#define led 13
#define btn 12

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(btn, INPUT_PULLUP);
  while (!Serial);  
}

void loop() {
  if (digitalRead(btn) == LOW) { 
    startTime = millis();
    Serial.println("button pressed");
    Serial.println("PING");      
    digitalWrite(led, HIGH);       
    delay(200);
    digitalWrite(led, LOW);        

    
    unsigned long timeout = millis() + 1000; 
    while (Serial.available() == 0 && millis() < timeout);

    if (Serial.available() > 0) {
      String response = Serial.readStringUntil('\n');
      response.trim();

      if (response == "PONG") {
        unsigned long rtt = millis() - startTime;
        Serial.print("RTT:");
        Serial.print(rtt);
        Serial.println(" ms");
      }
    } else {
      Serial.println("Timeout waiting for PONG");
    }
  }

  delay(200);
}


// #include <Arduino.h>
// #define btn 12

// void setup() {
//   Serial.begin(115200);
//   pinMode(btn, INPUT_PULLUP);
//   while (!Serial);  // wait for serial port to connect
// }

// void loop(){
//   if(digitalRead(btn)==LOW){
//     Serial.println("Button pressed");
//   }
// }