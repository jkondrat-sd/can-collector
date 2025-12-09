#include <Arduino.h> // Framework
// FreematicsPlus Libs
#include <FreematicsPlus.h>
#include "FreematicsBase.h"
#include "telestore.h"
#include "config.h"
#include "teleclient.h"
// extras dependences
#include "esp_wifi.h"
#include "esp32-hal-timer.h"

#include <WiFiGeneric.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#ifdef ESP32
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif // ESP32
#endif // ESP8266

// OBD Config
#define STATE_MEMS_READY 0x8
// #define ENABLE_MEMS 1
// #define BEEP_CONFIG false
// OBD states
#define STATE_STORAGE_READY 0x1
#define STATE_OBD_READY 0x2
#define STATE_GPS_READY 0x4
#define STATE_MEMS_READY 0x8
#define STATE_NET_READY 0x10
#define STATE_GPS_ONLINE 0x20
#define STATE_CELL_CONNECTED 0x40
#define STATE_WIFI_CONNECTED 0x80
#define STATE_WORKING 0x100
#define STATE_STANDBY 0x200

// // ThingsBoard Config
// #define THINGSBOARD_ENABLE_DEBUG
// #define USING_HTTPS true // using HTTP protocol
// #define ENCRYPTED true // TLS

// Enables sending messages that are bigger than the predefined message size,
// where the message will be sent byte by byte as a fallback instead.
// Requires an additional library, see https://github.com/bblanchon/ArduinoStreamUtils for more information.
// Simply install that library and the feature will be enabled automatically.
// Enables the ThingsBoard class to be fully dynamic instead of requiring template arguments to statically allocate memory.
// If enabled the program might be slightly slower and all the memory will be placed onto the heap instead of the stack.
// #define THINGSBOARD_ENABLE_DYNAMIC 1

// If the THINGSBOARD_ENABLE_DYNAMIC 1 setting causes this error log message to appear [TB] Unable to de-serialize received json data with error (DeserializationError::NoMemory).
// Simply add this configuration line as well.
//#define THINGSBOARD_ENABLE_PSRAM 0


#if USING_HTTPS
#include <Arduino_HTTP_Client.h>
#include <ThingsBoardHttp.h>
#else
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#endif

constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
constexpr char TOKEN[] = "";
constexpr char THINGSBOARD_SERVER[] = "mobilidade.inmetro.gov.br";



#if USING_HTTPS
// HTTP port used to communicate with the server, 80 is the default unencrypted HTTP port,
// whereas 443 would be the default encrypted SSL HTTPS port
#if ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 443U;
#else
constexpr uint16_t THINGSBOARD_PORT = 8080U;
#endif
#else
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
#if ENCRYPTED
constexpr uint16_t THINGSBOARD_PORT = 8883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 1883U;
#endif
#endif

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 128U;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 128U;

// Baud rate for the debugging serial connection
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

#if ENCRYPTED
// See https://comodosslstore.com/resources/what-is-a-root-ca-certificate-and-how-do-i-download-it/
// on how to get the root certificate of the server we want to communicate with,
// this is needed to establish a secure connection and changes depending on the website.
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";
#endif

constexpr char CONNECTING_MSG[] = "Connecting to: (%s) with token (%s)\n";
constexpr char DEVICE_TYPE_KEY[] = "device_type";
constexpr char ACTIVE_KEY[] = "active";
constexpr char SENSOR_VALUE[] = "sensor";

// Initialize underlying client, used to establish a connection
#if ENCRYPTED
WiFiClientSecure espClient;
#else
WiFiClient espClient;
#endif
// Initialize ThingsBoard instance with the maximum needed buffer size
#if USING_HTTPS
// Initalize the Http client instance
Arduino_HTTP_Client httpClient(espClient, THINGSBOARD_SERVER, THINGSBOARD_PORT);
ThingsBoardHttp tb(httpClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);
#else
// Initalize the Mqtt client instance
Arduino_MQTT_Client mqttClient(espClient);
// Initialize used apis
const std::array<IAPI_Implementation*, 0U> apis = {};
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE, Default_Max_Stack_Size, apis);
#endif

// Freematics OBD config
FreematicsESP32 sys; // Instances the system Freematics
COBD obd; // communication CAN
bool connected = false; // status communication OBD
unsigned long count = 0;
GPS_DATA* gd = nullptr; // pointer GPS data

