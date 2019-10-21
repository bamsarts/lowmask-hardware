#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "EspMQTTClient.h"

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
  
int ledPin = 13; // GPIO13

EspMQTTClient client(
  "hxp",    //ssid name
  "internet1234", //password wifi
  "broker.shiftr.io", // MQTT Broker server ip
  "0023b036",         // username mqtt
  "66a31b69fd6df2e2", // password mqtt
  "hardware-lowmask", // Client name that uniquely identify your device
  1883                // The MQTT port, default to 1883. this line can be omitted
); 

bool stateGps = false;
String locations = "";
 
void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  delay(10);
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  delay(1000);
}

void onConnectionEstablished()
{
  client.subscribe("lowmask/gps-sender", [](const String & payload) {
         stateGps = true;  
  });

   client.subscribe("lowmask/switch-led", [](const String & payload) {

      if(payload == "1"){
         digitalWrite(ledPin, HIGH);
      }else if(payload == "0"){
         digitalWrite(ledPin, LOW);
      }
   
  });
}

 
void loop() {
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
 
  if (gps.location.isValid()) {
    if(stateGps){
      client.executeDelayed(3 * 1000, []() {
        locations += "http://maps.google.com/maps?q=loc:";
        locations += gps.location.lat();
        locations += ",";
        locations += gps.location.lng();
        client.publish("lowmask/gps-receive", String(locations));
        locations = "";
      });
    }

    stateGps = false;
  }
  else {

    if(stateGps){
      client.executeDelayed(3 * 1000, []() {
        client.publish("lowmask/gps-receive", "maaf,perangkat tidak mendapat sinyal gps :(");
      });
    }

    stateGps = false;
  }

  client.loop();
  delay(1);
  displayInfo();
  Serial.println("Client disonnected");
  Serial.println("");
  
 
}

void displayInfo() {
  Serial.print(F("Location: ")); 
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
