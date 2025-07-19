import RPi.GPIO as GPIO
import time
import subprocess
import sys
from datetime import datetime
import serial

BOOT0_PIN = 26  # GPIO23 (physical pin 16)
RESET_PIN = 16  # GPIO24 (physical pin 18)
FIRMWARE_PATH = sys.argv[1]
SERIAL_PORT = "/dev/ttyAMA0" # UART port
LOG_FILE_PATH = "/home/avopadla/ota_upg/changelog.txt"

"""
you can upload firmware by using uart and bootloader
for boot mode you should make BOOT0 pin high and reset low for a moment
after upload firmware u need to make BOOT0 low and keep reset pin high
"""

def log_result(message):
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    log_entry = f"[{timestamp}] {message}\n"
    try:
        with open(LOG_FILE_PATH, 'a', encoding='utf-8') as f:
            f.write(log_entry)
    except Exception as e:
        print(f"!!! error while logging {LOG_FILE_PATH}: {e} !!!")


def setup_gpio():
    GPIO.setwarnings(False) 
    GPIO.setmode(GPIO.BCM)  
    GPIO.setup(BOOT0_PIN, GPIO.OUT)
    GPIO.setup(RESET_PIN, GPIO.OUT)
    print("GPIO pins have been configured.")

def enter_bootloader_mode():
    print("Entering System Bootloader mode...")
    GPIO.output(BOOT0_PIN, GPIO.HIGH)
    GPIO.output(RESET_PIN, GPIO.HIGH)
    time.sleep(0.1)

    GPIO.output(RESET_PIN, GPIO.LOW)
    time.sleep(0.1)
    GPIO.output(RESET_PIN, GPIO.HIGH)
    time.sleep(2) # Give the STM32 time to initialize
    print("STM32 should now be in bootloader mode.")

def reboot_to_normal_mode():
    print("Rebooting STM32 into normal mode...")
    GPIO.output(BOOT0_PIN, GPIO.LOW)
    GPIO.output(RESET_PIN, GPIO.HIGH)
    time.sleep(0.1)
    
    # Perform a reset pulse to restart the MCU
    GPIO.output(RESET_PIN, GPIO.LOW)
    time.sleep(0.1)
    GPIO.output(RESET_PIN, GPIO.HIGH)
    print("STM32 has been rebooted.")

def flash_firmware():
    print(f"Flashing firmware from {FIRMWARE_PATH} via {SERIAL_PORT}...")

    command = ["stm32flash", "-w", FIRMWARE_PATH, "-v", SERIAL_PORT]
    
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        print(">>> Flashing completed successfully! <<<")
        # print(result.stdout)
        return True, "successful updating firmware"
    except FileNotFoundError:
        print("!!! ERROR: 'stm32flash' command not found.")
        print("!!! Please install it using: sudo apt-get install stm32flash")
        return False,"error: download stm32flash"
    except subprocess.CalledProcessError as e:
        print("!!! ERROR: Flashing failed. 'stm32flash' returned an error. !!!")
        print("\n--- stm32flash output ---")
        print(e.stdout) # Print the standard output from the utility
        print(e.stderr) # Print the standard error output from the utility
        print("-------------------------\n")
        error_msg = f"error: 'stm32flash' output: {e.stderr.strip()}"
        return False,  error_msg


if __name__ == "__main__":
    try:
        setup_gpio()
        
        ser = serial.Serial(SERIAL_PORT, timeout=0.1)
        ser.close()
        time.sleep(0.5) 
        
        enter_bootloader_mode()
        is_success, message = flash_firmware()
        log_result(message)
        if is_success:
            reboot_to_normal_mode()
            
            
    except Exception as e:
        error_msg =f"\nAn unexpected error occurred: {e}"
        print(f"\n{error_msg}")
        log_result(error_msg)
    finally:
        GPIO.cleanup()
        print("GPIO cleanup finished. Script has ended.")
