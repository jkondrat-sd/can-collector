import can
from dictionary_pids import *

#Functions for web CAN

# configuration of CAN Bus 
INTERFACE = 'can0'
BITRADE = 500000
EXTENDED = False
# ID of request OBD-II
REQUEST_ID = 0x7DF

def send_obd_request(bus, mode, pid01, pid02):
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

def short_term_secondary_oxygen_sensor(A,B):
    return f"bank 1: {((100/128)*A)-100}% bank 3: {((100/128)*B)-100}%"

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

# Decode the CAN response
def decode_mode01(msg,pid):
    A = msg.data[3]
    B = msg.data[4]
    C = msg.data[5]
    D = msg.data[6]
    E = msg.data[7]

    try:
        F = msg.data[8]
    except:
        F = None
        pass

    match pid:
        case 0x00:
            return f"{A}"
        
        case 0x01:
            return
        
        case 0x02:
            return
        
        case 0x03:
            match A:
                case 0:
                    return f"The motor is off"
                case 1:
                    return f"Open loop due to insufficient engine temperature"
                case 2:
                    return f"Closed loop, using oxygen sensor feedback to determine fuel mix"
                case 4:
                    return f"Open loop due to engine load OR fuel cut due to deceleration"
                case 8:
                    return f"Open loop due to system failure"
                case 16:
                    return f"Closed loop, using at least one oxygen sensor but there is a fault in the feedback system"
                case _:
                    return f"not found"
            
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
            match A:
                case 1: 
                    return f"Upstream"
                case 2: 
                    return f"Downstream of catalytic converter"
                case 4: 
                    return f"From the outside atmosphere or off"
                case 8: 
                    return f"Pump commanded on for diagnostics"

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
            match A:
                case 1:
                    return f"OBD-II as defined by the CARB"
                case 2:
                    return f"OBD as defined by the EPA"
                case 3:
                    return f"OBD and OBD-II"
                case 4:
                    return  f"OBD-I"
                case 5:
                    return f"Not OBD compliant"
                case 6:
                    return f"EOBD (Europe)"
                case 7:
                    return f"EOBD and OBD-II"
                case 8:
                    "EOBD and OBD"
                case 9:
                    return f"EOBD, OBD and OBD II"
                case 10:
                    return f"JOBD (Japan)"
                case 11:
                    return f"JOBD and OBD II"
                case 12:
                    return f"JOBD and EOBD"
                case 13:
                    return f"JOBD, EOBD, and OBD II"
                case 14:
                    return f"OBD, EOBD, and KOBD"
                case 15:
                    return f"OBD, OBD II, EOBD, and KOBD"
                case 16:
                    return f"Reserved"
                case 17:
                    return f"Engine Manufacturer Diagnostics (EMD)"
                case 18:
                    return f"Engine Manufacturer Diagnostics Enhanced (EMD+)"
                case 19:
                    return f"Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)"
                case 20:
                    return f"Heavy Duty On-Board Diagnostics (HD OBD)"
                case 21:
                    return f"World Wide Harmonized OBD (WWH OBD)"
                case 22:
                    return f"Reserved"
                case 23:
                    return f"Heavy Duty Euro OBD Stage I without NOx control (HD EOBD-I)"
                case 24:
                    return f"Heavy Duty Euro OBD Stage I with NOx control (HD EOBD-I N)"
                case 25:
                    return f"Heavy Duty Euro OBD Stage II without NOx control (HD EOBD-II)"
                case 26:
                    return f"Heavy Duty Euro OBD Stage II with NOx control (HD EOBD-II N)"
                case 27:
                    return f"Heavy Duty ZEV"
                case 28:
                    return f"Brazil OBD Phase 1 (OBDBr-1)"
                case 29:
                    return f"Brazil OBD Phase 2 (OBDBr-2)"
                case 30:
                    return f"Korean OBD (KOBD)"
                case 31:
                    return f"India OBD I (IOBD I)"
                case 32:
                    return f"India OBD II (IOBD II)"
                case 33:
                    return f"Heavy Duty Euro OBD Stage VI (HD EOBD-IV)"
                case 34:
                    return f"OBD, OBD-II, and HD OBD"
                case 35:
                    return f"Brazil OBD Phase 3 (OBDBr-3)"

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

        case 0x4D:
            return f"{(256*A)+B}min"
        
        case 0x4E:
            return f"{(256*A)+B}min"
        
        case 0x4F:
            return f"{A}ratio, {B}V, {C}mA, {D*10}kPa"
        
        case 0x50:
            return f"{(A*10)}g/s"

        case 0x51:
            match A:
                case 1:
                    return f"Gasoline"
                case 2:
                    return f"Methanol"
                case 3:
                    return f"Ethanol"
                case 4:
                    return f"Diesel"
                case 5:
                    return f"LPG"
                case 6:
                    return f"CNG"
                case 7:
                    return f"Propane"
                case 8:
                    return f"Electric"
                case 9:
                    return f"Bifuel running Gasoline"
                case 10:
                    return f"Bifuel running Methanol"
                case 11:
                    return f"Bifuel running Ethanol"
                case 12:
                    return f"Bifuel running LPG"
                case 13:
                    return f"Bifuel running CNG"
                case 14:
                    return f"Bifuel running Propane"
                case 15:
                    return f"Bifuel running Electricity"
                case 16:
                    return f"Bifuel running electric and combustion engine"
                case 17:
                    return f"Hybrid gasoline"
                case 18:
                    return f"Hybrid Ethanol"
                case 19:
                    return f"Hybrid Diesel"
                case 20:
                    return f"Hybrid Electric"
                case 21:
                    return f"Hybrid running electric and combustion engine"
                case 22:
                    return f"Hybrid Regenerative"
                case 23:
                    return f"Bifuel running diesel"
                case _:
                    return f"Not available"
        
        case 0x52:
            return f"{(100/255)*A}%"
        
        case 0x53:
            return f"{((256*A)+B)/200}kPa"
        case 0x54:
            return f"{(256*A)+B}Pa"
        
        case 0x55:
            return short_term_secondary_oxygen_sensor(A,B)
        case 0x56:
            return short_term_secondary_oxygen_sensor(A,B)
        case 0x57:
            return short_term_secondary_oxygen_sensor(A,B)
        case 0x58:
            return short_term_secondary_oxygen_sensor(A,B)
        
        case 0x59:
            return f"{10*((256*A)+B)}kPa"
        
        case 0x5A:
            return f"{(100/255)*A}%"
        
        case 0x5B:
            return f"{(100/255)*A}%"
        
        case 0x5C:
            return f"{A-40}°C"
        
        case 0x5D:
            return f"{(((256*A)+B)/128)-210}°"
        
        case 0x5E:
            return f"{((256*A)+B)/20}"
        
        case 0x5F:
            return f"{A}"
        
        case 0x60:
            return f"{A}"
        
        case 0x61:
            return f"{A-125}%"
        
        case 0x62:
            return f"{A-125}%"
        
        case 0x63:
            return f"{(256*A)+B}N.m"
        
        case 0x64:
            return f"{A-125} Idle, {B-125} Engine point 1, {C-125} Engine point 2, {D-125}Engine point 3, {E-125} Engine point 4"
        
        case 0x65:
            return f"{A}"
        
        case 0x66:
            return f"Sensor A: {((256*B)+C)/32}g/s, Sensor B: {((256*D)+E)/32}g/s"
        
        case 0x67:
            return f"Sensor1:{B-40}°C, Sensor 2:{C-40}°C"
        
        case 0x68:
            return f"Sensor1:{B-40}°C, Sensor 2:{C-40}°C"
        
        case 0x69:
            return f"{A}"
        case 0x6A:
            return f"{A}"
        case 0x6B:
            return f"{A}"
        case 0x6C:
            return f"{A}"
        case 0x6D:
            return f"{A}"
        case 0x6E:
            return f"{A}"
        case 0x6F:
            return f"{A}"
        
        case 0x70:
            return f"Sensor 1: {((256*D)+E)/0.03125}kPa"
        
        case 0x71:
            return f"{A}"
        case 0x72:
            return f"{A}"
        case 0x73:
            return f"{A}"
        case 0x74:
            return f"{A}"
        case 0x75:
            return f"{A}"
        case 0x76:
            return f"{A}"
        case 0x77:
            return f"{A}"
        case 0x78:
            return f"{A}"
        case 0x79:
            return f"{A}"
        case 0x7A:
            return f"{A}"
        case 0x7B:
            return f"{A}"
        
        case 0x7C:
            return f"{(((256*A)+B)/10)-40}°C"
        
        case 0x7D:
            return f"{A}"
        case 0x7E:
            return f"{A}"
        
        case 0x7F:
            return f"{(B*(2^24))+(C*(2^16))+(D*(2^8))+(E)}s"
        
        case 0x80:
            return f"{A}"
        case 0x81:
            return f"{A}"
        case 0x82:
            return f"{A}"
        case 0x83:
            return f"{A}"
        case 0x84:
            return f"{A}"
        
        case 0x85:
            if(F):
                return f"{(100/255)*F}%"
            else:
                return F

        case 0x86:
            return f"{A}"
        case 0x87:
            return f"{A}"
        case 0x88:
            return f"{A}"
        case 0x89:
            return f"{A}"
        case 0x8A:
            return f"{A}"
        case 0x8B:
            return f"{A}"
        case 0x8C:
            return f"{A}"

        case 0x8D:
            return f"{A}%"

        case 0x8E:
            return f"{A-125}%"        
        
        case 0x8F:
            return f"{A}"
        
        case 0x90:
            return f"{A}h"
        case 0x91:
            return f"{A}h"
        
        case 0x92:
            return f"{A}"
        
        case 0x93:
            return f"{A}h"
        
        case 0x94:
            return f"{A}"
        
        case 0x98:
            return f"{A}"
        
        case 0x99:
            return f"{A}"
        
        case 0x9A:
            return f"{A}"
        
        case 0x9B:
            return f"{(100/255)*D}%"
        
        case 0x9C:
            return f"{A}"
        
        case 0x9D:
            return f"{A}g/s"
        
        case 0x9E:
            return f"{A}kg/h"
        
        case 0x9F:
            return f"{A}"
        
        case 0xA1:
            return f"{A}ppm"

        case 0xA2:
            return f"{((256*A)+B)/32}mg/stroke"

        case 0xA3:
            return f"{A}Pa"

        case 0xA4:
            return f"{((256*C)+D)/1000}ratio"
        
        case 0xA5:
            return f"{B/2}%"
        
        case 0xA6:
            return f"{((A*(2^24))+(B*(2^16))+(C*(2^8))+D)/10}km"
        
        case 0xA7:
            return f"{A}"
        
        case 0xA8:
            return f"{A}"
        
        case 0xA9:
            if(A == 0):
                A = f"Supported"
            else:
                A = f"Unsupported"
            
            if(B == 1):
                B = f"Yes"
            else:
                B = f"No"

            return f"[A0]={A}, [B0]={B}"
        
        case 0xC5:
            return f"{A}kPa"
        
        case 0xC7:
            return f"{A}km"
        
        case 0xC8:
            return f"{A}Bit"
        
        case _:
            return None

def decode_mode02(msg,pid):
    return decode_mode01(msg,pid)

def decode_mode03(msg,pid):
    return f"{msg}"

def decode_mode04(msg,pid):
    return f"{msg}"

def decode_mode05(msg,pid,pid02):
    A = msg[3]
    B = msg[4]
    C = msg[5]
    D = msg[6]
    E = msg[7]

    if(pid == 0x01 and pid02 == 0x00):
        return f"{A}"

    return f"{A}V"

def decode_mode09(msg,pid):
    A = msg.data[0]
    B = msg.data[1]
    C = msg.data[2]
    D = msg.data[3]
    E = msg.data[4]
    F = msg.data[5]
    G = msg.data[6]
    H = msg.data[7]

    match pid:
        case 0x0A: # ECU name
            return f"{chr(F)}{chr(G)}{chr(H)}"
        case _:
            return None

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
            return decode_mode05(msg, pid,pid02)
        case 9:
            return decode_mode09(msg, pid)
        case _:
            return f"The mode was not acknowledged. PID:{pid}{pid02} Mode: {mode} MSG: {msg} "