/* MEMS Config*/
#if ENABLE_MEMS
float accBias[3] = {0};
float accSum[3] = {0};
float acc[3] = {0};
float gyr[3] = {0};
float mag[3] = {0};
uint8_t accCount = 0;
#endif

/*  data ICM_42627 */
class GYROSCOPE {
  public:
    // giroscópio (deg/s)
    float gyr_x = 0;
    float gyr_y = 0;
    float gyr_z = 0;
    // Acelerômetro (m/s²)
    float acc_x = 0;
    float acc_y = 0;
    float acc_z = 0;
    // Magnetômetro (uT)
    float mag_x = 0;
    float mag_y = 0;
    float mag_z = 0;
    // msg
    char erroMsgData[33] = "Failed to read sensor data";
    char erroMsgInit[26] = "Uninitialized sensor"; 
};

/* MEMS functions */
MEMS_I2C* mems = 0;
class STATE {
  public:
    bool check(uint16_t flags) {return (state & flags) == flags;}
    void set(uint16_t flags) {state |= flags;}
    void clear(uint16_t flags) {state &= ~flags;}
    uint16_t state = 0;
};

STATE state;

// Functions
void WriteLogObd();
void beep(int duration);
void ObdReconnect();
void cliLogs();
void initialize();
void process();
void InitWiFi();
bool reconnect();

/// @brief It uses the beep (internal OBD sound device) to send sound response
/// @param duration receives a value of type int that determines the time this asset goes
void beep(int duration)
{
  if(BEEP_CONFIG)
    // turn on buzzer at 2000Hz frequency
    // 1, 193, 180 
    //sys.buzzer(2000);
    sys.buzzer(1800);
    delay(duration);
    // turn off buzzer
    sys.buzzer(0);
}

void beepCode(int i)
{
  if(BEEP_CONFIG){
    for(int g; g <= i; g++){
      beep(300);
      delay(50);
    }
  }
}

/* Data model for GPS */
struct MAIN_GPS{
  public:
  uint32_t ts = 0; // timestamp
  uint32_t date = 0; 
  uint32_t time = 0;
  float lat = 0; // latitude
  float lng = 0; // longitude
  float alt = 0; /* meter */
  float speed = 0; /* knot */
  uint16_t heading = 0; /* degree */
  uint8_t hdop = 0; 
  uint8_t sat = 0;  // satellites
  uint16_t sentences = 0; // 
  uint16_t errors = 0; // errors detected
};

int gpsStatus = 0; // Validates GPS startup

typedef struct {
  byte pid;
  String telemetry;
  // String telemetry;
} PIDTELEMETRY;

PIDTELEMETRY pidtelemetry[]={
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
};

void ObdReconnect(){
  while(!sys.begin());
  obd.begin(sys.link);
}

CBufferManager bufman;
Task subtask;

#if STORAGE == STORAGE_SPIFFS
SPIFFSLogger logger;
#elif STORAGE == STORAGE_SD
SDLogger logger;
#endif

#if STORAGE != STORAGE_NONE
int fileid = 0;
uint16_t lastSizeKB = 0;
#endif

void WriteLogObd(){
#if STORAGE == STORAGE_NONE
  return;
#else
  if (!state.check(STATE_STORAGE_READY) || fileid == 0) {
    return;
  }

  logger.timestamp(millis());

  for (int i = 0; i < sizeof(pidtelemetry) / sizeof(pidtelemetry[0]); i++) {
    int value;
    if (obd.readPID(pidtelemetry[i].pid, value)) {
      int32_t v = value;
      logger.log(pidtelemetry[i].pid, &v, 1);
    }
  }

  if (gd && gpsStatus == 0) {
    uint32_t gpsTime = gd->time;
    uint32_t gpsDate = gd->date;
    float lat = gd->lat;
    float lng = gd->lng;
    float alt = gd->alt;
    float kph = gd->speed * 1.852f;
    uint16_t heading = gd->heading;
    uint8_t sat = gd->sat;
    uint8_t hdop = gd->hdop;

    logger.log(PID_GPS_TIME, &gpsTime, 1);
    logger.log(PID_GPS_DATE, &gpsDate, 1);
    logger.log(PID_GPS_LATITUDE, &lat, 1, "%.6f");
    logger.log(PID_GPS_LONGITUDE, &lng, 1, "%.6f");
    logger.log(PID_GPS_ALTITUDE, &alt, 1, "%.1f");
    logger.log(PID_GPS_SPEED, &kph, 1, "%.1f");
    logger.log(PID_GPS_HEADING, &heading, 1);

    if (sat) logger.log(PID_GPS_SAT_COUNT, &sat, 1);
    if (hdop) logger.log(PID_GPS_HDOP, &hdop, 1);
  }

  uint16_t sizeKB = (uint16_t)(logger.size() >> 10);
  if (sizeKB != lastSizeKB) {
    logger.flush();
    lastSizeKB = sizeKB;
    Serial.print("[FILE] ");
    Serial.print(sizeKB);
    Serial.println("KB");
  }
#endif
}

