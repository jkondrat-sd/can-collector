import can
import time
import sys

# Configurações da CAN Bus 
INTERFACE = 'can0'
BITRADE = 500000
EXTENDED = False

# Lista de pids
PIDS = {
    0x0C: "RPM",
    0x0D: "speed",
    0x10: "maf",
    0x0F: "Intake air temperatue",
    0x11: "Throttle position",
    # 0x14: "Oxygen Sensor 1",
    # 0x15: "Oxygen Sensor 2",
    # 0x16: "Oxygen Sensor 3",
    # 0x17: "Oxygen Sensor 4",
    # 0x18: "Oxygen Sensor 5",
    # 0x19: "Oxygen Sensor 6",
    # 0x1A: "Oxygen Sensor 7",
    # 0x1B: "Oxygen Sensor 8",
    0x46: "Ambient air temperature"
}
# ID de requisição OBD-II
REQUEST_ID = 0x7DF

# Ler o pid desejado
def send_obd_request(bus, pid):
    data = [
        0x02,
        0x01,
        pid,
        0x00, 0x00, 0x00, 0x00, 0x00
    ]

    msg = can.Message(
        arbitration_id=REQUEST_ID,
        data=data,
        is_extended_id=EXTENDED,
        is_rx=False
    )

    try:
        bus.send(msg)
        #print(f"-> Enviando PID 0x{pid:02X}")
    except can.CanError:
        print("Error ao enviar mensagem")

# Decodifica a resposta da rede CAN
def decode_obd_response(msg, pid):
    if len(msg.data) < 4 or msg.data[1] != 0x41:
        return None
    
    if msg.data[2] != pid:
        return None
    
    A = msg.data[3]

    match pid:
        case 0x0C: # rpm
            B = msg.data[4]
            resultado = ((256*A)+B)/4
            return f"{resultado} rpm"
    
        case 0x0D: # vehicle speed
            return f"{A}km/h"
    
        case 0x10: # MAF
            B = msg.data[4]
            resultado = ((256*A)+B)/100
            return f"{resultado}g/s"

        case 0x0F: # Intake air temperatue
            resultado = A - 40
            return f"{resultado}°C"

        case 0x46: # Ambient air temperature
            resultado = A - 40
            return f"{resultado}°C"
        case 0x11: # Throttle position
            resultado = (100/255)*A
            return f"{resultado}%"
        case 0x14:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x15:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x16:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x17:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x18:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x19:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x1A:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])
        case 0x1B:
            if msg.data[4] == 0xFF:
                resultado = "sensor is not used in trim calculation"
                return f"{resultado}"
            else:
                return OxygenSensor(msg.data[3], msg.data[4])


def OxygenSensor(A,B):
    resultado = (A/200)
    resultado02 = (((100/128)*B)-100)
    return f"{resultado}V {resultado02}%"

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
            for pid, description in PIDS.items():
                send_obd_request(bus, pid)
                time.sleep(0.05)
                start_time = time.time()
                while time.time() - start_time < 0.2:
                    msg = bus.recv(timeout=0.05)
                    if msg and msg.arbitration_id in (0x7E8, 0x7E9, 0x7EA, 0x7EB):
                        valor = decode_obd_response(msg, pid)
                        if valor:
                            print(f"PID: 0x{pid:02X} | Descrição: {description} | Valor: {valor}")
                            break
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
