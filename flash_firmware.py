import RPi.GPIO as GPIO
import time
import subprocess


BOOT0_PIN = 26  # GPIO23 (physical pin 16)
RESET_PIN = 16  # GPIO24 (physical pin 18)
FIRMWARE_PATH = "/home/avopadla/ota_upg/files/rccc.hex" 
SERIAL_PORT = "/dev/ttyAMA0" # UART port


"""
you can upload firmware by using uart and bootloader
for boot mode you should make BOOT0 pin high and reset low for a moment
after upload firmware u need to make BOOT0 low and keep reset pin high
"""


def setup_gpio():
    GPIO.setwarnings(False) 
    GPIO.setmode(GPIO.BCM)  # Use BCM numbering scheme (the GPIO numbers)
    GPIO.setup(BOOT0_PIN, GPIO.OUT)
    GPIO.setup(RESET_PIN, GPIO.OUT)
    print("GPIO pins have been configured.")

def enter_bootloader_mode():
    print("Entering System Bootloader mode...")
    # Set BOOT0 to HIGH, keep RESET HIGH initially
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
    """Calls the stm32flash utility to flash the firmware."""
    print(f"Flashing firmware from {FIRMWARE_PATH} via {SERIAL_PORT}...")
    
    command = [
        "stm32flash",
        "-w", FIRMWARE_PATH, 
        "-v",               
        SERIAL_PORT
    ]
    
    try:
        # Run the command. check=True will automatically raise an exception if the command fails.
        # capture_output=True and text=True help to get the output from the command.
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        print(">>> Flashing completed successfully! <<<")
        # print(result.stdout)
        return True
    except FileNotFoundError:
        print("!!! ERROR: 'stm32flash' command not found.")
        print("!!! Please install it using: sudo apt-get install stm32flash")
        return False
    except subprocess.CalledProcessError as e:
        print("!!! ERROR: Flashing failed. 'stm32flash' returned an error. !!!")
        print("\n--- stm32flash output ---")
        print(e.stdout) # Print the standard output from the utility
        print(e.stderr) # Print the standard error output from the utility
        print("-------------------------\n")
        return False


if __name__ == "__main__":
    try:
        setup_gpio()
        enter_bootloader_mode()
        if flash_firmware():
            reboot_to_normal_mode()
            
    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")
    finally:
        GPIO.cleanup()
        print("GPIO cleanup finished. Script has ended.")