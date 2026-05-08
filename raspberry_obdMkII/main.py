import can
import time
import sys

print("="*32)
print("||READ CAN OBD IN RASPBERRY PI||")
print("="*32)

# configuration of CAN Bus 
INTERFACE = 'can0'
BITRADE = 500000
EXTENDED = False
# ID of request OBD-II
REQUEST_ID = 0x7DF

PIDS_ = {
    #mode:{description:[pid01,pid02]}
    0x01:{
        "PIDs supported [$01 - $20]":[0x00,0x00],
        "Monitor status since DTCs cleared":[0x01,0x00],
        "DTC that caused freeze frame to be stored":[0x02,0x00],
        "Fuel system status":[0x03,0x00],
        "Calculated engine load":[0x04,0x00],
        "Engine coolant temperature":[0x05,0x00],
        "Short term fuel trim (STFT)—Bank 1":[0x06,0x00],
        "Long term fuel trim (LTFT)—Bank 1":[0x07,0x00],
        "Short term fuel trim (STFT)—Bank 2":[0x08,0x00],
        "Long term fuel trim (LTFT)—Bank 2":[0x09,0x00],
        "Fuel pressure (gauge pressure)":[0x0A,0x00],
        "Intake manifold absolute pressure":[0x0B,0x00],
        "Engine speed":[0x0C,0x00],
        "Vehicle speed":[0x0D,0x00],
        "Timing advance":[0x0E,0x00],
        "Intake air temperature":[0x0F,0x00],
        "Mass air flow sensor (MAF) air flow rate":[0x10,0x00],
        "Throttle position":[0x11,0x00],
        "Commanded secondary air status":[0x12,0x00],
        "Oxygen sensors present (in 2 banks)":[0x13,0x00],
        "Oxygen Sensor 1":[0x14,0x00],
        "Oxygen Sensor 2":[0x15,0x00],
        "Oxygen Sensor 3":[0x16,0x00],
        "Oxygen Sensor 4":[0x17,0x00],
        "Oxygen Sensor 5":[0x18,0x00],
        "Oxygen Sensor 6":[0x19,0x00],
        "Oxygen Sensor 7":[0x1A,0x00],
        "Oxygen Sensor 8":[0x1B,0x00],
        "OBD standards this vehicle conforms to":[0x1C,0x00],
        "Oxygen sensors present (in 4 banks)":[0x1D,0x00],
        "Auxiliary input status":[0x1E,0x00],
        "Run time since engine start":[0x1F,0x00],
        "PIDs supported [$21 - $40]":[0x20,0x00],
    	"Distance traveled with malfunction indicator lamp (MIL)":[0x21,0x00],
    	"Fuel Rail Pressure (relative to manifold vacuum)":[0x22,0x00],
    	"Fuel Rail Gauge Pressure":[0x23,0x00],
    	"Oxygen Sensor 1":[0x24,0x00],
    	"Oxygen Sensor 2":[0x25,0x00],
    	"Oxygen Sensor 3":[0x26,0x00],
    	"Oxygen Sensor 4":[0x27,0x00],
    	"Oxygen Sensor 5":[0x28,0x00],
    	"Oxygen Sensor 6":[0x29,0x00],
    	"Oxygen Sensor 7":[0x2A,0x00],
    	"Oxygen Sensor 8":[0x2B,0x00],
    	"Commanded EGR":[0x2C,0x00],
    	"EGR Error":[0x2D,0x00],
    	"Commanded evaporative purge":[0x2E,0x00],
    	"Fuel Tank Level Input":[0x2F,0x00],
    	"Warm-ups since codes cleared":[0x30,0x00],
    	"Distance traveled since codes cleared":[0x31,0x00],
    	"Evap. System Vapor Pressure":[0x32,0x00],
    	"Absolute Barometric Pressure":[0x33,0x00],
    	"Oxygen Sensor 1":[0x34,0x00],
    	"Oxygen Sensor 2":[0x35,0x00],
    	"Oxygen Sensor 3":[0x36,0x00],
    	"Oxygen Sensor 4":[0x37,0x00],
    	"Oxygen Sensor 5":[0x38,0x00],
    	"Oxygen Sensor 6":[0x39,0x00],
    	"Oxygen Sensor 7":[0x3A,0x00],
    	"Oxygen Sensor 8":[0x3B,0x00],
    	"Catalyst Temperature: Bank 1, Sensor 1":[0x3C,0x00],
    	"Catalyst Temperature: Bank 2, Sensor 1":[0x3D,0x00],
    	"Catalyst Temperature: Bank 1, Sensor 2":[0x3E,0x00],
    	"Catalyst Temperature: Bank 2, Sensor 2":[0x3F,0x00],
    	"PIDs supported [$41 - $60]":[0x40,0x00],
    	"Monitor status this drive cycle":[0x41,0x00],
    	"Control module voltage":[0x42,0x00],
    	"Absolute load value":[0x43,0x00],
    	"Commanded Air-Fuel Equivalence Ratio":[0x44,0x00],
    	"Relative throttle position":[0x45,0x00],
    	"Ambient air temperature":[0x46,0x00],
    	"Absolute throttle position B":[0x47,0x00],
    	"Absolute throttle position C":[0x48,0x00],
    	"Absolute throttle position D":[0x49,0x00],
    	"Absolute throttle position E":[0x4A,0x00],
    	"Absolute throttle position F":[0x4B,0x00],
    	"Commanded throttle actuator":[0x4C,0x00],
    	"Time run with MIL on":[0x4D,0x00],
    	"Time since trouble codes cleared":[0x4E,0x00],
    	"Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure":[0x4F,0x00],
    	"Maximum value for air flow rate from mass air flow sensor":[0x50,0x00],
    	"Fuel Type":[0x51,0x00],
    	"Ethanol fuel":[0x52,0x00],
    	"Absolute Evap system Vapor Pressure":[0x53,0x00],
    	"Evap system vapor pressure":[0x54,0x00],
    	"Short term secondary oxygen sensor trim, A: bank 1, B: bank 3":[0x55,0x00],
    	"Short term secondary oxygen sensor trim, A: bank 1, B: bank 3":[0x56,0x00],
    	"Short term secondary oxygen sensor trim, A: bank 2, B: bank 4":[0x57,0x00],
    	"Short term secondary oxygen sensor trim, A: bank 2, B: bank 4":[0x58,0x00],
    	"Fuel rail absolute pressure":[0x59,0x00],
    	"Relative accelerator pedal position":[0x5A,0x00],
    	"Hybrid battery pack remaining life":[0x5B,0x00],
    	"Engine oil temperature":[0x5C,0x00],
    	"Fuel injection timing":[0x5D,0x00],
    	"Engine fuel rate":[0x5E,0x00],
    	"Emission requirements to which vehicle is designed":[0x5F,0x00],
    	"PIDs supported [$61 - $80]":[0x60,0x00],
    	"Driver's demand engine - percent torque":[0x61,0x00],
    	"Actual engine - percent torque":[0x62,0x00],
    	"Engine reference torque":[0x63,0x00],
    	"Engine percent torque data":[0x64,0x00],
    	"Auxiliary input / output supported":[0x65,0x00],
    	"Mass air flow sensor":[0x66,0x00],
    	"Engine coolant temperature":[0x67,0x00],
    	"Intake air temperature sensor":[0x68,0x00],
    	"Actual EGR, Commanded EGR, and EGR Error":[0x69,0x00],
    	"Commanded Diesel intake air flow control and relative intake air flow position":[0x6A,0x00],
    	"Exhaust gas recirculation temperature":[0x6B,0x00],
    	"Commanded throttle actuator control and relative throttle position":[0x6C,0x00],
    	"Fuel pressure control system":[0x6D,0x00],
    	"Injection pressure control system":[0x6E,0x00],
    	"Turbocharger compressor inlet pressure":[0x6F,0x00],
    	"Boost pressure control":[0x70,0x00],
    	"Variable Geometry turbo (VGT) control":[0x71,0x00],
    	"Wastegate control":[0x72,0x00],
    	"Exhaust pressure":[0x73,0x00],
    	"Turbocharger RPM":[0x74,0x00],
    	"Turbocharger temperature":[0x75,0x00],
    	"Turbocharger temperature":[0x76,0x00],
    	"Charge air cooler temperature (CACT)":[0x77,0x00],
    	"Exhaust Gas temperature (EGT) Bank 1":[0x78,0x00],
    	"Exhaust Gas temperature (EGT) Bank 2":[0x79,0x00],
    	"Diesel particulate filter (DPF) differential pressure":[0x7A,0x00],
    	"Diesel particulate filter (DPF)":[0x7B,0x00],
    	"Diesel particulate filter (DPF) temperature":[0x7C,0x00],
    	"NOx NTE (Not-To-Exceed) control area status":[0x7D,0x00],
    	"PM NTE (Not-To-Exceed) control area status":[0x7E,0x00],
    	"Engine run time":[0x7F,0x00],
    	"PIDs supported [$81 - $A0]":[0x80,0x00],
    	"Engine run time for Auxiliary Emissions Control Device(AECD)":[0x81,0x00],
    	"Engine run time for Auxiliary Emissions Control Device(AECD)":[0x82,0x00],
    	"NOx sensor":[0x83,0x00],
    	"Manifold surface temperature":[0x84,0x00],
    	"NOx reagent system":[0x85,0x00],
    	"Particulate matter (PM) sensor":[0x86,0x00],
    	"Intake manifold absolute pressure":[0x87,0x00],
    	"SCR Induce System":[0x88,0x00],
    	"Run Time for AECD #11-#15":[0x89,0x00],
    	"Run Time for AECD #16-#20":[0x8A,0x00],
    	"Diesel Aftertreatment":[0x8B,0x00],
    	"O2 Sensor (Wide Range)":[0x8C,0x00],
    	"Throttle Position G":[0x8D,0x00],
    	"Engine Friction - Percent Torque":[0x8E,0x00],
    	"PM Sensor Bank 1 & 2":[0x8F,0x00],
    	"WWH-OBD Vehicle OBD System Information":[0x90,0x00],
    	"WWH-OBD Vehicle OBD System Information":[0x91,0x00],
    	"Fuel System Control":[0x92,0x00],
    	"WWH-OBD Vehicle OBD Counters support":[0x93,0x00],
    	"NOx Warning And Inducement System":[0x94,0x00],
    	"Exhaust Gas Temperature Sensor":[0x98,0x00],
    	"Exhaust Gas Temperature Sensor":[0x99,0x00],
    	"Hybrid/EV Vehicle System Data, Battery, Voltage":[0x9A,0x00],
    	"Diesel Exhaust Fluid Sensor Data":[0x9B,0x00],
    	"O2 Sensor Data":[0x9C,0x00],
    	"Engine Fuel Rate":[0x9D,0x00],
    	"Engine Exhaust Flow Rate":[0x9E,0x00],
    	"Fuel System Percentage Use":[0x9F,0x00],
    	"PIDs supported [$A1 - $C0]":[0xA0,0x00],
    	"NOx Sensor Corrected Data":[0xA1,0x00],
    	"Cylinder Fuel Rate":[0xA2,0x00],
    	"Evap System Vapor Pressure":[0xA3,0x00],
    	"Transmission Actual Gear":[0xA4,0x00],
    	"Commanded Diesel Exhaust Fluid Dosing":[0xA5,0x00],
    	"Odometer":[0xA6,0x00],
    	"NOx Sensor Concentration Sensors 3 and 4":[0xA7,0x00],
    	"NOx Sensor Corrected Concentration Sensors 3 and 4":[0xA8,0x00],
    	"ABS Disable Switch State":[0xA9,0x00],
    	"PIDs supported [$C1 - $E0]":[0xC0,0x00],
    	"Fuel Level Input A/B":[0xC3,0x00],
    	"Exhaust Particulate Control System Diagnostic Time/Count":[0xC4,0x00],
    	"Fuel Pressure A and B":[0xC5,0x00],
    	"Byte 1 - Particulate control - driver inducement system status Byte 2,3 - Removal or block of the particulate aftertreatment system counter Byte 4,5 - Liquid regent injection system (e.g. fuel-borne catalyst) failure counter Byte 6,7 - Malfunction of Particulate control monitoring system counter":[0xC6,0x00],
    	"Distance Since Reflash or Module Replacement":[0xC7,0x00],
    	"NOx Control Diagnostic (NCD) and Particulate Control Diagnostic (PCD) Warning Lamp status":[0xC8,0x00]
    },
    0x02:{
        "Show freeze frame data":[0x00,0x00]
    },
    0x03:{
        "Request trouble codes":[0x00,0x00]
    },
    0x04:{
        "Clear trouble codes / Malfunction indicator lamp (MIL) / Check engine light":[0x00,0x00]
    },
    0x05:{
        "OBD Monitor IDs supported ($01 – $20)": [0x01,0x00],
        "O2 Sensor Monitor Bank 1 Sensor 1": [0x01,0x01],
        "O2 Sensor Monitor Bank 1 Sensor 2": [0x01,0x02],
        "O2 Sensor Monitor Bank 1 Sensor 3": [0x01,0x03],
        "O2 Sensor Monitor Bank 1 Sensor 4": [0x01,0x04],
        "O2 Sensor Monitor Bank 2 Sensor 1": [0x01,0x05],
        "O2 Sensor Monitor Bank 2 Sensor 2": [0x01,0x06],
        "O2 Sensor Monitor Bank 2 Sensor 3": [0x01,0x07],
        "O2 Sensor Monitor Bank 2 Sensor 4": [0x01,0x08],
        "O2 Sensor Monitor Bank 3 Sensor 1": [0x01,0x09],
        "O2 Sensor Monitor Bank 3 Sensor 2": [0x01,0x0A],
        "O2 Sensor Monitor Bank 3 Sensor 3": [0x01,0x0B],
        "O2 Sensor Monitor Bank 3 Sensor 4": [0x01,0x0C],
        "O2 Sensor Monitor Bank 4 Sensor 1": [0x01,0x0D],
        "O2 Sensor Monitor Bank 4 Sensor 2": [0x01,0x0E],
        "O2 Sensor Monitor Bank 4 Sensor 3": [0x01,0x0F],
        "O2 Sensor Monitor Bank 4 Sensor 4": [0x01,0x10],
        "O2 Sensor Monitor Bank 1 Sensor 1": [0x02,0x01],
        "O2 Sensor Monitor Bank 1 Sensor 2": [0x02,0x02],
        "O2 Sensor Monitor Bank 1 Sensor 3": [0x02,0x03],
        "O2 Sensor Monitor Bank 1 Sensor 4": [0x02,0x04],
        "O2 Sensor Monitor Bank 2 Sensor 1": [0x02,0x05],
        "O2 Sensor Monitor Bank 2 Sensor 2": [0x02,0x06],
        "O2 Sensor Monitor Bank 2 Sensor 3": [0x02,0x07],
        "O2 Sensor Monitor Bank 2 Sensor 4": [0x02,0x08],
        "O2 Sensor Monitor Bank 3 Sensor 1": [0x02,0x09],
        "O2 Sensor Monitor Bank 3 Sensor 2": [0x02,0x0A],
        "O2 Sensor Monitor Bank 3 Sensor 3": [0x02,0x0B],
        "O2 Sensor Monitor Bank 3 Sensor 4": [0x02,0x0C],
        "O2 Sensor Monitor Bank 4 Sensor 1": [0x02,0x0D],
        "O2 Sensor Monitor Bank 4 Sensor 2": [0x02,0x0E],
        "O2 Sensor Monitor Bank 4 Sensor 3": [0x02,0x0F],
        "O2 Sensor Monitor Bank 4 Sensor 4": [0x02,0x10]
    },
    0x06:{
        "mode 6":[0x00,0x00]
    },
    0x07:{
        "mode 7":[0x00,0x00]
    },
    0x08:{
        "mode 8":[0x00,0x00]
    },
    0x09:{
        "Service 9 supported PIDs ($01 to $20)":[0x00,0x00],
        "VIN Message":[0x01,0x00],
        "VIN":[0x02,0x00],
        "Calibration ID message count for PID 04":[0x03,0x00],
        "Calibration ID":[0x04,0x00],
        "Calibration verification numbers (CVN) message count for PID 06":[0x05,0x00],
        "Calibration Verification Numbers (CVN) Several CVN can be output":[0x06,0x00],
        "In-use performance tracking message count for PID 08 and 0B":[0x07,0x00],
        "In-use performance tracking for spark ignition vehicles":[0x08,0x00],
        "ECU name message count for PID 0A":[0x09,0x00],
        "ECU name":[0x0A,0x00],
        "In-use performance tracking for compression ignition vehicles":[0x0B,0x00],
    },
    0x0A:{
        "mode 10":[0x00,0x00]
    }
}

