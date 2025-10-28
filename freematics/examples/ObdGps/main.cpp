#include <Arduino.h>
#include "esp_wifi.h"
#include <FreematicsPlus.h>
#include <FreematicsBase.h>

COBD obd;
FreematicsESP32 sys;
GPS_DATA* gd = nullptr;

int gpsStatus = 0;

void setup()
{
    while(!sys.begin());
    if(sys.gpsBegin()){/* */}
    else{
        gpsStatus = 1;
    }

    obd.begin(sys.link);
}

void loop()
{
    delay(1000);
    if(sys.gpsGetData(&gd) && gd != nullptr){
        if(gd->sat >= 4) {
            Serial.printf("latitude: "); Serial.println(gd -> lat, 6);
            Serial.printf("longitude: "); Serial.println(gd -> lng, 6);
            Serial.printf("altitude: "); Serial.println(gd -> alt, 2);
            Serial.printf("velocidade: "); Serial.println(gd -> speed * 1.852, 2);
            Serial.printf(""); Serial.println(gd -> heading);
            Serial.printf(""); Serial.println(gd -> sat);
            Serial.printf(""); Serial.println(gd -> hdop);
            Serial.printf(""); Serial.println(gd -> date);
            Serial.printf(""); Serial.println(gd -> time);
            Serial.printf(""); Serial.println(gd -> ts);
        } else {
            Serial.println("Erro! Não foi possível conectar ao satélite!");
        }
    } else {
        Serial.println("Erro ao ler o GPS!");
    }
}