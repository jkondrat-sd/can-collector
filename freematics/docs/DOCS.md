Freematics OBD + Thingsboard
============================

Informações e orientações sobre cada funcionalidade do Thingsboard e Freematics OBD. O código utiliza o <a href="https://github.com/thingsboard/thingsboard-client-sdk/tree/master">Thingsboard</a> na versão 0.15.0, e a bilioteca <a href="https://github.com/stanleyhuangyc/Freematics/tree/master">FreematicsPlus.h</a> do FreematicsOBD. O disposipositivo utilizado é o Freematics OBD One Plus modelo H, essa versão possui atualizações de hardware superiores as versões A e B.

<h3>Detalhes e informações sobre o <a href="https://freematics.com/pages/products/freematics-one-plus-model-h/">modelo H</a></h3>
<table border="1" cellpadding="5" width="100%">
	<tbody>
		<tr>
			<th>&nbsp;</th>
			<th>SIM7600E-H</th>
			<th>SIM7600A-H</th>
		</tr>
		<tr>
			<td>Mobile Network Bands</td>
			<td>LTE-TDD B38/B40/B41<br>
			LTE-FDDB1/B3/B5/B7/B8/B20<br>
			UMTS/HSPA+ B1/B5/B8<br>
			GSM/GPRS/EDGEB3/B8</td>
			<td>
			<p>LTE-FDD B2/B4/B12<br>
			UMTS/HSPA+ B2/B5</p>
			</td>
		</tr>
		<tr>
			<td>Data Transfer Speed</td>
			<td colspan="2">LTE CAT4: Uplink up to 50Mbps, Downlink up to 150Mbps<br>
			HSPA+: Uplink up to 5.76Mbps, Downlink up to 42 Mbps<br>
			UMTS: Uplink/Downlink up to 384Kbps<br>
			EDGE: Uplink/Downlink up to 236.8Kbps<br>
			GPRS: Uplink/Downlink up to 85.6Kbps</td>
		</tr>
		<tr>
			<td>Regions</td>
			<td>Europe, Asia, Australia</td>
			<td>North America (AT&amp;T Certified)</td>
		</tr>
	</tbody>
</table>

<h3>Comparação de Modelos</h3>

<table border="1" cellpadding="5" width="100%">
	<tbody>
		<tr>
			<th>&nbsp;</th>
			<th width="37%">Model H</th>
			<th width="37%"><a href="https://freematics.com/store/index.php?route=product/product&amp;product_id=85">Model A</a></th>
		</tr>
		<tr>
			<td>RAM Configuration</td>
			<td>520KB IRAM + 8MB PSRAM</td>
			<td>520KB IRAM</td>
		</tr>
		<tr>
			<td>Flash Memory</td>
			<td>16MB</td>
			<td>4MB</td>
		</tr>
		<tr>
			<td>RTC</td>
			<td>External 32K</td>
			<td>Built-in (less accurate)</td>
		</tr>
		<tr>
			<td>Cellular Module</td>
			<td>Integrated 4G LTE CAT4 module</td>
			<td>Optional cellular module</td>
		</tr>
		<tr>
			<td>GNSS</td>
			<td>Integrated M8030 10Hz GNSS module and antenna</td>
			<td>Via external GNSS receiver</td>
		</tr>
		<tr>
			<td>External I/O</td>
			<td>2x GPIO for digital I/O, analog input, serial UART etc.</td>
			<td>Occupied if GNSS receiver is connected</td>
		</tr>
		<tr>
			<td>Co-Processor Features</td>
			<td>Vehicle ECU interfacing<br>
			GNSS data processing</td>
			<td>Vehicle ECU interfacing</td>
		</tr>
		<tr>
			<td>Heavy Vehicle Support</td>
			<td>HD-OBD connector, 24V system, SAE J1939</td>
			<td>N/A</td>
		</tr>
	</tbody>
</table>

**# InitWiFi**
--------------
Inicializa a comunicação WiFi, utilizando a biblioteca WiFi.h nativa do ESP32.
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

### **OBD config**
====================
Essas são configurações e orientações para utilizar o Freematics OBD e seus determindados módulos para a leitura de dados.

Configurações para iniciar a leitura de CAN no Freematics OBD.
```c

// Freematics OBD config
FreematicsESP32 sys; // Instances the system Freematics
COBD obd; // Communication CAN
bool connected = false; // Status communication OBD
unsigned long count = 0;

void ObdReconnect(){
  while(!sys.begin());
  obd.begin(sys.link);
}

void setup()
{
  while (!sys.begin()); // Init system Freematics

  // Initializations
  obd.begin(sys.link);
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

  int value;
  obd.readPID(PID_SPEED,value);
  Serial.printf("speed: %d\n", value);
}
```

-------------------

Configurações para a leitura do giroscópio. Esse código liga o módulo interno e permiete a leitura 
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

------------------------------
Configurações de beep para retorna uma resposta sonora, informando erros ou etapas realizadas pelo Freematics OBD.
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

----------------------------
Configurações para iniciar e utilizar o módulo interno de GPS.
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

---------------------------------------
PIDs de telemeria, dados coletados do veículo, para leitura, armazenamento ou enviar a plataforma.
<a href="../examples/ReadPidsObd/main.cpp">código exemplo.</a>

```c
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
};

void loop()
{
  for(int i = 0; i < sizeof(pidTelemetry)/sizeof(pidTelemetry[0]);i++){
      int value;
      obd.readPID(pidTelemetry[i].pid, value);
      String telemetry = pidTelemetry[i].telemetry;
      Serial.printf("PID: %s VALUE: %d",telemetry, value);
      Serial.println("\n");
  }
}
```

### **Thingsboard config**
==========================