def send_obd_request(bus, mode, pid01, pid02):
    # print(f'send mode actual: {mode}')
    data = [
        0x03,
        mode,
        pid01,
        pid02,
        0x00, 0x00, 0x00, 0x00
    ]

    msg = can.Message(
        arbitration_id=REQUEST_ID,
        data=data,
        is_extended_id=EXTENDED,
        is_rx=False
    )

    try:
        bus.send(msg)
        return True
    except can.CanError:
        print("Error to get mensage")
        return False

def short_or_long_term_fuel_trim(A):
    return f'{((100/128)*A)-100}%' # or f'{(A/1.28)-100}'

def Oxygen_sensor_type_01(A,B):
    result_A = (A/200)
    result_B = (((100/128)*B)-100)
    return f"{result_A}V {result_B}%"

def oxygen_sensor_type_02(A,B,C,D):
    return f'{((2/65536)*((256*A)+B))}ratio {((8/65536)*((256*C)+D))}V'

def oxygen_sensor_type_03(A,B,C,D):
    return f'{((2/65536)*((256*A)+A))}ratio {(((256*C)+D)/256)-128}mA'

def catalyst_temperature(A,B):
    return f'{(((256*A)+B)/10)-40}°C'

def absolute_throttle_position(A):
    return f'{(100/255)*A}%'

