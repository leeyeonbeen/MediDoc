//승연, 온도센서 값 AWS출력시도 + 시리얼 모니터에도 출력
#include<WiFi.h>
#include<DHT.h>
#include<AWS_IOT.h>
#include <LM35.h>

#define DHT_PIN 33
#define DHT_TYPE DHT11
#define RXp2 16
#define TXp2 17

#define WIFI_SSID "U5"
#define WIFI_PASSWD "4"

#define CLIENT_ID "l"
#define MQTT_TOPIC "e"
#define AWS_HOST "m"

DHT dht(DHT_PIN,DHT_TYPE);
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

Serial.print("\n initialing DHT11...");
dht.begin();
Serial.println("Done.");

Serial.println("\n initialing connection to AWS...");
if(aws.connect(AWS_HOST,CLIENT_ID)==0){
  Serial.println(" connected to AWS\n done.");
}
else{
 Serial.println("connection failed\n make system blabla"); 
}
Serial.println("done\n\ndone.\n");
}

void loop(){
  float temp=dht.readTemperature();

  if(temp==NAN){
    Serial.println("reading failed");
  }
  else{
    String temp_humidity="temp = ";
    temp_humidity +=String(temp);
    temp_humidity =(Serial2.readString());
  
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
  }
  delay(1000);
}