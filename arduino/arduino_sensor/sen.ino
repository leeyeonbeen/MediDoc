#include "ssd1306h.h"
#include "MAX30102.h"
#include "Pulse.h"
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <Adafruit_MLX90614.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64

#define OLED_RESET LED_BUILTIN
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define ECG 135

SSD1306 oled; 
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;
MAFilter bpm;

#define LED LED_BUILTIN
#define BUTTON 3
#define OPTIONS 7
  
static const uint8_t heart_bits[] PROGMEM = { 0x00, 0x00, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0xfe, 0xfe, 0xff, 
                                        0xfe, 0xff, 0xfc, 0x7f, 0xf8, 0x3f, 0xf0, 0x1f, 0xe0, 0x0f,
                                        0xc0, 0x07, 0x80, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
                                        0x00, 0x00 };

//spo2_table is approximated as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t spo2_table[184] PROGMEM =
        { 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99, 
          99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
          100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 
          97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 
          90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 
          80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 
          66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
          49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29, 
          28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 
          3, 2, 1 } ;

int getVCC() {
  //reads internal 1V1 reference against VCC
  #if defined(__AVR_ATmega1284P__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
  #endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low = ADCL;
  unsigned int val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  low = ADCL;
  val = (ADCH << 8) | low;
  
  return (((long)1024 * 1100) / val)/100;  
}

void print_digit(int x, int y, long val, char c=' ', uint8_t field = 3,const int BIG = 2) {  
    uint8_t ff = field;
    do { 
        char ch = (val!=0) ? val%10+'0': c;
        oled.drawChar( x+BIG*(ff-1)*6, y, ch, BIG);
        val = val/10; 
        --ff;
    }
    while (ff>0);
}
/*
 *   Record, scale  and display PPG Wavefoem
 */

const uint8_t MAXWAVE = 72;

class Waveform {
  public:
    Waveform(void) {
      wavep = 0;
    }
    void record(int waveval) {
      waveval = waveval/8;         // scale to fit in byte  ?????????????????????
      waveval += 128;              //shift so entired waveform is +ve  
      waveval = waveval<0? 0 : waveval;
      waveform[wavep] = (uint8_t) (waveval>255)?255:waveval; 
      wavep = (wavep+1) % MAXWAVE;
    }
  
    void scale() {
      uint8_t maxw = 0;
      uint8_t minw = 255;
      for (int i=0; i<MAXWAVE; i++) { 
        maxw = waveform[i]>maxw?waveform[i]:maxw;
        minw = waveform[i]<minw?waveform[i]:minw;
      }
      uint8_t scale8 = (maxw-minw)/4 + 1;  //scale * 8 to preserve precision
      uint8_t index = wavep;
      for (int i=0; i<MAXWAVE; i++) {
        disp_wave[i] = 31-((uint16_t)(waveform[index]-minw)*8)/scale8;
        index = (index + 1) % MAXWAVE;
      }
    }
    void draw(uint8_t X) {
      for (int i=0; i<MAXWAVE; i++) {
        uint8_t y = disp_wave[i];
        oled.drawPixel(X+i, y);
        if (i<MAXWAVE-1) {
          uint8_t nexty = disp_wave[i+1];
          if (nexty>y) {
            for (uint8_t iy = y+1; iy<nexty; ++iy)  
              oled.drawPixel(X+i, iy);
          } 
          else if (nexty<y) {
            for (uint8_t iy = nexty+1; iy<y; ++iy)  
            oled.drawPixel(X+i, iy);
         }
       }
     } 
   }
 private:
   uint8_t waveform[MAXWAVE];
   uint8_t disp_wave[MAXWAVE];
   uint8_t wavep = 0;
} wave;

int  beatAvg;
int  SPO2, SPO2f, SPO2m;
int  voltage;
bool filter_for_graph = false;
bool draw_Red = false;
uint8_t pcflag =0;
uint8_t istate = 0;
uint8_t sleep_counter = 0;

extern volatile unsigned long timer0_millis;