def decode_mode01(msg,pid):
    A = msg.data[3]
    B = msg.data[4]
    C = msg.data[5]
    D = msg.data[6]
    E = msg.data[7]

    match pid:
        case 0x04:
            return f'{(100/255)*A}%' # or f'{A/2.55}'
        
        case 0x05:
            return f'{A-40}°C'

        case 0x06:
            return short_or_long_term_fuel_trim(A)
        case 0x07:
            return short_or_long_term_fuel_trim(A)
        case 0x08:
            return short_or_long_term_fuel_trim(A)
        case 0x09:
            return short_or_long_term_fuel_trim(A)
        
        case 0x0A:
            return f'{3*A}kPa'
        
        case 0x0B:
            return f'{A}kPa'
        
        case 0x0C: # rpm
            B = msg.data[4]
            resultado = ((256*A)+B)/4
            return f"{resultado} rpm"

        case 0x0D: # vehicle speed
            return f"{A}km/h"
        
        case 0x0E:
            return f'{(A/2)-64}° before TDC'
        
        case 0x0F: # Intake air temperatue
            result = A - 40
            return f"{result}°C"
                
        case 0x10: # MAF
            B = msg.data[4]
            result = ((256*A)+B)/100
            return f"{result}g/s"
        
        case 0x11: # Throttle position
            result = (100/255)*A
            return f"{result}%"
        
        case 0x12:
            return f'{A}'
        
        case 0x14:
            if msg.data[4] == 0xFF:
                result = "sensor is not used in trim calculation"
                return f"{result}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])
        case 0x15:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])           
        case 0x16:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])
        case 0x17:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])       
        case 0x18:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])        
        case 0x19:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])       
        case 0x1A:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])            
        case 0x1B:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return Oxygen_sensor_type_01(msg.data[3], msg.data[4])
        
        case 0x1C:
            return f'{A}'
        
        case 0x1F:
            return f'{(256*A)+B}s'
        
        case 0x21:
            return f'{(256*A)+B}km'
        case 0x22:
            return f'{0.079*((256*A)+B)}kPa'
        case 0x23:
            return f'{10*((256*A)+B)}kPa'
        
        case 0x24:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x25:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x26:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x27:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x28:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x29:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x2A:
            return oxygen_sensor_type_02(A,B,C,D)
        case 0x2B:
            return oxygen_sensor_type_02(A,B,C,D)
        
        case 0x2C:
            return f'{(100/255)*A}%'
        
        case 0x2D:
            return f'{((100/128)*A)-100}%'
        
        case 0x2E:
            return f'{(100/255)*A}%'
        
        case 0x2F:
            return f'{(100/255)*A}%'
        
        case 0x30:
            return f'{A}'
        
        case 0x31:
            return f'{(256*A)+B}km'
        
        case 0x32:
            return f'{((256*A)+B)/4}Pa'
        
        case 0x33:
            return f'{A}kPa'
        
        case 0x34:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x35:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x36:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x37:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x38:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x39:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x3A:
            return oxygen_sensor_type_03(A,B,C,D)
        case 0x3B:
            return oxygen_sensor_type_03(A,B,C,D)
        
        case 0x3C:
            return catalyst_temperature(A,B)
        case 0x3D:
            return catalyst_temperature(A,B)
        case 0x3E:
            return catalyst_temperature(A,B)
        case 0x3F:
            return catalyst_temperature(A,B)
        
        case 0x41:
            return f'{A}'
        
        case 0x42:
            return f'{((256*A)+B)/1000}V'
        
        case 0x43:
            return f'{(100/255)*((256*A)+B)}%'
        
        case 0x44:
            return f'{(2/65536)*((256*A)+B)}ratio'
        
        case 0x45:
            return f'{(100/255)*A}%'
        
        case 0x46: # Ambient air temperature
            result = A - 40
            return f"{result}°C"

        case 0x47:
            return absolute_throttle_position(A)
        case 0x48:
            return absolute_throttle_position(A)
        case 0x49:
            return absolute_throttle_position(A)
        case 0x4A:
            return absolute_throttle_position(A)
        case 0x4B:
            return absolute_throttle_position(A)
        case 0x4C:
            return absolute_throttle_position(A)
          
        case _:
            return None


