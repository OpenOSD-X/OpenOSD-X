import argparse
import logging
import sys
import time
from enum import IntEnum
from typing import List, Optional

import serial
from intelhex import IntelHex
from tqdm import tqdm

# Assuming these external custom modules exist and function as intended.
import serials_find
import SerialPassthrough

# --- Global Configuration ---

# Setup a global logger for consistent logging across the script.
logger = logging.getLogger(__name__)

# --- Constants Definition ---

class BootloaderCommand(IntEnum):
    """Encapsulates all STM32 UART Bootloader command codes for readability."""
    GET = 0x00
    GET_VERSION = 0x01
    GET_ID = 0x02
    READ_MEMORY = 0x11
    GO = 0x21
    WRITE_MEMORY = 0x31
    ERASE = 0x43
    EXTENDED_ERASE = 0x44
    WRITE_PROTECT = 0x63
    WRITE_UNPROTECT = 0x73
    READOUT_PROTECT = 0x82
    READOUT_UNPROTECT = 0x92

class BootloaderResponse(IntEnum):
    """Encapsulates ACK/NACK responses for clarity."""
    ACK = 0x79
    NACK = 0x1F

class STM32G4_CONSTANTS:
    """Groups device-specific parameters for the STM32G4 series."""
    FLASH_BASE = 0x08000000
    PAGE_SIZE = 2048  # 2KB

# --- Flashing Configuration ---
WRITE_CHUNK_SIZE = 32    # Data size for each write operation in bytes (max 256).
FIXED_BAUDRATE = 115200   # Baud rate is fixed as per user request.

# --- Main Bootloader Class ---

