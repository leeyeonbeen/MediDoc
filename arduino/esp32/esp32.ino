#include<WiFi.h>
#include "ArduinoJson.h"
#include<AWS_IOT.h>

#define WIFI_SSID "{wifi_ssid}"
#define WIFI_PASSWD "{wifi_passwd}"

#define CLIENT_ID "{client_id}"
#define MQTT_TOPIC "{mqtt_id}"
#define AWS_HOST "{aws_host}"

#define RXp2 16
#define TXp2 17

AWS_IOT aws;

void setup(){
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  Serial.print("\ninitializing thing temp_humidity_\n");

  Serial.print("\n initialing wifi: connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.print(" ");
  while(WiFi.status()!=WL_CONNECTED){
   Serial.print(".");
  delay(500);
  } 
Serial.println("\n connected.\n done");

Serial.println("\n initialing connection to AWS...");
if(aws.connect(AWS_HOST,CLIENT_ID)==0){
  Serial.println(" connected to AWS\n done.");
}
else{
 Serial.println("connection failed\n make syre blabla"); 
}
Serial.println("done\n\ndone.\n");
}

void loop(){
    String temp_humidity= "";
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["format"] = "string";
    root["topic"] = MQTT_TOPIC;
    root["timestamp"] = "1627363020648";
    root["payload"] = Serial2.readString();
    root.printTo(temp_humidity);
    
    char payload[200];
    temp_humidity.toCharArray(payload,200);

    Serial.println("pub : ");
  Serial.println(payload);
  if(aws.publish(MQTT_TOPIC,payload)==0){
    Serial.println("success\n");    
  }
  else{
    Serial.println("failed\n");
  }
}