def decode_mode02(msg,pid):
    return f"ok {pid}"

def decode_mode03(msg,pid):
    return f"ok {pid}"

def decode_mode04(msg,pid):
    return f"ok {pid}"

def decode_mode05(msg,pid):
    A = msg[3]
    B = msg[4]
    C = msg[5]
    D = msg[6]
    #E = msg[7]
    return f"ok {pid}"

def decode_mode06(msg,pid):
    return f"ok {pid}"

def decode_mode07(msg,pid):
    return f"ok {pid}"

def decode_mode08(msg,pid):
    return f"ok {pid}"

def decode_mode09(msg,pid):
    return f"ok {pid}"

def decode_mode10(msg,pid):
    return f"ok {pid}"


# Decodifica a resposta da rede CAN
def decode_obd_response(mode,msg, pid, pid02):  
    
    match mode:
        case 1:
            return decode_mode01(msg, pid)
        case 2:
            return decode_mode02(msg, pid)
        case 3:
            return decode_mode03(msg, pid)
        case 4:
            return decode_mode04(msg, pid)
        case 5:
            return decode_mode05(msg, pid)
        case 6:
            return decode_mode06(msg, pid)
        case 7:
            return decode_mode07(msg, pid)
        case 8:
            return decode_mode08(msg, pid)
        case 9:
            return decode_mode09(msg, pid)
        case 10:
            return decode_mode10(msg, pid)
        case _:
            return f"The mode was not acknowledged. PID:{pid}{pid02} Mode: {mode} MSG: {msg} "
    
    '''
        can0  7DF   [8]  02 01 00 00 00 00 00 00
        can0  7E8   [8]  06 41 00 FF FF FF FF 00
        can0  7DF   [8]  02 02 00 00 00 00 00 00
        can0  7E8   [8]  07 42 00 00 FF FF FF FF
        can0  7DF   [8]  02 03 00 00 00 00 00 00
        can0  7E8   [8]  02 43 00 00 00 00 00 00
        can0  7DF   [8]  02 04 00 00 00 00 00 00
        can0  7E8   [8]  01 44 00 00 00 00 00 00
        can0  7DF   [8]  02 05 00 00 00 00 00 00
        can0  7DF   [8]  02 05 01 00 00 00 00 00
        can0  7DF   [8]  02 06 00 00 00 00 00 00
        can0  7E8   [8]  07 46 00 84 00 10 00 00
        can0  7DF   [8]  02 07 00 00 00 00 00 00
        can0  7E8   [8]  02 47 00 00 00 00 00 00
        can0  7DF   [8]  02 08 00 00 00 00 00 00
        can0  7E8   [8]  03 7F 08 22 00 00 00 00
        can0  7DF   [8]  02 09 00 00 00 00 00 00
        can0  7E8   [8]  06 49 00 FF FF FF FF 00
        can0  7DF   [8]  02 0A 00 00 00 00 00 00
        can0  7E8   [8]  02 4A 00 00 00 00 00 00
    '''


