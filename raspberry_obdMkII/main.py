import os
import can
import sys
import time
import sys

from tb_device_http import TBHTTPDevice
from dotenv import load_dotenv

# importing functions and dictionaries for web CAN
from can_translator import *
from dictionary_pids import *

print("="*32)
print("||READ CAN OBD IN RASPBERRY PI||")
print("="*32)


def main():

    try:
        bus = can.interface.Bus(
            channel=INTERFACE,
            bustype='socketcan',
            bitrade=BITRADE
        )
    except Exception as e:
        print(f"Error opening the interface CAN: {e}")
        sys.exit(1)

    load_dotenv()
    url_server = os.getenv("URL_SERVER")
    token = os.getenv("TOKEN_DEVICE")
    connection_with_thigsboard = False

    if token and url_server:
        client = TBHTTPDevice(url_server, token)
        if(client.connect()):
            connection_with_thigsboard = True
    else:
        print("Link server and token not thickened!")
    try:
        while True:
            for mode, mode_data in PIDS_.items():
                for description, pid_values in mode_data.items():
                    pid01, pid02 = pid_values
                    send_obd_request(bus, mode, pid01, pid02)
                    start_time = time.time()
                    while time.time() - start_time < 0.2:
                        msg = bus.recv(timeout=0.05)
                        if msg and msg.arbitration_id in (0x7E8, 0x7E9, 0x7EA, 0x7EB):
                            value = decode_obd_response(mode,msg, pid01, pid02)
                            if value:
                                if pid02 == 0x00:
                                    print(f"MODE: {mode} | PID: 0x{pid01:02X} | Description: {description} | Value: {value}")
                                else:
                                    print(f"MODE: {mode} | PID: 0x{pid01:02X} 0x{pid02:02X} | Description: {description} | Value: {value}")
                                
                                if(connection_with_thigsboard):
                                    if(client.send_telemetry({description:value}, queued=False)):
                                        continue
                                    else:
                                        connection_with_thigsboard = False
                        
                time.sleep(0.05)
            time.sleep(0.8)
            print("\n")
    except KeyboardInterrupt:
        print(f"ending...")
    except Exception as e:
        print(f"Erro: {e}")
    finally:
        bus.shutdown()

if __name__ == "__main__":
   main()