class STM32Bootloader:
    """
    Handles all communication with the STM32's UART bootloader.
    This class abstracts the low-level serial commands into high-level actions
    like erasing, writing, and synchronizing.
    """

    def __init__(self, port: str, baudrate: int = FIXED_BAUDRATE, timeout: int = 2):
        """
        Initializes the serial connection.
        
        :param port: The serial port name (e.g., 'COM3').
        :param baudrate: The communication speed.
        :param timeout: Read timeout in seconds.
        :raises serial.SerialException: If the port fails to open.
        """
        logger.info(f"Initializing serial port '{port}' at {baudrate} bps...")
        self.ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            parity=serial.PARITY_NONE,
            timeout=timeout
        )
        logger.info(f"Serial port '{port}' opened successfully.")

    def close(self):
        """Safely closes the serial port if it's open."""
        if self.ser and self.ser.is_open:
            logger.info("Closing serial port...")
            self.ser.close()
            logger.info("Serial port closed.")

    # --- Low-Level Communication Methods ---

    def _send_bytes(self, data: bytes):
        """Sends a raw byte sequence and logs it for debugging."""
        logger.debug(f"TX > {data.hex().upper()}")
        self.ser.write(data)

    def _read_bytes(self, size: int) -> bytes:
        """Reads a specified number of bytes and logs them for debugging."""
        data = self.ser.read(size)
        logger.debug(f"RX < {data.hex().upper()}")
        return data

    def _send_command(self, cmd: BootloaderCommand) -> bool:
        """
        Sends a command byte followed by its checksum (inverted byte).
        This is the standard procedure for sending commands to the bootloader.
        
        :return: True if the command was acknowledged (ACK received).
        """
        logger.debug(f"Sending command: {cmd.name} (0x{cmd.value:02X})")
        self.ser.reset_input_buffer() # Clear any stale data
        command_bytes = bytes([cmd.value, cmd.value ^ 0xFF])
        self._send_bytes(command_bytes)
        return self._wait_for_ack()

    def _wait_for_ack(self) -> bool:
        """
        Waits for a single-byte ACK response from the bootloader.
        
        :return: True for ACK, False for NACK or timeout.
        """
        response = self._read_bytes(1)
        if response == bytes([BootloaderResponse.ACK]):
            logger.debug("Received ACK.")
            return True
        elif response == bytes([BootloaderResponse.NACK]):
            logger.warning("Received NACK.")
            return False
        else:
            logger.warning(f"No ACK/NACK received. Got: {response.hex()}")
            return False

    @staticmethod
    def _checksum(data: bytes) -> int:
        """Calculates the XOR checksum for a given byte sequence."""
        checksum = 0
        for byte in data:
            checksum ^= byte
        return checksum

    # --- High-Level Bootloader Actions ---

    def sync(self, retry_count: int = 5) -> bool:
        """
        Establishes initial communication with the bootloader by sending the
        0x7F sync byte until an ACK is received.
        """
        logger.info("Synchronizing with bootloader...")
        for i in range(retry_count):
            logger.debug(f"Sync attempt... ({i + 1}/{retry_count})")
            self.ser.reset_input_buffer()
            self._send_bytes(b'\x7F')
            if self._read_bytes(1) == bytes([BootloaderResponse.ACK]):
                logger.info("-> Sync successful (ACK received).")
                return True
            time.sleep(0.1)
        logger.error("-> Failed to synchronize.")
        return False

    def get_id(self) -> Optional[int]:
        """Gets the device's Product ID (PID)."""
        logger.info("Getting device ID...")
        if not self._send_command(BootloaderCommand.GET_ID):
            logger.error("Failed to send GET_ID command.")
            return None

        try:
            # Read the number of bytes that follow
            num_bytes_resp = self._read_bytes(1)
            num_bytes = int.from_bytes(num_bytes_resp, 'big')
            logger.debug(f"Reading {num_bytes + 1} bytes of PID data.")

            # Read the PID data and wait for the final ACK
            pid_bytes = self._read_bytes(num_bytes + 1)
            if self._wait_for_ack():
                pid = int.from_bytes(pid_bytes, 'big')
                logger.info(f"-> Product ID: 0x{pid:04X}")
                return pid
        except Exception as e:
            logger.error(f"An error occurred while reading PID: {e}")

        return None

    def erase_pages(self, pages: List[int]) -> bool:
        """
        Performs an extended erase on a list of page numbers.
        The erase is done one page at a time for better feedback.
        """
        logger.info(f"Performing extended erase on {len(pages)} pages...")
        logger.debug(f"Pages to erase: {pages}")

        original_timeout = self.ser.timeout
        self.ser.timeout = 30  # Erase can take time, so extend the timeout temporarily.
        try:
            with tqdm(total=len(pages), desc="Erasing pages") as pbar:
                for page_num in pages:
                    # 1. Send the Extended Erase command
                    if not self._send_command(BootloaderCommand.EXTENDED_ERASE):
                        logger.error(f"Failed to send extended erase command for page {page_num}.")
                        return False

                    # 2. Prepare and send the payload for a single page erase.
                    # The protocol specifies sending (N-1) pages, so N=1 means we send 0x0000.
                    num_pages_to_erase_minus_one = 0
                    payload = num_pages_to_erase_minus_one.to_bytes(2, 'big') + page_num.to_bytes(2, 'big')
                    checksum = self._checksum(payload)

                    logger.debug(f"Erase payload for page {page_num}: {payload.hex().upper()}, checksum: 0x{checksum:02X}")
                    self._send_bytes(payload)
                    self._send_bytes(bytes([checksum]))

                    # 3. Wait for ACK to confirm the erase for this page
                    if not self._wait_for_ack():
                        logger.error(f"Extended erase failed for page {page_num}.")
                        return False
                    
                    pbar.update(1)
        finally:
            self.ser.timeout = original_timeout # Always restore the original timeout

        logger.info("-> Extended erase successful.")
        return True

    def write_memory(self, address: int, data: bytes) -> bool:
        """
        Writes a chunk of data (up to 256 bytes) to a specific memory address.
        """
        if not (0 < len(data) <= 256):
            raise ValueError("Data length must be between 1 and 256 bytes.")

        logger.debug(f"Writing {len(data)} bytes to address 0x{address:08X}...")
        
        # 1. Send Write Memory command
        if not self._send_command(BootloaderCommand.WRITE_MEMORY):
            return False

        # 2. Send the target address and its checksum
        addr_bytes = address.to_bytes(4, 'big')
        self._send_bytes(addr_bytes)
        self._send_bytes(bytes([self._checksum(addr_bytes)]))
        if not self._wait_for_ack():
            logger.warning("Failed to get ACK after sending address.")
            return False

        # 3. Send the data payload (N-1 bytes followed by data) and its checksum
        num_bytes = len(data) - 1
        payload = bytes([num_bytes]) + data
        self._send_bytes(payload)
        self._send_bytes(bytes([self._checksum(payload)]))
        if not self._wait_for_ack():
            logger.warning("Failed to get ACK after sending data payload.")
            return False
        
        return True

    def go(self, address: int):
        """
        Commands the bootloader to jump to a specific address, usually the
        start of the newly flashed application.
        """
        logger.info(f"Jumping to application at 0x{address:08X}...")
        if self._send_command(BootloaderCommand.GO):
            addr_bytes = address.to_bytes(4, 'big')
            self._send_bytes(addr_bytes)
            self._send_bytes(bytes([self._checksum(addr_bytes)]))
            # The 'Go' command might not send an ACK as it reboots the MCU.
            self._wait_for_ack()
            logger.info("-> 'Go' command sent.")

# --- Script Execution ---

def setup_logging(debug_enabled: bool):
    """
    Configures the global logger.
    - INFO level for normal operation.
    - DEBUG level for verbose, byte-level output.
    """
    level = logging.DEBUG if debug_enabled else logging.INFO
    log_format = "[%(levelname)s] %(message)s"
    
    # Clear any existing handlers to prevent duplicate log messages in some environments
    for handler in logging.root.handlers[:]:
        logging.root.removeHandler(handler)
        
    logging.basicConfig(level=level, format=log_format, stream=sys.stdout)


