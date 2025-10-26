#include <Arduino.h>
#include <FreematicsPlus.h>
#include "esp_wifi.h"

FreematicsESP32 sys;
COBD obd;

#define STATE_MEMS_READY 0x8
float accBias[3] = {0};
float accSum[3] = {0};
float acc[3] = {0};
float gyr[3] = {0};
float mag[3] = {0};
uint8_t accCount = 0;

class GYROSCOPE {
    public:
        float gyr_x = 0;
        float gyr_y = 0;
        float gyr_z = 0;

        float acc_x = 0;
        float acc_y = 0;
        float acc_z = 0;

        float mag_x = 0;
        float mag_y = 0;
        float mag_z = 0;        
}

MEMS_I2C* mems = 0;
class STATE {
    public:
        bool check(uint16_t flags) {return (state & flags) == flags;}
        void set(uint16_t flags) {state |= flags;}
        void clear(uint16_t flags) { state &= ~flags;}
        uint16_t state = 0;
};

STATE state;

void ObdReconnect()
{
    while(!sys.begin());
    obd.begin(sys.link);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    if(!state.check(STATE_MEMS_READY)){
        mems = new ICM_42627;
        byte ret = mems->begin();
        if(ret){
            state.set(STATE_MEMS_READY);
        }else{
            delete mems;
            mems = 0;
        }
    }

    delay(1000);
}

void loop()
{
    delay(1000);

    if(state.check(STATE_MEMS_READY)){
        if(mems->read(acc,gyr,mag)){
            Serial.println("acc x: "); Serial.println(acc[0]);
            Serial.println(" acc y: "); Serial.println(acc[1]);
            Serial.println(" acc z: "); Serial.println(acc[2]);
            Serial.println("\n");
            Serial.println("gyr: "); Serial.println(gyr[0]);
            Serial.println(" gyr: "); Serial.println(gyr[1]);
            Serial.println(" gyr: "); Serial.println(gyr[2]);
            Serial.println("\n");
            Serial.println("mag x: \r"); Serial.println(mag[0]);
            Serial.println(" mag y: "); Serial.println(mag[1]);
            Serial.println(" mag z: "); Serial.println(mag[2]);
        }
    }
}