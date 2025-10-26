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
            Serial.println("latitude: "); Serial.println(gd -> lat, 6);
            Serial.println("longitude: "); Serial.println(gd -> lng, 6);
            Serial.println("altitude: "); Serial.println(gd -> alt, 2);
            Serial.println("velocidade: "); Serial.println(gd -> speed * 1.852, 2);
            Serial.println(""); Serial.println(gd -> heading);
            Serial.println(""); Serial.println(gd -> sat);
            Serial.println(""); Serial.println(gd -> hdop);
            Serial.println(""); Serial.println(gd -> date);
            Serial.println(""); Serial.println(gd -> time);
            Serial.println(""); Serial.prinln(gd -> ts);
        } else {
            Serial.println("Erro!\nNão foi possível conectar ao satélite!")
        }
    } else {
        Serial.println("Erro ao ")
    }
}