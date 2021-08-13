//승연, MAX30102의 BMP값을 OLED에 표시, (I2C통신)
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     4 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0; 

float beatsPerMinute;
int beatAvg;

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B00000000,
B00000000, B00000000,
B00000000, B00000000,
B00000000, B00000000,
B00000000, B00000000,
B00000000, B00000000,
B00110000, B01111000,
B01110000, B01111110,
B01111000, B11111110,
B01111100, B11101110,
B11001101, B11100110,
B11001111, B11000110,
B11000111, B11000110,
B11000111, B10000110,
B11000011, B10000110,
B11000001, B00001110,
B11100000, B00001100,
B11110000, B00001100,
B01110000, B00011100,
B00111000, B00011100,
B00111100, B00111100,
B00011110, B01111000,
B00001111, B11110000,
B00000111, B11100000,
B00000000, B00000000,
B00000000, B00000000,
B00110000, B00111000,
B01111000, B00101100,
B01001100, B01100110,
B01000100, B01000010,
B00000000, B00000000,
B00000000, B00000000
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    while (1);
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    for(;;); // Don't proceed, loop forever
  }
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0);
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:
   long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  //Serial.print("IR=");
  //Serial.print(irValue);
  //Serial.print(", BPM=");
  //Serial.print(beatsPerMinute);
  //Serial.print(", Avg BPM=");
  display.clearDisplay();

  display.drawBitmap(34,0,logo_bmp, 16, 32, 1);

  display.setTextSize(3); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 0);
  display.println(beatAvg);
  display.display();
Serial.println(beatAvg);
  //if (irValue < 50000)
    //Serial.print(" No finger?");

  //Serial.println();
}