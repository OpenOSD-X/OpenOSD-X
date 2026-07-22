
<p align="right">
  <a href="./README_jp.md">日本語はこちら</a>
</p>

# OpenOSD-X

**OpenOSD-X** is a project that integrates analog OSD and VTX functionality.

## Features

* Enables the use of 5.8GHz analog video when combined with digital-only FCs (without analog OSD)
* Provides both an **SD version** (same resolution as MAX7456) and an **HD version** capable of displaying more information
* Supports font updates directly from the Betaflight Configurator
* Uses a compact and affordable **STM32G431KBT** MCU, allowing OSD to run standalone

### Development in Progress

[![Watch the video](https://img.youtube.com/vi/iuA0HPM-mJo/0.jpg)](https://youtu.be/iuA0HPM-mJo)

---

## Example Connection

![connection](doc/Connection.png)

---

## Setup Instructions

1. Connect the camera, OpenOSD-X, FC, and PC.
2. Power all devices. Connect a battery if needed.
3. Launch Betaflight Configurator.
4. Configure serial ports:

   * Set the port connected to OpenOSD-X as **VTX (MSP+DisplayPort)**.
   * After rebooting all devices, OSD and VTX parameters will be detected.
5. Configure OSD:

   * Select **HD** (not NTSC/PAL).
   * Resolution is automatically configured based on the camera and firmware.
6. Configure VTX:

   * The VTX table is built-in and automatically configured.
   * Set the channel and output power as needed.

---

## Firmware Update

You can update the firmware via FC (SerialPassthrough) or USB-Serial (FTDI).

1. Connect the OpenVTX board, FC, and PC.
2. Run the Python script (automatically detects COM port and flashes):

   ```bash
   python flashOpenOSD-X.py hex-file
   ```

   ⚠ The Betaflight serial port (DisplayPort-VTX) must be enabled.

---

## HEX Files

The latest HEX files are available here:
[https://github.com/OpenOSD-X/OpenOSD-X/actions](https://github.com/OpenOSD-X/OpenOSD-X/actions)

Releases are provided per hardware and resolution:

* `openosd-x_breakoutboard_sd.hex` …… Standard-resolution version for breakout boards
* `openosd-x_breakoutboard_hd.hex` …… High-resolution version for breakout boards

---

## Font Update

Font updates can be done directly from the Betaflight Configurator.

* Requires **Betaflight 2025.12.0 or later**
* Use Betaflight firmware built with the custom define `"USE_MSP_DISPLAYPORT_FONT"`
* Make sure your Configurator version supports this feature

---

# For Hardware Developers

## Reference Schematics and Block Diagrams

[https://github.com/OpenOSD-X/OpenOSD-X/tree/main/doc](https://github.com/OpenOSD-X/OpenOSD-X/tree/main/doc)

## Testing with Development Boards

You can test the OSD portion using ST Nucleo or WeACT development boards.
Although the target MCU is **STM32G431KBT**, it has also been confirmed to work on **STM32G431C** and **G473**.

## Initial Flashing

Use an ST-LINK to flash the following two HEX files:

* Application firmware: `openosd-x_***.hex`
* Bootloader: `openosd-x_bootloader.hex`

## VPD Table

A lookup table for VTX PA (Power Amplifier) voltage settings.
Not required if you don’t use the VTX function.
Example (for breakout board):

| 　            | 5600MHz | 5650MHz | 5700MHz | 5750MHz | 5800MHz | 5850MHz | 5900MHz | 5950MHz | 6000MHz |
| ------------ | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- |
| 14dBm(25mW)  | 1300mV  | 1330mV  | 1345mV  | 1400mV  | 1480mV  | 1590mV  | 1670mV  | 1710mV  | 1760mV  |
| 20dBm(100mW) | 1910mV  | 1970mV  | 1980mV  | 2120mV  | 2270mV  | 2430mV  | 2540mV  | 2620mV  | 2750mV  |

If you design your own board, you’ll need to create a custom VPD table to match your PA and components.
Use the dev firmware to measure and generate the correct values.

---

## Using the Dev Firmware

The development firmware is used for TX tuning and VPD table creation.

### Connection Example

```text
[PC (Terminal)] --- UART (115200bps) --- [OpenOSD-X (dev build)] --- 5.8GHz --- [Measurement Device]
```

### Example Command

```text
> vtx_set 5800 1000    ...... Frequency 5800MHz, VPD target voltage 1000mV
```

Adjust the voltage until the desired TX output is achieved, and summarize the results into a VPD table.

---

# For Firmware Developers

## Build Instructions

Build using standard **cmake** procedures.

* Install `srecord` in advance:

  ```bash
  $ sudo apt-get install -y srecord
  ```

* Build:

  ```bash
  $ mkdir -p build && cd build
  $ cmake -DTARGET=breakoutboard ..
  $ make
  ```

* Example cmake options:

  ```bash
  $ cmake -DTARGET=novtx          # build without VTX feature
  $ cmake -DRESOLUTION_HD=ON      # build HD version
  $ cmake -DINAV_FONT=ON          # embed the 512-character INAV font
  ```

## Flash Memory Map

| Address           | Description |
| ----------------- | ----------- |
| 0801F800–0801FFFF | Per-device VPD table (optional) |
| 0801B800–0801F7FF | Font data |
| 0801B000–0801B7FF | Configuration storage |
| 08004200–0801AFFF | OpenOSD-X application |
| 08004000–080041FF | OpenOSD-X application header |
| 08000000–08003FFF | Bootloader |

### Application Header

| Address           | Description |
| ----------------- | ----------- |
| 08004000–08004003 | Application size |
| 08004004–08004007 | Application CRC (STM32) |
| 08004080–0800408F | Board name (string) |
| 08004090–0800409F | Version (string) |


---

## Developer Channel

Official Discord Server:
[https://discord.gg/9qxJNTPSPs](https://discord.gg/9qxJNTPSPs)

---

## Open Source

OpenOSD-X is open-source software provided free of charge and without warranty.
Parts of the source code are based on the following projects:

* Betaflight
* OpenVTx
* OpenPixelOSD



