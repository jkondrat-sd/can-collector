#include <FreematicsPlus.h>
#include <FreematicsBase.h>
#include <FreematicsMEMS.h>
#include <FreematicsOBD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
/*
#include <Arduino_HTTP_Client.h>
#include <ThingsBoardHttp.h>
#include <ThingsBoard.h>
*/
#include <ArduinoJson.h>

#include "config.h"
// libs SD card
// #include "datalogger.h"
#include "telestore.h"
#include <SD.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>

#include <HttpClient.h>

#include "CustomHTTPClient_Oficial.h"

// states
#define STATE_STORE_READY 0x1
#define STATE_MEMS_READY 0x20
#define STATE_FILE_READY 0x40

// alternative https
HTTPClient_Custom http;


FreematicsESP32 freematics;
COBD obd;
GPS_DATA* gd;

const char* WIFI_SSID = "ssidWifi";
const char* WIFI_PASSWORD = "123123";
const char* TOKEN = "tokentoken";
const char* THINGSBOARD_SERVER = "mobilidade.inmetro.gov.br";

// macros
#ifdef ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 443U;
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";
WiFiClientSecure wifiClient;
#else
    constexpr uint16_t THINGSBOARD_PORT = 8080U;
    WiFiClient wifiClient;
#endif

// Arduino_HTTP_Client httpClient(wifiClient, THINGSBOARD_SERVER, THINGSBOARD_PORT);
// ThingsBoardHttp tb(httpClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT, true, 2048);
// ThingsBoardHttp tb(httpClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);

struct VehicleData {
    int speed = 0;
    int rpm = 0;
    float temperature = 0;
    float voltage = 0;
    uint32_t timestamp = 0;
};

struct GpsData {
  float lat = 0;
  float lng = 0;
  float altitude = 0;
  float speed = 0;
  uint8_t satellites = 0;
  uint32_t timestamp = 0;  
};

struct DataMEMS {
    float accel[3] = {0,0,0};
    float gyro[3] = {0,0,0};
    float mag[3] = {0,0,0};
    ORIENTATION orientation = {0,0,0};
    float temperature = 0;
    uint32_t timestamp = 0;
};

struct DataServer {
    // mems
    float mems_accel[3] = {0,0,0};
    float mems_gyro[3] = {0,0,0};
    float mems_mag[3] = {0,0,0};
    ORIENTATION mems_orientation = {0,0,0};
    float mems_temperature = 0;
    uint32_t mems_timestamp = 0;

    // gps
    float gps_lat = 0;
    float gps_lng = 0;
    float gps_altitude = 0;
    float gps_speed = 0;
    uint8_t gps_satellites = 0;
    uint32_t gps_timestamp = 0;

    // obd
    int speed = 0;
    int rpm = 0;
    float temperature = 0;
    float voltage = 0;
    uint32_t timestamp = 0;
};

VehicleData vehicleData;
GpsData gpsData;
DataMEMS dataMEMS;
DataServer dataServer;

Mutex mutexVehicle;
Mutex mutexGPS;
Mutex mutexMEMS;
Mutex mutexSD;

volatile bool initObd = false;
volatile bool initMems = false;
volatile bool initGps = false;
volatile bool connectedWifi = false;
volatile bool initSD = false;
volatile bool connectedThingsboard = false;


// Data Can
byte pids[] = {PID_SPEED, PID_RPM, PID_AMBIENT_TEMP, PID_THROTTLE, PID_MAF_FLOW};
int values[5];

// GPS config
// MEMS config
// ICM_20948_I2C mems;
MEMS_I2C* mems = 0;

class DataLogger {
public:
    bool checkState(byte flags) { return (m_state & flags) == flags; }
    void setState(byte flags) { m_state |= flags; }
    void clearState(byte flags) { m_state &= ~flags; }    
private:
    byte m_state = 0;
};