void cliLogs(){
  Serial.println("=== CLI LOGS ===");
  Serial.printf("WiFi SSID: %s\n", WIFI_SSID);
  Serial.printf("WiFi status: %s\n", WiFi.isConnected() ? "connected" : "disconnected");
  if (WiFi.isConnected()) {
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  }

#if STORAGE != STORAGE_NONE
  Serial.printf("Storage: %s\n", STORAGE == STORAGE_SD ? "SD" : "SPIFFS");
  Serial.printf("File ID: %d\n", fileid);
  Serial.printf("File size: %lu bytes\n", (unsigned long)logger.size());
#endif

  Serial.printf("OBD: %s (errors: %d)\n", connected ? "connected" : "disconnected", obd.errors);
  Serial.println("PIDs lidos:");
  for (int i = 0; i < (int)(sizeof(pidtelemetry) / sizeof(pidtelemetry[0])); i++) {
    int value = 0;
    bool ok = connected && obd.readPID(pidtelemetry[i].pid, value);
    Serial.printf("  %s (0x%02X): %s%d\n",
                  pidtelemetry[i].telemetry.c_str(),
                  pidtelemetry[i].pid,
                  ok ? "" : "(fail) ",
                  value);
  }

  if (gd && gpsStatus == 0) {
    Serial.printf("GPS: fix with %u sats HDOP %u\n", gd->sat, gd->hdop);
    Serial.printf("GPS pos: %.6f, %.6f alt: %.1f speed: %.1fkm/h heading: %u\n",
                  gd->lat, gd->lng, gd->alt, gd->speed * 1.852f, gd->heading);
    Serial.printf("GPS date/time: %u / %u ts: %u\n", gd->date, gd->time, gd->ts);
  } else {
    Serial.printf("GPS: no fix (status=%d)\n", gpsStatus);
  }

#if ENABLE_MEMS
  if (state.check(STATE_MEMS_READY) && mems) {
    float accCur[3] = {0}, gyrCur[3] = {0}, magCur[3] = {0};
    if (mems->read(accCur, gyrCur, magCur)) {
      Serial.printf("MEMS ACC: %.2f %.2f %.2f\n", accCur[0], accCur[1], accCur[2]);
      Serial.printf("MEMS GYR: %.2f %.2f %.2f\n", gyrCur[0], gyrCur[1], gyrCur[2]);
      Serial.printf("MEMS MAG: %.2f %.2f %.2f\n", magCur[0], magCur[1], magCur[2]);
    } else {
      Serial.println("MEMS: read failed");
    }
  } else {
    Serial.println("MEMS: not ready");
  }
#else
  Serial.printf("MEMS: %s\n", state.check(STATE_MEMS_READY) ? "ready" : "not ready");
#endif

#if USING_HTTPS
  Serial.printf("ThingsBoard (HTTP) server: %s:%u\n", THINGSBOARD_SERVER, THINGSBOARD_PORT);
#else
  Serial.printf("ThingsBoard (MQTT) connected: %s\n", tb.connected() ? "yes" : "no");
  Serial.printf("ThingsBoard server: %s:%u\n", THINGSBOARD_SERVER, THINGSBOARD_PORT);
#endif
  Serial.println("================");
}

void initialize(){
#if STORAGE != STORAGE_NONE
  if (!state.check(STATE_STORAGE_READY)) {
    // init storage
    if (logger.init()) {
      state.set(STATE_STORAGE_READY);
    }
  }
  if (state.check(STATE_STORAGE_READY)) {
    fileid = logger.begin();
  }
#endif
}
void process(){

  CBuffer* buffer = bufman.getFree();
  buffer->state = BUFFER_STATE_FILLING;

#if STORAGE != STORAGE_NONE
  if (state.check(STATE_STORAGE_READY)) {
    buffer->serialize(logger);
    uint16_t sizeKB = (uint16_t)(logger.size() >> 10);
    if (sizeKB != lastSizeKB) {
      logger.flush();
      lastSizeKB = sizeKB;
      Serial.print("[FILE] ");
      Serial.print(sizeKB);
      Serial.println("KB");
    }
  }
#endif
}

