Freematics OBD + Thingsboard
================
Informações e orientações sobre cada funcionalidade do Thingsboard e Freematics OBD.

**# InitWiFi**
Inicializa a comunicação WiFi
```c
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
  Serial.println("Connected to AP");
#if ENCRYPTED
  espClient.setCACert(ROOT_CERT);
#endif
}
```

### **# OBD config**
Configurações para iniciar a leitura de CAN e instancias do Freematics
```c

// Freematics OBD config
FreematicsESP32 sys; // Instances the system Freematics
COBD obd; // communication CAN
bool connected = false; // status communication OBD
unsigned long count = 0;
GPS_DATA* gd = nullptr; // pointer GPS data

void setup()
{
  while (!sys.begin()); // Inicia o sistema Freematics
  if (sys.gpsBegin()) { /*GPS started successfully */} 
  else { // Failed to start GPS!
    gpsStatus = 1; // Please note that the GPS module has not been initialized
  }

  // initializations
  obd.begin(sys.link);
}
```

Configurações para a leitura do giroscópio.
<a href="../examples/gyroscopeObd/main.cpp">código de exemplo.</a>

```c
#define STATE_MEMS_READY 0x8
#define ENABLE_MEMS true

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

```

Configurações de beep para retorna uma resposta sonora, informando erros ou etapas realizadas pelo OBD.
```c
/// @brief It uses the beep (internal OBD sound device) to send sound response
/// @param duration receives a value of type int that determines the time this asset goes
void beep(int duration)
{
    // turn on buzzer at 2000Hz frequency 
    sys.buzzer(2000);
    delay(duration);
    // turn off buzzer
    sys.buzzer(0);
}
```

Configurações para iniciar e utilizar o GPS.
<a href="../examples/ObdGps/main.cpp">código exemplo.</a>

```c
GPS_DATA* gd = nullptr; // pointer GPS data
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

void setup()
{
  /* Read GPS*/
  if (sys.gpsBegin()) { /*GPS started successfully */} 
   else { // Failed to start GPS!
     gpsStatus = 1; // Please note that the GPS module has not been initialized
   }
}
```

PIDs de telemeria, dados coletados do veículo para enviar a plataforma.
<a href="../examples/ReadPidsObd/main.cpp">código exemplo.</a>

```c
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
};
```
```c
void ObdReconnect(){
  while(!sys.begin());
  obd.begin(sys.link);
}
```

### **# Thingsboard config**