def main():
    try:
        bus = can.interface.Bus(
            channel=INTERFACE,
            bustype='socketcan',
            bitrade=BITRADE
        )
    except Exception as e:
        print(f"Error ao abrir a interface CAN: {e}")
        sys.exit(1)

    try:
        while True:
            for mode, mode_data in PIDS_.items():
                #print(f"Mode: {mode} Data mode: {mode_data}")
                for description, pid_values in mode_data.items():
                    pid01, pid02 = pid_values
                    send_obd_request(bus, mode, pid01, pid02)
                    # print(
                        # f"Mode: 0x{mode:02X} | Descrição: {description} | "
                        # f"PID01: 0x{pid01:02X} | PID02: 0x{pid02:02X}"
                    # )
                    start_time = time.time()
                    while time.time() - start_time < 0.2:
                        msg = bus.recv(timeout=0.05)
                        if msg and msg.arbitration_id in (0x7E8, 0x7E9, 0x7EA, 0x7EB):
                            value = decode_obd_response(mode,msg, pid01, pid02)
                            if value:
                                if pid02 == 0x00:
                                    print(f"PID: 0x{pid01:02X} | Description: {description} | Value: {value}")
                                else:
                                    print(f"PID: 0x{pid01:02X} 0x{pid02:02X} | Description: {description} | Value: {value}")
                        
                time.sleep(0.05)
            time.sleep(0.8)
            print("\n")
    except KeyboardInterrupt:
        print(f"Finalizando...")
    except Exception as e:
        print(f"Erro: {e}")
    finally:
        bus.shutdown()

if __name__ == "__main__":
   main()
