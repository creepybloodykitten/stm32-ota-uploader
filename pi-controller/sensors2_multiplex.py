import serial
import RPi.GPIO as GPIO
import sys
import time

# --- Pin Configuration ---
# GPIO pins connected to A, B, C of BOTH multiplexers
SELECT_PINS = [17, 27, 22]  # GPIO17 -> A, GPIO27 -> B, GPIO22 -> C

# --- Channel Selection Function ---
def select_device(device_id):
    """Selects a device from 0 to 7 for communication."""
    if not 0 <= device_id <= 7:
        raise ValueError("Device ID must be between 0 and 7")
    
    print(f"--- Switching to device {device_id} ---")
    
    # Set the address on the A, B, C pins using bitwise operations
    GPIO.output(SELECT_PINS[0], (device_id >> 0) & 1)  # LSB for A
    GPIO.output(SELECT_PINS[1], (device_id >> 1) & 1)  # Middle bit for B
    GPIO.output(SELECT_PINS[2], (device_id >> 2) & 1)  # MSB for C
    
    time.sleep(0.05) # Allow time for the multiplexer channel to stabilize


# --- Main Program Block ---
ser = None
try:
    
    # --- Command Line Argument Check ---
    if len(sys.argv) < 2:
        print("Error: Device ID not specified.", file=sys.stderr)
        print("Usage: python your_script_name.py <device_id_from_0_to_7>", file=sys.stderr)
        sys.exit(1)

    try:
        device_to_contact = int(sys.argv[1])
        if not 0 <= device_to_contact <= 7:
            raise ValueError("Device ID must be in the range of 0 to 7.")
    except ValueError as e:
        print(f"Error: Invalid device ID. {e}", file=sys.stderr)
        sys.exit(1)

    # --- Hardware Initialization ---
    GPIO.setwarnings(False) 
    GPIO.cleanup()
    GPIO.setmode(GPIO.BCM)
    for pin in SELECT_PINS:
        GPIO.setup(pin, GPIO.OUT)
    ser = serial.Serial('/dev/ttyAMA0', baudrate=115200, timeout=1)
    
    # --- Select the target device (happens once before the loop) ---
    select_device(device_to_contact)
    print(f"Switched to device {device_to_contact}. Ready to communicate.")

    # Clear port buffers to start fresh
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    # --- Optional: Send an initial command to start data stream ---
    # You can comment this out if the device sends data automatically
    initial_command = b'START_STREAM\n'
    print(f"Sending initial command: {initial_command.decode().strip()}")
    ser.write(initial_command)
    # --- Infinite Reading Loop ---
    while True:
        # Check if there's any data waiting in the serial buffer
        if ser.in_waiting > 0:
            # Read one line of data
            response_bytes = ser.readline()
            
            # Decode bytes to string and remove leading/trailing whitespace
            response_str = response_bytes.decode('utf-8', errors='ignore').strip()
            
            # Print the received data
            # print(f"Data from device {device_to_contact}: {response_str}")
            print(response_str)
            sys.stdout.flush() # Ensure the output is displayed immediately

except KeyboardInterrupt:
    print("\nProgram interrupted by user.", file=sys.stderr)
except Exception as e:
    print(f"An error occurred: {e}", file=sys.stderr)
finally:
    # Important! Clean up resources properly on exit
    if ser and ser.is_open:
        ser.close()
        print("Serial port closed.")
    GPIO.cleanup()
    print("GPIO cleanup complete. Exiting.")
