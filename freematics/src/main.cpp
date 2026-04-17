#include <FreematicsPlus.h>
#include <FreematicsBase.h>
#include <FreematicsMEMS.h>
#include <FreematicsOBD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <Arduino_HTTP_Client.h>
#include <ThingsBoardHttp.h>
#include <ThingsBoard.h>

#include <ArduinoJson.h>

#include "config.h"
// lib SD card
#include "FreematicsSD.h"

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

#define HTTP_POST_PATH "Content-Type: application/json"

// macros
#ifdef ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 443U;
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
MIIEqTCCA5GgAwIBAgIQd70NdT8uGWAb1U4KAkRGdjANBgkqhkiG9w0BAQsFADBM
MSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEGA1UEChMKR2xv
YmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0yMDA3MDUwMDAwMDBaFw0y
NzA0MjUxMTAwMDBaMFAxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWdu
IG52LXNhMSYwJAYDVQQDEx1UcnVzdGVkIFJvb3QgVExTIENBIFNIQTI1NiBHMzCC
ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKXRe6ZnoR+4xcoJL/S5vx1D
qMMe4LCFB+zMMOCi5stfYUfK6bBgilYKilk42F85z5o1eLKUfj4ycGfnaUVLdSsj
waP1HMlobr2xBMiH9+q1KgyWui1v8KRv+IhdPaxRojgLLM4jlPLlHvqvorcmGHQl
x0wi2VM8Grqf+cdkhYXHGTBRvBLhAofAhgoWTVzrwTZ6MtAPaCATJ+gcSVQHw9LL
XM7xVQukmUwMMpp1mixfKbmIv+DETaMk9O+kt5R4nPLt7M8lUS1CaezdivbKvvnL
4Wnc7nXzRC/NNLtuWs+6i154Yj4a9JB19oGBrInMm673GanbB1r8IuZoROKdGakC
AwEAAaOCAYEwggF9MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD
AQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQU3k/X3Seu1X9Y
geEsR6wjt8Z7V+8wHwYDVR0jBBgwFoAUj/BLf6guRSSuTVD6Y5qL3uLdG7wwegYI
KwYBBQUHAQEEbjBsMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5nbG9iYWxzaWdu
LmNvbS9yb290cjMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9zZWN1cmUuZ2xvYmFsc2ln
bi5jb20vY2FjZXJ0L3Jvb3QtcjMuY3J0MDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6
Ly9jcmwuZ2xvYmFsc2lnbi5jb20vcm9vdC1yMy5jcmwwRwYDVR0gBEAwPjA8BgRV
HSAAMDQwMgYIKwYBBQUHAgEWJmh0dHBzOi8vd3d3Lmdsb2JhbHNpZ24uY29tL3Jl
cG9zaXRvcnkvMA0GCSqGSIb3DQEBCwUAA4IBAQAW8OVBjtYaLZ+QvyJta++B4k8B
DklbV6XCiJaapjYbZoHssAMd4o+GDr24R0MOmm83sWLdo4gOuNlEwhSVyezr42EJ
2QBLZEJSZwHnMpfPddc2mlZ5gMN6PECZrz4W+t2xmSgOYQi1uIHIiDAoSm4/eppe
8/SLClJTYk/mgF5FfnmNDeOhDE+e836s83RyaVzLxTbl3ZfDSCw2WWglSbrXheWg
ydDeEOmWtqeQjlrTmVX/UwFutWPc4W+v8xZ9KT3vcpG4PZd3ZH0GnckeNjBcFiAi
dXm4Aymp9y9XHFAsTfHPH3UMmR7WIVc+iboT/14tlK9n2oASLiJjPaACGnpU
-----END CERTIFICATE-----
)";
WiFiClientSecure wifiClient;
#else
    constexpr uint16_t THINGSBOARD_PORT = 8080U;
    WiFiClient wifiClient;
#endif

Arduino_HTTP_Client httpClient(wifiClient, THINGSBOARD_SERVER, THINGSBOARD_PORT);
ThingsBoardHttp tb(httpClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT, true, 1024);
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
const char* pidsName[] = {"speed","rpm","ambient_temp","throttle","maf_flow"};
int values[sizeof(pids)/sizeof(pids[0])];
// GPS config
const char *gpsList[] = {"gps_lat","gps_lng","gps_altitude","gps_speed","gps_satellites","gps_timestamp"};
// MEMS config
const char *memsList[] = {"accel_x","accel_y","accel_z","gyro_x","gyro_y","gyro_z","mems_temperature","mems_timestamp"};
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


DataLogger logger;

int fileid = 0;
uint16_t lastSizeKB = 0;

TaskSD taskSD;

// functions 
bool initWifi();
void collectCan();
void collectGps();
void collectGyro();
void taskFreematics(void *pvParameters);

