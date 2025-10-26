#include <FreematicsPlus.h>
#include <Arduino.h>
#include "esp_wifi.h"

COBD obd;
FreematicsESP32 sys;
bool connected = false;
unsigned long count = 0;

typedef struct {
    byte pid;
    String telemetry;
} PIDTELEMETRY;

PIDTELEMETRY pidTelemetry[] = {
  { (byte)PID_THROTTLE, "throttle"},
  { (byte)PID_BATTERY_VOLTAGE, "battery"},
  {PID_SPEED, "speed"},
  { (byte)PID_RPM, "rpm"},
  { (byte)PID_MAF_FLOW, "maf"},
  { (byte)PID_INTAKE_MAP, "map"},
  { (byte)PID_FUEL_LEVEL, "fuel_level"},
  { (byte)PID_AMBIENT_TEMP, "ambient_temp"},
  { (byte)PID_BATTERY_VOLTAGE, "battery_voltage"},
  { (byte)PID_ENGINE_LOAD, "engine_load"},
  { (byte)PID_COOLANT_TEMP,  "coolant_temp"},
  { (byte)PID_ODOMETER, "odometer"},
  { (byte)PID_ENGINE_OIL_TEMP, "engine_oil_temp"},
  { (byte)PID_ENGINE_FUEL_RATE, "engine_fuel_rate"},
  { (byte)PID_ENGINE_LOAD, "engine_load"},
  { (byte)PID_ENGINE_REF_TORQUE, "engine_ref_torque"},
  { (byte)PID_ENGINE_TORQUE_DEMANDED, "engine_torque_demanded"},
  { (byte)PID_ENGINE_TORQUE_PERCENTAGE, "engine_torque_percentage"},
  { (byte)PID_ABSOLUTE_ENGINE_LOAD, "absolute_engine_load"},
  { (byte)PID_FUEL_INJECTION_TIMING, "fuel_injection_timing"},
  { (byte)PID_DEVICE_HALL, "device_hall"}
}

void ObdReconnect()
{
    while(!sys.begin());
    obd.begin(sys.link);
}
void setup()
{
    Serial.begin(115200);
    delay(1000);

    while (!sys.begin());
    obd.begin(sys.link);
    delay(1000);
}

void loop()
{
    delay(1000);
    ObdReconnect();

    if(!connected){
        if(obd.init()){
            connected = true;
        }
        return;
    }

    if(obd.errors > 2) {
        connected = false;
        obd.reset();
    }

    for(int i = 0; i < sizeof(pidTelemetry)/sizeof(pidTelemetry[0]);i++){
        int value;
        obd.readPID(pidTelemtry[i].pid, value);
        String telemetry = pidTelemetry[i].telemetry;
        Serial.println(telemetry);
        Serial.println(value);
        Serial.println("\n");
    }
}