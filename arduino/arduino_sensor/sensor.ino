//승연, AWS값 송출을 위한 MAX30102 새로운 코드 시도
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

PulseOximeter pox;    //  맥박, 산소포화도 관련 객체 생성

uint32_t tsLastReport = 0;

// 콜백함수, 맥박이 감지되면 실행될 함수
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);

    Serial.print("Initializing pulse oximeter..");

    // 맥박, 산소포화도 관련 객체 초기화

    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

   
    // 콜백함수 등록
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // 센서값을 계속 최신화
    pox.update();

    // 1초에 한번씩 시리얼 모니터에 값을 출력
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");

        tsLastReport = millis();
    }
}