def main():
    """Main execution function."""
    # --- Argument Parsing ---
    parser = argparse.ArgumentParser(
        description="A Python script to flash STM32 devices via UART bootloader.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("hexfile", help="Path to the Intel HEX file to be flashed.")
    parser.add_argument("-p", "--port", help="Serial port to use (e.g. COM19). Auto-detected if omitted.")
    parser.add_argument("-d", "--debug", action="store_true", help="Enable verbose debug logging.")
    args = parser.parse_args()

    setup_logging(args.debug)

    # --- Step 1: Load HEX File ---
    logger.info("Step 1: Loading HEX file...")
    try:
        ih = IntelHex(args.hexfile)
        start_addr = ih.minaddr()
        end_addr = ih.maxaddr()
        # Sanity check: ensure the firmware is intended for the flash memory region.
        if start_addr < STM32G4_CONSTANTS.FLASH_BASE:
            logger.error(f"HEX file contains data below flash start address (0x{STM32G4_CONSTANTS.FLASH_BASE:08X}).")
            sys.exit(1)
        logger.info(f"Loaded HEX file: '{args.hexfile}'")
        logger.info(f"Address range: 0x{start_addr:08X} - 0x{end_addr:08X}")
    except Exception as e:
        logger.error(f"Failed to load or parse HEX file: {e}")
        sys.exit(1)
    
    # --- Step 2: Enable Betaflight Serial Passthrough (Custom hardware-specific step) ---
    port = args.port if args.port else serials_find.get_serial_port()
    try:
        SerialPassthrough.serialpassthrough_start(port)
        SerialPassthrough.bootloader_start(port)
        time.sleep(2) # Wait for device to enter bootloader mode
    except SerialPassthrough.PassthroughEnabled as err:
        logger.error(f"Error during passthrough setup: {err}")
        sys.exit(1)

    bl = None
    try:
        # --- Step 3: Initialize Bootloader ---
        logger.info("Step 2: Initializing bootloader interface...")
        bl = STM32Bootloader(port) # Baud rate is fixed

        # --- Step 4: Synchronize ---
        logger.info("Step 3: Synchronizing with bootloader...")
        if not bl.sync():
            sys.exit(1) # Exit if we can't communicate

        # --- Step 5: Get Device Info ---
        logger.info("Step 4: Getting device information...")
        if bl.get_id() is None:
            logger.error("Could not communicate with the device. Check connections and bootloader mode.")
            sys.exit(1)

        # --- Step 6: Calculate Pages to Erase ---
        logger.info("Step 5: Calculating pages to erase...")
        start_page = (start_addr - STM32G4_CONSTANTS.FLASH_BASE) // STM32G4_CONSTANTS.PAGE_SIZE
        end_page = (end_addr - STM32G4_CONSTANTS.FLASH_BASE) // STM32G4_CONSTANTS.PAGE_SIZE
        pages_to_erase = list(range(start_page, end_page + 1))
        logger.info(f"Calculated pages to erase: {start_page} to {end_page} ({len(pages_to_erase)} pages)")

        # --- Step 7: Erase Pages ---
        logger.info("Step 6: Erasing flash pages...")
        if not bl.erase_pages(pages_to_erase):
            sys.exit(1)

        # --- Step 8: Write Firmware ---
        logger.info("Step 7: Writing firmware...")
        with tqdm(total=len(ih), unit='B', unit_scale=True, desc="Writing firmware") as pbar:
            # Iterate through contiguous segments in the HEX file
            for start, end in ih.segments():
                address = start
                data_segment = ih.gets(start, end - start)
                
                # Write the segment in smaller chunks
                for i in range(0, len(data_segment), WRITE_CHUNK_SIZE):
                    chunk = data_segment[i:i + WRITE_CHUNK_SIZE]
                    if not bl.write_memory(address, chunk):
                        logger.error(f"\nFailed to write at address 0x{address:08X}")
                        sys.exit(1)
                    address += len(chunk)
                    pbar.update(len(chunk))
        logger.info("-> Write successful.")

        # --- Step 9: Start Application (Optional) ---
        # This step is commented out by default as it might not always be desired.
        # logger.info("Step 8: Jumping to application...")
        # bl.go(start_addr)

    except serial.SerialException as e:
        logger.error(f"Serial port error: {e}")
        sys.exit(1)
    except Exception as e:
        logger.error(f"An unexpected error occurred: {e}")
        sys.exit(1)
    finally:
        # --- Cleanup ---
        # Ensure the serial port is always closed, even if errors occur.
        if bl:
            bl.close()
        logger.info("Process finished.")


if __name__ == "__main__":
    # This block ensures the code runs only when the script is executed directly.
    main()
