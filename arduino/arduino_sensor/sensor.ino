//연빈, only 와이파이+AWS연결 다시 정리한 코드
#include<WiFi.h>
#include<AWS_IOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWD ""

#define CLIENT_ID ""
#define MQTT_TOPIC ""
#define AWS_HOST ""

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
    String temp_humidity = Serial2.readString();
  
    String message="Welcome";
    char payload[40];
    temp_humidity.toCharArray(payload,40);

    Serial.println("pub : ");
  Serial.println(payload);
  if(aws.publish(MQTT_TOPIC,payload)==0){
    Serial.println("success\n");    
  }
  else{
    Serial.println("failed\n");
  }
  delay(1000);
}