int cnt=0;
int cnt1=0;
int AVG_bpm=0;
float AVG_temp=0;
int AVG_O=0;
int i=0;
int sum_bpm;
int sum_temp;
int sum_O;
int E[ECG]={0};

long lastBeat = 0;    //Time of the last beat 
long displaytime = 0; //Time of the last display update
bool led_on = false;

void button(void){
    pcflag = 1;
}

void checkbutton(){
  if (pcflag && !digitalRead(BUTTON)) {
    istate = (istate +1) % 4;
    filter_for_graph = istate & 0x01;
    draw_Red = istate & 0x02;
    EEPROM.write(OPTIONS, filter_for_graph);
    EEPROM.write(OPTIONS+1, draw_Red);
  }
  pcflag = 0;
}

void restart(){
   if(pcflag && !digitalRead(BUTTON)){
     setup();
   }
   pcflag = 0;
}

void go_sleep() {
  i=0;
  cnt1=0; cnt=0;
  sum_bpm=0; AVG_bpm=0;
  AVG_temp=0; sum_temp=0;
  AVG_O=0; sum_O=0; 
  oled.fill(0);
  oled.off();
  delay(10);
  sensor.off();
  delay(10);
  cbi(ADCSRA, ADEN);  // disable adc
  delay(10);
  pinMode(0,INPUT);
  pinMode(2,INPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);     
  sleep_mode();  // sleep until button press 
  // cause reset
  setup();
}

void draw_oled(int msg) {
  oled.firstPage();
  do{
    switch(msg){
      case 0:  oled.drawStr(10,14,F("Device Error"),1); 
               break;
      case 1:  oled.drawStr(0,0,F("PLACE YOUR"),2); 
               oled.drawStr(25,18,F("FINGER"),2);      
               break;
      case 2:  oled.drawStr(0,0,F("Pulse Rate"),1);
               print_digit(90,0,beatAvg,' ',3,1);
               oled.drawStr(0,12,F("Oxygen"),1);
               print_digit(90,12,SPO2f,' ',3,1);
               oled.drawChar(116,12,'%',1);
               oled.drawStr(0,24,F("Temp"),1);
               print_digit(90,24,mlx.readObjectTempC(),' ',3,1);               
               break;
      case 3:  oled.drawStr(5,0,F("Biometrics"),2);
               oled.drawStr(0,15,F("Measurement"),2);
               //oled.drawXBMP(6,8,16,16,heart_bits);
               break;
      case 4:  oled.drawStr(28,12,F("OFF IN"),1);
               oled.drawChar(76,12,10-sleep_counter/10+'0');
               oled.drawChar(82,12,'s');
               break;
      case 5:  oled.drawStr(0,0,F("Measurement Complete"),1);
               oled.drawStr(0,9,F("Avg Pulse"),1); 
               print_digit(90,9,AVG_bpm,' ',3,1);
               oled.drawStr(0,17,F("Avg Oxygen"),1); 
               print_digit(90,17,SPO2f,' ',3,1);
               oled.drawStr(0,25,F("Temp"),1); 
               print_digit(90,25,AVG_temp,' ',3,1);               
               break;
       case 6: oled.drawStr(24,0,F("Good"),2); 
               oled.drawStr(25,18,F("bye"),2);
               break;      
       case 7: oled.drawStr(25,0,F("ATTACH"),2); 
               oled.drawStr(45,18,F("ECG"),2); 
               break;
    }
  }  
  while (oled.nextPage());
}

void setup(void) { 
  pinMode(BUTTON, INPUT_PULLUP);//????????? ??????
  pinMode(10, INPUT); // ECG
  pinMode(11, INPUT); //ECG
  filter_for_graph = EEPROM.read(OPTIONS);
  draw_Red = EEPROM.read(OPTIONS+1);
  oled.init();  //oled????????? 
  oled.fill(0x00);
  draw_oled(3);
  delay(3000); 
  draw_oled(7);
  delay(2000); 
  if (!sensor.begin())  {
    draw_oled(0);
    while (1);
  }
  sensor.setup(); 
  attachInterrupt(digitalPinToInterrupt(BUTTON),button, CHANGE);
  Serial.begin(9600);
  mlx.begin(); 
}