class SerialDataOutput : public FileLogger
{
    void write(const char* buf, byte len)
    {
#if ENABLE_SERIAL_OUT
        Serial.println(buf);
#endif
    }
};

DataLogger logger;

// SD card
// SDLogger store(new SerialDataOutput);

SDLogger store;

int fileid = 0;
uint16_t lastSizeKB = 0;

// functions 
bool initWifi();
void taskFreematics(void *pvParameters);
void pvThingsBoardServer(void *pvParameters);

void taskFreematics(void *pvParameters) {
    while(initObd == false) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    for(int i = 0; i < 5; i++) {
        if(initGps) {
            break;
        }
        Serial.println("Waiting init GPS.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    for(int i = 0; i < 5; i++) {
        if(initMems) {
            break;
        }
        Serial.println("Waiting init MEMS.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    for(int i = 0; i < 5; i++) {
        if(initSD) {
            break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    for(;;) {
        if (obd.getState() == OBD_CONNECTED) {

            byte count = obd.readPID(pids,5,values);
           /*
	    uint8_t dataList[2] = {(uint8_t)vehicleData.speed, (uint8_t)vehicleData.rpm};
            uint8_t valueObd[5];
            int valueObd_0;
            for(int i = 0; i < 5; i++) {
                obd.readPID(pids[i], valueObd_0);
                valueObd[i] = valueObd_0;
            }
            uint32_t valuesData[] = {(uint32_t)1024};
            uint32_t valuesData02[] = {(uint32_t)values[1]};
            for(int i = 0;i < 15;i++) {
                store.log(PID_SPEED, valuesData, (uint8_t)count);
                store.log(PID_SPEED, valuesData, (uint8_t)count);
                store.log(PID_SPEED, valuesData, (uint8_t)count);
                store.log(PID_SPEED, valuesData, (uint8_t)count);
                store.log(PID_SPEED, valuesData, (uint8_t)count);

            }*/ 
            if (count > 0) {
                mutexVehicle.lock();
                vehicleData.speed = values[0];
                vehicleData.rpm = values[1];
                vehicleData.temperature = values[2];
                vehicleData.timestamp = millis();
                mutexVehicle.unlock();
                Serial.printf("[OBD] speed: %d km/h, RPM: %d, temp: %d°C, throttle: %d, maf: %d\n", values[0], values[1], values[2], values[3], values[4]);
            }

            static uint32_t lastVoltageRead = 0;
            if (millis() - lastVoltageRead > 5000) {
                float voltage = obd.getVoltage();
                vehicleData.voltage = voltage;
                lastVoltageRead = millis();
                Serial.printf("[OBD] Voltage: %.2f V\n", voltage);
            }
        } else {
            Serial.println("[OBD] Disconnected, trying to reconnect... ");
            obd.init(PROTO_AUTO, true);
        }

        if(initGps)
        {
            if (freematics.gpsGetData(&gd)) {
                mutexGPS.lock();
                gpsData.lat = gd->lat;
                gpsData.lng = gd->lng;
                gpsData.altitude = gd->alt;
                gpsData.speed = gd->speed * 1.852; // Convert from knots to km/h
                gpsData.satellites = gd->sat;
                gpsData.timestamp = millis();
                mutexGPS.unlock();
                Serial.printf("[GPS] Lat: %.6f, Lng: %.6f, Alt: %.1fm, Vel: %.1f km/h, Sats: %d\n", gd->lat, gd->lng, gd->alt, gd->speed * 1.852, gd->sat);
            }
        }

        if(initMems)
        {
            mutexMEMS.lock();
            mems->read(dataMEMS.accel,dataMEMS.gyro, dataMEMS.mag);
            mutexMEMS.unlock();
            
            Serial.println("Acc     Gyro    Mag");
            for(int i = 0; i <= 2; i++) {
                 Serial.printf("%f     %f    %f\n", dataMEMS.accel[i], dataMEMS.gyro[i], dataMEMS.mag[i]);
             }
            Serial.println("");
        }

        if (initSD) 
        {
            /*
            mutexVehicle.lock();
            writeSD.log(PID_SPEED,ELEMENT_UINT16, &value);
            writeSD.log(PID_RPM, vehicleData.rpm);
            writeSD.log(PID_AMBIENT_TEMP, vehicleData.temperature);
            writeSD.log(PID_TIMESTAMP, vehicleData.timestamp);
            mutexVehicle.unlock();

            mutexGPS.lock();
            writeSD.log(PID_GPS_ALTITUDE, gpsData.altitude);
            writeSD.log(PID_GPS_LATITUDE, gpsData.lat);
            writeSD.log(PID_GPS_LONGITUDE, gpsData.lng);
            writeSD.log(PID_GPS_SPEED, gpsData.speed);
            writeSD.log(PID_GPS_SAT_COUNT, gpsData.satellites);
            writeSD.log(PID_GPS_TIME, gpsData.timestamp);
            mutexGPS.unlock();

            mutexMEMS.lock();
            store.log(PID_ACC, dataMEMS.accel[0],dataMEMS.accel[1], dataMEMS.accel[2]);
            store.log(PID_GYRO, dataMEMS.gyro[0], dataMEMS.gyro[1], dataMEMS.gyro[2]);
            mutexMEMS.unlock();
            */
        }
        // mutexGPS.lock();
        // mutexGPS.unlock();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void postData(String payload, const char* thingsboard_server, const char* token) {
    http.begin("https://" + String(thingsboard_server) + "/api/v1/" + token + "/telemetry");
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);

    Serial.print("HTTP Code: "); Serial.println(httpCode);
    if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Resposta: " + response);
    } else {
        Serial.println("Falha na conexão HTTP");
    }
    http.end();
}

bool initWifi() {
    Serial.print("[WIFI] connecting...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI("WIFI","Connected!");
        Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
        connectedWifi = true;
#ifdef ENCRYPTED
        wifiClient.setInsecure();
#endif // ENCRYPTED
        // wifiClient.setCACert(); // CA for HTTPS
        return true;
    } else {
        // ESP_LOGE("WIFI","Failed to start the WiFi!");
        Serial.println("[WIFI] Failed to start the WiFi!");
        connectedWifi = false;
        return false;
    }
}

void modulesStatus() {
    Serial.printf("Status Device OBD: %s, GPS: %s, MEMS: %s, SD: %s, Wifi: %s\n", initObd ? "true":"false", initGps ? "true":"false", initMems ? "true":"false", initSD ? "true":"false", connectedWifi ? "connected":"desconnected");
}

void setup() 
{
    Serial.begin(115200);

    if(!freematics.begin(true, true)) {
        Serial.println("[Freematics] Failed to start the Freematics!");
        return;
    }
    Serial.println("[Freematics] Initialized!");

    // init GPS
    if (freematics.gpsBegin()) {
        Serial.println("[GPS] Initialized!");
        initGps = true;
    } else {
        Serial.println("[GPS] Failed to start the GPS!");
    }

    // Init OBD
    if (freematics.link) {
        obd.begin(freematics.link);
        if (obd.init(PROTO_AUTO, true)) {
            initObd = true;
            Serial.println("[OBD] Connected and initialized!");

            // read VIN
            char vin[18];
            if(obd.getVIN(vin, sizeof(vin))) {
                Serial.printf("[OBD] VIN: %s\n", vin);
            }
            initObd = true;
        } else {
            Serial.println("[OBD] Failed to start connection");
        }
    } else {
        Serial.println("[OBD] Link not available");
    }

    // init MEMS
    if (!logger.checkState(STATE_MEMS_READY)) do {
        Serial.print("MEMS:");
        mems = new ICM_42627;
        byte ret = mems->begin();
        if (ret) {
            logger.setState(STATE_MEMS_READY);
            Serial.println("ICM-42627");
            initMems = true;
            break;
        }
        delete mems;
        mems = new ICM_20948_I2C;
        ret = mems->begin();
        if (ret) {
            logger.setState(STATE_MEMS_READY);
            Serial.println("ICM-20948");
            initMems = true;
            break;
        }
        delete mems;
        mems = new MPU9250;
        ret = mems->begin();
        if (ret) {
            logger.setState(STATE_MEMS_READY);
            Serial.println("MPU-9250");
            initMems = true;
            break;
        } 
        Serial.println("NO");
    } while (0);
    // init SD
    /*
    Serial.println("");
    int volsize = store.begin();
    if (volsize > 0) {
        Serial.println(volsize);
        Serial.println("MB");
        logger.setState(STATE_STORE_READY);
        initSD = true;
    } else {
        Serial.println("Fail in init SD card");
        initSD = false;
    }
    */
   if(!logger.checkState(STATE_STORE_READY)) {
        if(store.init()) {
            logger.setState(STATE_STORE_READY);
        }
   }
   if (logger.checkState(STATE_STORE_READY)) {
        fileid = store.begin();
   }
    // init WiFi
    initWifi();

    // FreeRTOS Tasks
    xTaskCreatePinnedToCore(
        taskFreematics,
        "TaskFreematcs",
        8192,
        NULL,
        2,
        NULL,
        1
    );

    // xTaskCreatePinnedToCore(
    //     pvThingsBoardServer,
    //     "ThingsBoardServer",
    //     2048,
    //     NULL,
    //     2,
    //     NULL,
    //     0
    // );
}


void loop()
{
    if (connectedWifi == false) {
        initWifi();
    }

    // if (obd.getState() == OBD_CONNECTED) {

    //     byte count = obd.readPID(pids,5,values);

    //     if (count > 0) {
    //         vehicleData.speed = values[0];
    //         vehicleData.rpm = values[1];
    //         vehicleData.temperature = values[2];
    //         vehicleData.timestamp = millis();
    //         Serial.printf("[OBD] speed: %d km/h, RPM: %d, temp: %d°C, throttle: %d, maf: %d\n", values[0], values[1], values[2], values[3], values[4]);
    //     }
        
    //     static uint32_t lastVoltageRead = 0;
    //     if (millis() - lastVoltageRead > 5000) {
    //         float voltage = obd.getVoltage();
    //         vehicleData.voltage = voltage;
    //         lastVoltageRead = millis();
    //         Serial.printf("[OBD] Voltage: %.2f V\n", voltage);
    //     }
    // } else {
    //     Serial.println("[OBD] Disconnected, trying to reconnect... ");
    //     obd.init(PROTO_AUTO, true);
    // }

    // if (freematics.gpsGetData(&gd)) {
    //     gpsData.lat = gd->lat;
    //     gpsData.lng = gd->lng;
    //     gpsData.altitude = gd->alt;
    //     gpsData.speed = gd->speed * 1.852; // Convert from knots to km/h
    //     gpsData.satellites = gd->sat;
    //     gpsData.timestamp = millis();
    //     Serial.printf("[GPS] Lat: %.6f, Lng: %.6f, Alt: %.1fm, Vel: %.1f km/h, Sats: %d\n", gd->lat, gd->lng, gd->alt, gd->speed * 1.852, gd->sat);
    // }
    /*
    mems->read(dataMEMS.accel,dataMEMS.gyro, dataMEMS.mag);
    Serial.println("Acc     Gyro    Mag");
    for(int i = 0; i <= 2; i++) {
        Serial.printf("%f     %f    %f\n", dataMEMS.accel[i], dataMEMS.gyro[i], dataMEMS.mag[i]);
    }
    Serial.println("");
    */
    // Serial.printf("%d",mems->read(dataMEMS.accel));
    if(initObd) {
        mutexVehicle.lock();
        dataServer.speed = vehicleData.speed;
        dataServer.rpm = vehicleData.rpm;
        dataServer.temperature = vehicleData.temperature;
        dataServer.timestamp = vehicleData.timestamp;
        mutexVehicle.unlock();
    }
    
    if(initGps) {
        mutexGPS.lock();
        dataServer.gps_altitude = gpsData.altitude;
        dataServer.gps_lat = gpsData.lat;
        dataServer.gps_lng = gpsData.lng;
        dataServer.gps_satellites = gpsData.satellites;
        dataServer.gps_speed = gpsData.speed;
        dataServer.gps_timestamp = gpsData.timestamp;
        mutexGPS.unlock();
    }

    if (initMems) {
        mutexMEMS.lock();
        for(int i = 0; i <= 2; i++) {
            dataServer.mems_accel[i] = dataMEMS.accel[i];
            dataServer.mems_gyro[i] = dataMEMS.gyro[i];
            // Serial.printf("ACC: %f GYRO: %f", dataServer.mems_accel[i], dataServer.mems_gyro[i]);
        }
        dataServer.mems_orientation = dataMEMS.orientation;
        dataServer.mems_temperature = dataMEMS.temperature;
        dataServer.mems_timestamp = dataMEMS.timestamp;
        mutexMEMS.unlock();
    }

    // Teste Json
    StaticJsonDocument<512> doc;

    doc["gps_lat"] = dataServer.gps_lat;
    doc["gps_lng"] = dataServer.gps_lng;
    doc["gps_altitude"] = dataServer.gps_altitude;
    doc["gps_satellites"] = dataServer.gps_satellites;
    doc["gps_speed"] = dataServer.gps_speed;
    doc["gps_timestamp"] = dataServer.gps_timestamp;
    
    doc["speed"] = dataServer.speed;
    doc["rpm"] = dataServer.rpm;
    doc["ttemperature"] = dataServer.temperature;
    doc["timestamp"] = dataServer.timestamp;

    // ex: {gps_teste:{gps_lat:9.99, gps_lng:10.8}}
    JsonObject nested = doc.createNestedObject("mems_accel");
    nested["x"] = dataServer.mems_accel[0];
    nested["y"] = dataServer.mems_accel[1];
    nested["z"] = dataServer.mems_accel[2];

    JsonObject gyro = doc.createNestedObject("mems_gyro");
    gyro["x"] = dataServer.mems_gyro[0];
    gyro["y"] = dataServer.mems_gyro[1];
    gyro["z"] = dataServer.mems_gyro[2];

    String payload;
    serializeJson(doc, payload);
    size_t jsonSize = payload.length();

    Serial.print("JSON a enviar (");
    Serial.print(jsonSize);
    Serial.print(" bytes): ");
    Serial.println(payload);
    
    /*
    if(connectedWifi) {
        for(int i = 0; i < 5; i++) {
            break;
            if(tb.sendTelemetryJson(doc, jsonSize)) {
                break;
            }
        }
        // tb.sendTelemetryData("homeTeste",123);
    }
    */

    
    doc.clear();

    postData(payload, THINGSBOARD_SERVER, TOKEN);

    /*
    http.begin("https://" + String(THINGSBOARD_SERVER) + "/api/v1/" + TOKEN  + "/telemetry");
    http.addHeader("Content-Type", "application/json");

    String payload_2 = "{\"teste\":456}";
    int httpCode = http.POST(payload);

    Serial.print("HTTP Code: "); Serial.println(httpCode);
    if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Resposta: " + response);
    } else {
        Serial.println("Falha na conexão HTTP");
    }
    http.end();
    */
    modulesStatus();

    /*if(connectedWifi) {

        tb.sendTelemetryData("multithreading","teste");
        tb.sendTelemetryData("speed", dataServer.speed);
        tb.sendTelemetryData("rpm", dataServer.rpm);
        tb.sendTelemetryData("temperature", dataServer.temperature);
        tb.sendTelemetryData("timestamp", dataServer.timestamp);
        tb.sendTelemetryData("gps_speed", dataServer.gps_speed);
        
    }*/
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