void collectFreematicsData(void *pvParameters) {
    taskSD.checkSD(); // check and init SD card
    taskSD.fileSD();	// Create new 
    
    for(int i = 0; i < sizeof(pidsName)/sizeof(pidsName[0]); i++) {
        taskSD.logDataChar(pidsName[i]);
        taskSD.logDataChar(",");
    }
    for (int i = 0; i < sizeof(gpsList)/sizeof(gpsList[0]); i++) {
        taskSD.logDataChar(gpsList[i]);
        taskSD.logDataChar(",");
    }
    for(int i = 0; i < sizeof(memsList)/sizeof(memsList[0]); i++) {
        taskSD.logDataChar(memsList[i]);
        if(i < sizeof(memsList)/sizeof(memsList[0])) taskSD.logDataChar(",");
    }
    taskSD.logDataChar("\n");
    for(;;){
        if (initObd) {
            collectCan();
        }
        if (initGps) {
            collectGps();
        }
        if (initMems) {
            collectGyro();
        }
        if (taskSD.statusSD()) {
            // writing can data in SD card
            if (initObd) {
                for (int i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
                    taskSD.logDataCan(values[i]);
                    if (i < sizeof(values)/sizeof(values[0])) taskSD.logDataChar(",");
                }
            }
            // writing gps data in SD card
            if (initGps) {
                taskSD.logDataFloatCan(gpsData.lat);
                taskSD.logDataChar(",");
                taskSD.logDataFloatCan(gpsData.lng);
                taskSD.logDataChar(",");
                taskSD.logDataFloatCan(gpsData.altitude);
                taskSD.logDataChar(",");
                taskSD.logDataFloatCan(gpsData.speed);
                taskSD.logDataChar(",");
                taskSD.logDataCan(gpsData.satellites);
                taskSD.logDataChar(",");
                taskSD.logDataCan(gpsData.timestamp);
                taskSD.logDataChar(",");
            }
            // writing mems data in SD card
            if (initMems) {
                for(int i = 0; i < 3; i++){ 
                    taskSD.logDataFloatCan(dataMEMS.accel[i]);
                    taskSD.logDataChar(",");
                }
                for(int i = 0; i < 3; i++){ 
                    taskSD.logDataFloatCan(dataMEMS.gyro[i]);
                    if(i < 3) taskSD.logDataChar(",");
                }                
            }

            taskSD.logDataChar("\n");

        if (taskSD.size()/(1024*1024*1024) >= 1) {
            Serial.printf("File size: %dGB",taskSD.size()/(1024*1024*1024));
            taskSD.close();
            taskSD.fileSD();
            // writing hearder in SD card
            for(int i = 0; i < sizeof(pidsName)/sizeof(pidsName[0]); i++) {
                taskSD.logDataChar(pidsName[i]);
                taskSD.logDataChar(",");
            }
            for (int i = 0; i < sizeof(gpsList)/sizeof(gpsList[0]); i++) {
                taskSD.logDataChar(gpsList[i]);
                taskSD.logDataChar(",");
            }
            for(int i = 0; i < sizeof(memsList)/sizeof(memsList[0]); i++) {
                taskSD.logDataChar(memsList[i]);
                if(i < sizeof(memsList)/sizeof(memsList[0])) taskSD.logDataChar(",");
            }

        } else if(taskSD.size()/(1024*1024) >= 1) {
            Serial.printf("\nFile size: %dMB\n", taskSD.size()/(1024*1024));
        } else if(taskSD.size()/1024 >= 1) {
    	    Serial.printf("\nFile size: %dKB\n", taskSD.size()/1024);
        } else {
            Serial.printf("\nFile size: %dB\n", taskSD.size());
        }

        } else {
            taskSD.checkSD();
            taskSD.fileSD();
        }
        taskSD.tempClose();
        taskSD.tempOpen();

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void collectCan() {
    obd.readPID(pids,5,values);
    if (obd.getState() == OBD_CONNECTED) {
        byte count = obd.readPID(pids,5,values);
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
}

void collectGps() {
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

void collectGyro() {
    mutexMEMS.lock();
    mems->read(dataMEMS.accel,dataMEMS.gyro, dataMEMS.mag);
    mutexMEMS.unlock();
    
    Serial.println("Acc\tGyro\tMag");
    for(int i = 0; i <= 2; i++) {
        Serial.printf("%2f\t%2f\t%2f\n", dataMEMS.accel[i], dataMEMS.gyro[i], dataMEMS.mag[i]);
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
    Serial.printf("\nStatus Device OBD: %s,\t GPS: %s,\t MEMS: %s,\t SD: %s,\t Wifi: %s\n", initObd ? "true":"false", initGps ? "true":"false", initMems ? "true":"false", initSD ? "true":"false", connectedWifi ? "connected":"desconnected");
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

    // init WiFi
    initWifi();

    // FreeRTOS Tasks

    xTaskCreatePinnedToCore(
        collectFreematicsData,
        "collectFreematicsData",
        8192,
        NULL,
        2,
        NULL,
        1
    );
}

void loop()
{
    /*
    if (connectedWifi == false) {
        initWifi();
    }
    */

    while (initWifi() == false);

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
        }
        dataServer.mems_orientation = dataMEMS.orientation;
        dataServer.mems_temperature = dataMEMS.temperature;
        dataServer.mems_timestamp = dataMEMS.timestamp;
        mutexMEMS.unlock();
    }

    // Teste Json
    StaticJsonDocument<1024> doc;

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

    postData(payload, THINGSBOARD_SERVER, TOKEN);

    doc["ThingsboardStatus"] = "sucesso";
    if(tb.sendTelemetryJson(doc, jsonSize)){ 
        Serial.println("Sucesso");
        
    } else {
        Serial.println("Falha");
    }
    doc.clear();

    // modulesStatus(); // print Status modules

    vTaskDelay(500 / portTICK_PERIOD_MS);
}