void loop()  
{
  sensor.check();
  long now = millis();   //start time of this cycle
  if (!sensor.available()) return;
  uint32_t irValue = sensor.getIR(); 
  uint32_t redValue = sensor.getRed();
  sensor.nextSample();
  if (irValue<5000) {
    voltage = getVCC();
    checkbutton();
    draw_oled(sleep_counter<=50 ? 1 : 4); // finger not down message    
    delay(200);
    ++sleep_counter;
    if (sleep_counter>100) {
      go_sleep();
      sleep_counter = 0;
    }
    restart();
  }
  else { 
    sleep_counter = 0;
    int16_t IR_signal, Red_signal;
    bool beatRed, beatIR;
    if (!filter_for_graph) {
      IR_signal = pulseIR.dc_filter(irValue) ;
      Red_signal = pulseRed.dc_filter(redValue);
      beatRed = pulseRed.isBeat(pulseRed.ma_filter(Red_signal));
      beatIR =  pulseIR.isBeat(pulseIR.ma_filter(IR_signal));        
    } 
    else {
      IR_signal = pulseIR.ma_filter(pulseIR.dc_filter(irValue)) ;
      Red_signal = pulseRed.ma_filter(pulseRed.dc_filter(redValue));
      beatRed = pulseRed.isBeat(Red_signal);
      beatIR =  pulseIR.isBeat(IR_signal);
    }
    // invert waveform to get classical BP waveshape
    wave.record(draw_Red ? -Red_signal : -IR_signal ); 
    // check IR or Red for heartbeat     
    if (draw_Red ? beatRed : beatIR){
      long btpm = 60000/(now - lastBeat);
      if (btpm > 0 && +btpm < 200) beatAvg = bpm.filter((int16_t)btpm);
      lastBeat = now; 
      digitalWrite(LED, HIGH); 
      led_on = true;
      // compute SpO2 ratio
      long numerator   = (pulseRed.avgAC() * pulseIR.avgDC())/256;
      long denominator = (pulseRed.avgDC() * pulseIR.avgAC())/256;
      int RX100 = (denominator>0) ? (numerator * 100)/denominator : 999;
      // using formula
      SPO2f = (10400 - RX100*17+50)/100; 
      // from table
      if ((RX100>=0) && (RX100<184))
        SPO2 = pgm_read_byte_near(&spo2_table[RX100]);   
      if(now-displaytime>50){      
        unsigned long time1 = millis() / 1000;
        displaytime = now;
        wave.scale();
        draw_oled(2);
        cnt+=1;
        delay(10); 
        
        if(cnt>=30&&cnt<130){
          sum_bpm += beatAvg; 
          sum_temp += mlx.readObjectTempC();
                      
          AVG_bpm = sum_bpm/cnt;        
          AVG_temp = sum_temp/cnt;
          E[cnt]=analogRead(A0);
        }   
      
        if(cnt==130){
          Serial.println("");
          Serial.print("bpm : ");Serial.print(AVG_bpm);
          Serial.print(", spo2 : "); Serial.print(SPO2f);
          Serial.print(", temperature : ");Serial.println(AVG_temp);     
          delay(1500);

          Serial.print("ECG1 : ");
          for(i=30;i<=cnt-1;i++){
            if(i<80) {
              Serial.print(E[i]);
              Serial.print(", ");
            }
            else if(i==80){
              Serial.println("");
              delay(1500);
              Serial.print("ECG2 : ");
              Serial.print(E[i]);
            }
            else {
              Serial.print(", ");
              Serial.print(E[i]);
            }
            E[i]=0;
          } 
          draw_oled(5);
          delay(3000);
          draw_oled(6);
          delay(2000);
          pcflag = 1;    
          go_sleep(); 
        }
      }
    }
  }
}