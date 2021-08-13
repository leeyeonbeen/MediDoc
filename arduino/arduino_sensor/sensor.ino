//연빈,DHT센서 관련 코드를 뺀 only esp32-aws
#include<WiFi.h>
#include<AWS_IOT.h>

#define WIFI_SSID "U5"
#define WIFI_PASSWD "4"

#define CLIENT_ID "ly"
#define MQTT_TOPIC "ly"
#define AWS_HOST "am"

AWS_IOT aws;

void setup(){
  Serial.begin(9600);
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

Serial.print("\n initialing DHT11...");
Serial.println("Done.");

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
    String temp_humidity="temp = ";
    temp_humidity += "success ";
  
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