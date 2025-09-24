
import serial
import sys
import time

try:
    ser = serial.Serial('/dev/ttyACM0', baudrate=115200, timeout=1)
except serial.SerialException as e:
    print(f"Error opening serial port: {e}", file=sys.stderr)
    sys.exit(1) 

try:
    while True:
        if ser.in_waiting > 0:
            data_bytes = ser.readline()
            data_str = data_bytes.decode('utf-8', errors='ignore').strip()
            print(data_str)
            sys.stdout.flush()
        

except KeyboardInterrupt:
    print("Program interrupted", file=sys.stderr)
except Exception as e:
    print(f"An error occurred: {e}", file=sys.stderr)
finally:
    ser.close()
