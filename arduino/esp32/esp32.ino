#include <NTPClient.h>
#include <WiFiUdp.h>
#include<WiFi.h>
#include <ArduinoJson.h>
#include<AWS_IOT.h>

#define WIFI_SSID "SK_WiFiGIGAEC63_2.4G"
#define WIFI_PASSWD "2005003420"

#define CLIENT_ID "coco"
#define MQTT_TOPIC "$aws/things/coco/shadow/name/crunch"
#define AWS_HOST "a2utwh13lgm1sf-ats.iot.ap-northeast-2.amazonaws.com"

#define RXp2 16
#define TXp2 17

AWS_IOT aws;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "kr.pool.ntp.org", 32400,10);

void setup(){
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

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
    Serial.println("connection failed\n"); 
  }
  Serial.println("\ndone.\n");
  timeClient.begin();
}

void loop(){
  timeClient.update();
  String serial2 = Serial2.readString();
  String epochms;
  String epochs = String(timeClient.getEpochTime());
  long epochMS = timeClient.mssec;
  epochMS/100>0 ? epochms = String(epochMS) : 
  epochMS/10>0 ? epochms = "0" + String(epochMS) : 
  epochms = "00" + String(epochMS);
  String epoch = epochs + epochms;
  if(serial2 != ""){
    String jsondata= "";
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["timestamp"] = epoch;
    root["payload"] = serial2;
    root.printTo(jsondata);
    
    char payload[500];
    jsondata.toCharArray(payload,500);

    Serial.println("pub : ");
    Serial.println(payload);

    if(aws.publish(MQTT_TOPIC,payload)==0){
      Serial.println("success\n");    
    }
    else{
      Serial.println("failed\n"); 
      
    }
  }
}