/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  Serial.println("Connected to AP");
#if ENCRYPTED
   espClient.setCACert(ROOT_CERT);
#endif
}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }
  beep(3000);
  delay(100);
  beep(3000);
  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void setup() {
  // If analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  // Initalize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  
  // Sensor initialization ICM_42627
  if(!state.check(STATE_MEMS_READY)){
    // Serial.print("MEMS: ");
    mems = new ICM_42627;
    byte ret = mems->begin();
    if(ret){ // ICM_42627 successfully initialized
      state.set(STATE_MEMS_READY); 
    }else { // Failure to initialize ICM_42627
      delete mems;
      mems = 0;
    }
  }

  while (!sys.begin()); // Inicia o sistema Freematics
  if (sys.gpsBegin()) { /*GPS started successfully */} 
  else { // Failed to start GPS!
    gpsStatus = 1; // Please note that the GPS module has not been initialized
  }

  // initializations
  obd.begin(sys.link);
  delay(1000);
  InitWiFi();
  initialize();
}

void loop() {
#if CLI_LOGS == 1
  cliLogs();
#endif

  MAIN_GPS gps;
  GYROSCOPE gyroscope;
  // if case erro, reconnect comunication with obd
  // ObdReconnect();

  // Checking CAN communication
  if(!connected){
    if(obd.init()){
      connected = true;
    }
    return;
  }

  // detects errors in obd communication and reconnects communication
  if(obd.errors > 2){
    connected = false;
    obd.reset();
  }


  // reconnect wifi
  if (!reconnect()) {
    return;
  }

  /* GPS read */
  int gpsTypeError = 0;
  if (sys.gpsGetData(&gd) && gd != nullptr) { // Gets pointer to GPS data
    if (gd->sat >= 4) { // Check for GPS fix (minimum of 4 satellites)

      gps.lat = gd -> lat, 6; // Latitude
      gps.lng = gd -> lng, 6; // Longitude
      gps.alt = gd -> alt, 2; // Altitude
      gps.speed = gd -> speed * 1.852, 2; // Velocidade, converte nós para km/h
      gps.heading = gd -> heading; // Direção em Graus
      gps.sat = gd -> sat; // Satélites
      gps.hdop = gd -> hdop; // HDOP Precisão
      gps.date = gd -> date; // Data; Formato DDMMAA
      gps.time = gd -> time; // Hora; Formato HHMMSSsss
      gps.ts = gd -> ts; // Timestamp; Milissegundos
    } else {
      gpsTypeError = 2;
    }
  } else {
    gpsTypeError = 1;
  }

  /* Read */
  int gyrosStates = 0;
  if (state.check(STATE_MEMS_READY)){
    if(mems->read(acc, gyr, mag)){
      gyroscope.acc_x = acc[0];
      gyroscope.acc_y = acc[1];
      gyroscope.acc_z = acc[2];

      gyroscope.gyr_x = gyr[0];
      gyroscope.gyr_y = gyr[1];
      gyroscope.gyr_z = gyr[2];

      gyroscope.mag_x = mag[0];
      gyroscope.mag_y = mag[1];
      gyroscope.mag_z = mag[2];
    }else{
      gyrosStates = 1;
    }
  }else{
    gyrosStates = 2;
  }

#if !USING_HTTPS
  if (!tb.connected()) {
    // Reconnect to the ThingsBoard server,
    // if a connection was disrupted or has not yet been established
    Serial.printf(CONNECTING_MSG, THINGSBOARD_SERVER, TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
  }
#endif

  // Uploads new Attributes to ThingsBoard using HTTP.
  // See https://thingsboard.io/docs/reference/http-api/#attributes-api
  // for more details


  // Send attributes in a batch
  Serial.println("Sending attributes batch...");

  constexpr size_t ATTRIBUTES_SIZE = 2U;
  Attribute attributes[ATTRIBUTES_SIZE] = {
    { DEVICE_TYPE_KEY,  SENSOR_VALUE },
    { ACTIVE_KEY,       true     },
  };

  Telemetry* begin = attributes;
  Telemetry* end = attributes + ATTRIBUTES_SIZE;
#if THINGSBOARD_ENABLE_DYNAMIC
  tb.sendAttributes(begin, end);
#else
  tb.sendAttributes<ATTRIBUTES_SIZE>(begin, end);
#endif // THINGSBOARD_ENABLE_DYNAMIC

    int value;
    // device status
    /*tb.sendTelemetryData("device_voltage", obd.getVoltage());
    tb.sendTelemetryData("device_state", obd.getState());
    tb.sendTelemetryData("device_temp",obd.readPID(PID_DEVICE_TEMP, value));
    tb.sendTelemetryData("device_hall", obd.readPID(PID_DEVICE_HALL,value));
    */

    for(int i = 0; i < sizeof(pidtelemetry)/ sizeof(pidtelemetry[0]);i++){
      // printf("o");
      int value;
      obd.readPID(pidtelemetry[i].pid,value);
      // String telemetryData = String(pidtelemetry[i].telemetry);
      String telemetry = pidtelemetry[i].telemetry;
      // const char tetete[] = pidfood[i].name;
      tb.sendTelemetryData(telemetry.c_str(), value);
    }

    // GPS telemetry data
    if( gpsStatus == 0){
      tb.sendTelemetryData("gps_lat", gps.lat);
      tb.sendTelemetryData("gps_lng",gps.lng);
      tb.sendTelemetryData("gps_alt",gps.alt);
      tb.sendTelemetryData("gps_speed",gps.speed);
      tb.sendTelemetryData("gps_heading",gps.heading);
      tb.sendTelemetryData("gps_sat",gps.sat);
      tb.sendTelemetryData("gps_hdop",gps.hdop);
      tb.sendTelemetryData("gps_date",gps.date);
      tb.sendTelemetryData("gps_time",gps.time);
      tb.sendTelemetryData("timestamp",gps.ts);
      Serial.printf(" time: %d",gps.time);
      Serial.printf(" latitude: %d",gps.lat);
      Serial.printf(" longitude: %d", gps.lng);
      Serial.printf(" velociade: %d", gps.speed);
      Serial.printf(" heading: %d", gps.heading);
      Serial.printf(" sat: %d", gps.sat);
      Serial.printf(" hdop: %d", gps.hdop); 
      Serial.printf(" date: %d",gps.date);
      Serial.printf(" timestamp: %d\n", gps.ts);    
    }
      if(gpsStatus == 1){
        tb.sendTelemetryData("gps_msg", "No GPS data available"); // Mensagem caso satélite esteja fora do ar ou não consiga se comunicar
      }else if(gpsStatus == 2){
        tb.sendTelemetryData("gps_msg", "Waiting to fix GPS Satellites");
      } else {
        tb.sendTelemetryData("gps_msg","gps ok");
      }
    //} else {
    //  tb.sendTelemetryData("gps_msg","module gps no active");
    //}

    
    // ICM_42627 telemetry data
    if(gyrosStates == 1){
      tb.sendTelemetryData("msg", gyroscope.erroMsgData);
    } else if (gyrosStates == 2)
    {
      tb.sendTelemetryData("msg", gyroscope.erroMsgInit);
    } else {
      tb.sendTelemetryData("acc_x", gyroscope.acc_x);
      tb.sendTelemetryData("acc_y", gyroscope.acc_y);
      tb.sendTelemetryData("acc_z", gyroscope.acc_z);

      tb.sendTelemetryData("gyr_x", gyroscope.gyr_x);
      tb.sendTelemetryData("gyr_y", gyroscope.gyr_y);
      tb.sendTelemetryData("gyr_z", gyroscope.gyr_z);

      tb.sendTelemetryData("mag_x", gyroscope.mag_x);
      tb.sendTelemetryData("mag_y", gyroscope.mag_y);
      tb.sendTelemetryData("mag_z", gyroscope.mag_z);
    }    

    Serial.printf("Gyroscope:\nx: %d\ny: %d\nz: %d\n",gyroscope.gyr_x, gyroscope.gyr_y, gyroscope.gyr_z);
    Serial.printf("Acelerômetro:\nx: %d\ny: %d\nz: %d\n",gyroscope.acc_x, gyroscope.acc_y, gyroscope.acc_z);
    Serial.printf("Magmometro:\nx: %d\ny: %d\nz: %d\n",gyroscope.mag_x, gyroscope.mag_y, gyroscope.mag_z);

  WriteLogObd();

#if !USING_HTTPS
  tb.loop();
#endif
}
