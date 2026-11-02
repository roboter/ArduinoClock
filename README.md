# ⏰ Arduino Digital Clock with DS1302 & 4-Digit 7-Segment Display

[![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.wikipedia.org/wiki/C%2B%2B)
[![Hardware](https://img.shields.io/badge/Hardware-Nano%20%7C%20DS1302%20%7C%207--Segment-orange?style=for-the-badge)](https://github.com/roboter/ArduinoClock)
[![Level](https://img.shields.io/badge/Skill%20Level-Beginner%20Friendly-brightgreen?style=for-the-badge)](#)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](#)

> **Welcome to the Arduino Clock Tutorial!**  
> This beginner-friendly, step-by-step project guide will walk you through building your very own standalone digital desktop clock using an **Arduino Nano**, a **DS1302 Real-Time Clock (RTC)**, and a **4-digit 7-segment LED display**. No prior electronics experience is needed—we'll explain every wire, register, and line of code!

---

## 📺 Demonstration Video

Watch the clock in action displaying real-time hours and minutes with multiplexed digits:

https://user-images.githubusercontent.com/386485/235453814-eafb6d12-a159-49f7-aaa6-82d5a3d0413a.mp4

*(A local copy of the demo is also included in this repository as [`clock.mp4`](file:///e:/projects/ArduinoClock/clock.mp4))*

---

## 📑 Table of Contents

1. [Project Overview](#-project-overview)
2. [What You Will Learn](#-what-you-will-learn)
3. [Bill of Materials (BOM)](#-bill-of-materials-bom)
4. [How It Works (Core Concepts)](#-how-it-works-core-concepts)
   - [7-Segment Display Anatomy](#1-7-segment-display-anatomy)
   - [Multiplexing & Persistence of Vision (POV)](#2-multiplexing--persistence-of-vision-pov)
   - [Why Use an RTC (DS1302)?](#3-why-use-an-rtc-ds1302)
   - [Binary-Coded Decimal (BCD)](#4-binary-coded-decimal-bcd)
5. [Wiring & Pinout Guide](#-wiring--pinout-guide)
   - [DS1302 RTC to Arduino](#1-ds1302-rtc-wiring)
   - [4-Digit 7-Segment Display to Arduino](#2-4-digit-7-segment-display-wiring)
   - [Complete Pin Mapping Summary](#3-complete-pin-mapping-summary)
6. [Software Setup & Arduino IDE](#-software-setup--arduino-ide)
7. [Step-by-Step Build & Flash Guide](#-step-by-step-build--flash-guide)
   - [Step 1: Set the Initial Time](#step-1-set-the-initial-time-first-flash)
   - [Step 2: Lock In Normal Clock Mode](#step-2-lock-in-normal-clock-mode-second-flash)
   - [Step 3: Verification & Serial Debugging](#step-3-verification--serial-debugging)
8. [Code Deep Dive](#-code-deep-dive)
9. [Troubleshooting & FAQ](#-troubleshooting--faq)
10. [Bonus Ideas & Next Steps](#-bonus-ideas--next-steps)
11. [License](#-license)

---

## 🎯 Project Overview

Most digital clocks need two fundamental capabilities:
1. **Accurate Timekeeping:** Keeping time consistently even when powered off.
2. **Display Output:** Displaying digits clearly in real-time.

Instead of relying on Arduino's software delays (which drift and wipe out whenever power is lost), this project uses a dedicated hardware **DS1302 Real-Time Clock** backed by a coin-cell battery. The time is displayed across a **4-digit 7-segment display** driven directly by the Arduino using **digit multiplexing**.

---

## 🧠 What You Will Learn

By following this tutorial, you will master:
- How to connect and read an external **DS1302 RTC** over a 3-wire serial interface without bulky third-party drivers.
- How **7-segment displays** are structured and how to map numerals to segment bitmasks.
- How **display multiplexing** and **Persistence of Vision (POV)** let you control 32 individual LEDs using only 12 microcontroller pins.
- How **BCD (Binary-Coded Decimal)** is parsed into readable human numbers.
- How to read and write RTC chip registers in **burst mode**.

---

## 🛠️ Bill of Materials (BOM)

| Item | Qty | Description / Recommendation |
| :--- | :---: | :--- |
| **Arduino Nano** | 1 | ATmega328P microcontroller (Arduino Uno also works) |
| **DS1302 RTC Module** | 1 | Real-Time Clock board with 32.768 kHz crystal |
| **CR2032 (or CR1220)** | 1 | Backup battery for RTC (keeps time without USB power) |
| **4-Digit 7-Segment Display** | 1 | 12-pin multiplexed display (Common Anode recommended) |
| **Breadboard** | 1 | Standard 400-point or 830-point solderless breadboard |
| **Jumper Wires** | 15–20 | Male-to-Male (M-M) and Male-to-Female (M-F) wires |
| **USB Cable** | 1 | Mini-USB or Micro-USB cable to power and flash Arduino |

---

## 💡 How It Works (Core Concepts)

### 1. 7-Segment Display Anatomy

Each digit of a 7-segment display consists of 7 bar-shaped LEDs (labeled **A** through **G**) and an optional decimal point (**DP**):

```
       -- A --
     |         |
     F         B
     |         |
       -- G --
     |         |
     E         C
     |         |
       -- D --   [DP]
```

To render any digit from `0` to `9`, we turn on specific combinations of segments:
- **`0`**: Segments `A, B, C, D, E, F`
- **`1`**: Segments `B, C`
- **`8`**: Segments `A, B, C, D, E, F, G` (all segments)

In code, this is represented as an 8-bit binary mask (`B[G][F][E][D][C][B][A][DP]`):
```cpp
const int numeral[10] = {
  B01111110, // 0 -> A, B, C, D, E, F active
  B00001100, // 1 -> B, C active
  B10110110, // 2 -> A, B, D, E, G active
  B10011110, // 3 -> A, B, C, D, G active
  B11001100, // 4 -> B, C, F, G active
  B11011010, // 5 -> A, C, D, F, G active
  B11111010, // 6 -> A, C, D, E, F, G active
  B00001110, // 7 -> A, B, C active
  B11111110, // 8 -> A, B, C, D, E, F, G active
  B11011110  // 9 -> A, B, C, D, F, G active
};
```

---

### 2. Multiplexing & Persistence of Vision (POV)

If each of the 4 digits had its own dedicated pins, you would need `4 × 8 = 32` digital pins—far more than an Arduino Nano has!

Instead, 4-digit displays share the segment lines (`A` through `G` and `DP`) across all 4 digits, and provide **4 individual common digit pins** (`D1`, `D2`, `D3`, `D4`):

```
Arduino Nano
┌────────────┐   Segments (A-G, DP)    ┌────────────────────────────────────┐
│            ├────────────────────────>│ Digit 1  Digit 2  Digit 3  Digit 4 │
│            │                         └─┬─────────┬─────────┬─────────┬────┘
│            ├─────── D1 ────────────────┘         │         │         │
│            ├─────── D2 ──────────────────────────┘         │         │
│            ├─────── D3 ────────────────────────────────────┘         │
│            ├─────── D4 ──────────────────────────────────────────────┘
└────────────┘
```

**How Multiplexing Works:**
1. Activate Digit 1 (`D1 = HIGH`), write Hour tens, wait **4 milliseconds**.
2. Turn off Digit 1, activate Digit 2 (`D2 = HIGH`), write Hour units, wait **4 ms**.
3. Turn off Digit 2, activate Digit 3 (`D3 = HIGH`), write Minute tens, wait **4 ms**.
4. Turn off Digit 3, activate Digit 4 (`D4 = HIGH`), write Minute units, wait **4 ms**.
5. Repeat continuously.

Because this cycle repeats over 60 times per second (~62 Hz), the human eye perceives all 4 digits as being steadily illuminated at the same time. This optical phenomenon is called **Persistence of Vision (POV)**.

---

### 3. Why Use an RTC (DS1302)?

The Arduino internal timer (`millis()`) resets whenever power is disconnected or the board restarts. In addition, internal ceramic resonators drift by several seconds or minutes each day.

The **DS1302** is a dedicated Real-Time Clock IC with:
- A high-precision **32.768 kHz quartz crystal**.
- Low-power backup battery circuitry (consumes less than 300 nA at 2.0V).
- An internal calendar tracking seconds, minutes, hours, day, date, month, and leap years up to year 2100.

---

### 4. Binary-Coded Decimal (BCD)

The DS1302 stores time values in **BCD (Binary-Coded Decimal)** rather than standard decimal. In BCD:
- The upper 4 bits represent the **tens** digit.
- The lower 4 bits represent the **ones** digit.

For example, minute `45` is stored as byte `0x45` (`0100 0101` in binary):
- Tens = `0100` (4)
- Ones = `0101` (5)

Our code includes handy conversion macros:
```cpp
#define bcd2bin(h,l)   (((h)*10) + (l))
#define bin2bcd_h(x)   ((x)/10)
#define bin2bcd_l(x)   ((x)%10)
```

---

## 🔌 Wiring & Pinout Guide

### 1. DS1302 RTC Wiring

The DS1302 uses a simple 3-wire synchronous serial protocol (CE, I/O, SCLK):

| DS1302 Pin | Pin Function | Arduino Nano Pin | Description |
| :---: | :---: | :---: | :--- |
| **VCC** | Power Supply | **5V** | 5V DC power from Arduino |
| **GND** | Ground | **GND** | Common ground |
| **CLK** / SCLK | Serial Clock | **A2** | Clock pulses to sync data |
| **DAT** / I/O | Data Input/Output | **A1** | Bi-directional data transfer |
| **RST** / CE | Chip Enable / Reset | **A0** | High to enable communication |

---

### 2. 4-Digit 7-Segment Display Wiring

Standard 12-pin 4-digit displays feature 6 pins on top and 6 pins on bottom. Here is the physical pin identification:

```
        Pin 12   Pin 11   Pin 10   Pin 9    Pin 8    Pin 7
         [D1]     [A]      [F]     [D2]     [D3]      [B]
       ┌──────────────────────────────────────────────────┐
       │   [ 1 ]        [ 2 ]        [ 3 ]        [ 4 ]   │
       └──────────────────────────────────────────────────┘
         [E]      [D]     [DP]     [C]      [G]      [D4]
        Pin 1    Pin 2    Pin 3    Pin 4    Pin 5    Pin 6
```

---

### 3. Complete Pin Mapping Summary

| Display Pin # | Segment / Digit Name | Function | Arduino Nano Pin |
| :---: | :---: | :--- | :---: |
| **1** | **E** | Segment E (lower left) | **D3** |
| **2** | **D** | Segment D (bottom) | **D4** |
| **3** | **DP** | Decimal Point | **D5** |
| **4** | **C** | Segment C (lower right) | **D7** |
| **5** | **G** | Segment G (middle bar) | **D9** |
| **6** | **D4** | Digit 4 Control (rightmost minute digit) | **D11** |
| **7** | **B** | Segment B (upper right) | **D13** |
| **8** | **D3** | Digit 3 Control (tens minute digit) | **D12** |
| **9** | **D2** | Digit 2 Control (ones hour digit) | **D10** |
| **10** | **F** | Segment F (upper left) | **D8** |
| **11** | **A** | Segment A (top bar) | **D6** |
| **12** | **D1** | Digit 1 Control (tens hour digit) | **D2** |

> [!TIP]
> Use colored jumper wires (e.g., Red for Digits, Yellow for Segments, Blue for RTC) to make debugging much simpler on your breadboard.

---

## 💻 Software Setup & Arduino IDE

1. **Install Arduino IDE:**  
   Download and install the latest Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).

2. **Install Time Library:**  
   - In Arduino IDE, open **Tools > Manage Libraries...** (or press `Ctrl + Shift + I`).
   - Type **`Time`** in the search bar.
   - Look for **`Time` by Michael Margolis / Paul Stoffregen** and click **Install**.

3. **Open the Project:**  
   - Clone or download this repository.
   - Open [`Clock.ino`](file:///e:/projects/ArduinoClock/Clock.ino) inside the Arduino IDE.

4. **Select Board & Port:**  
   - Go to **Tools > Board > Arduino AVR Boards > Arduino Nano**.
   - Go to **Tools > Processor > ATmega328P** *(Note: If upload fails, try **ATmega328P (Old Bootloader)**)*.
   - Go to **Tools > Port** and select your active COM / serial port.

---

## 🚀 Step-by-Step Build & Flash Guide

### Step 1: Set the Initial Time (First Flash)

Because the RTC chip doesn't know what time it is when first powered on, we need to set it once.

1. Open [`Clock.ino`](file:///e:/projects/ArduinoClock/Clock.ino).
2. Locate lines 147–162 in `setup()`:
   ```cpp
   // Uncomment this line to set the initial time:
   #define SET_DATE_TIME_JUST_ONCE
   ```
3. Enter your current date and time:
   ```cpp
   seconds    = 0;
   minutes    = 45;   // Your current minute
   hours      = 14;   // Your current hour (24-hour format: 14 = 2 PM)
   dayofweek  = 5;    // 1 = Sunday, 2 = Monday ... 7 = Saturday
   dayofmonth = 25;   // Day of month
   month      = 9;    // Month (1-12)
   year       = 2026; // Year
   ```
4. Click **Upload** (`Ctrl + U`).
5. Once uploaded, the clock will start running immediately and write this timestamp to the DS1302 memory!

---

### Step 2: Lock In Normal Clock Mode (Second Flash)

If you leave `#define SET_DATE_TIME_JUST_ONCE` active, every time you press reset or unplug the Arduino, the clock will overwrite the current time with the old hardcoded timestamp!

1. Comment the line back out:
   ```cpp
   //#define SET_DATE_TIME_JUST_ONCE
   ```
2. Click **Upload** (`Ctrl + U`) again.
3. Done! Now your RTC is running autonomously, and your Arduino will always read the true live time on boot.

---

### Step 3: Verification & Serial Debugging

Open the **Serial Monitor** at **9600 baud** (**Tools > Serial Monitor** or `Ctrl + Shift + M`).

You will see real-time debug output showing the hour, minute, and second:
```text
DS1302 Real Time Clock
Time = 14:45:01, 
Time = 14:45:02, 
Time = 14:45:03, 
```

The 4-digit display will simultaneously show `14 45`.

---

## 🔬 Code Deep Dive

Let's understand the key sections of [`Clock.ino`](file:///e:/projects/ArduinoClock/Clock.ino):

### 1. Direct Bit-Banging the DS1302
The DS1302 communication is implemented directly using hardware-accurate timing:
- `_DS1302_start()` raises `CE` to begin a transaction.
- `_DS1302_togglewrite()` clocks out 8 bits to the `IO` pin using `delayMicroseconds(1)`.
- `_DS1302_toggleread()` reads bits back synchronously.
- `_DS1302_stop()` brings `CE` low to end transmission.

### 2. Burst Read Mode
Rather than issuing separate read commands for hours, minutes, and seconds, the code uses **Clock Burst Mode** (`0xBF`):
```cpp
void DS1302_clock_burst_read(uint8_t *p)
```
This transfers all 8 clock registers in a single rapid stream into the `ds1302_struct` data structure.

### 3. Display Refresh Routine
In `loop()`, each digit is rendered sequentially:
```cpp
showDigit(rtc.h24.Hour10, 0); // Tens of hours on Digit 1
showDigit(rtc.h24.Hour, 1);   // Ones of hours on Digit 2
showDigit(rtc.Minutes10, 2);  // Tens of minutes on Digit 3
showDigit(rtc.Minutes, 3);    // Ones of minutes on Digit 4
```
In `showDigit()`, the digit's common anode/cathode is activated, the 8 segment pins are set from the numeral lookup table, and a 4 millisecond delay gives sufficient brightness before advancing to the next digit.

---

## ❓ Troubleshooting & FAQ

<details>
<summary><b>1. Error: <code>avrdude: stk500_recv(): programmer is not responding</code></b></summary>
<br>
Many popular Arduino Nano clone boards use the legacy bootloader. In the Arduino IDE menu, go to:
<b>Tools > Processor > ATmega328P (Old Bootloader)</b> and re-attempt uploading.
</details>

<details>
<summary><b>2. The time resets to the initial time every time I plug it in!</b></summary>
<br>
You forgot Step 2! Comment out <code>#define SET_DATE_TIME_JUST_ONCE</code> and upload the sketch one more time. Also ensure a working CR2032/CR1220 battery is installed in your DS1302 module.
</details>

<details>
<summary><b>3. The display shows strange or jumbled segments</b></summary>
<br>
Double-check your segment wiring against the <a href="#3-complete-pin-mapping-summary">Pin Mapping Summary</a>:
<ul>
  <li>Segment A must go to Pin D6</li>
  <li>Segment B must go to Pin D13</li>
  <li>Segment C must go to Pin D7</li>
  <li>Segment D must go to Pin D4</li>
  <li>Segment E must go to Pin D3</li>
  <li>Segment F must go to Pin D8</li>
  <li>Segment G must go to Pin D9</li>
</ul>
</details>

<details>
<summary><b>4. Digits are flickering or dim</b></summary>
<br>
The delay between digits is set to <code>delay(4);</code> (4 milliseconds). If you add extra long delays (e.g. <code>delay(1000)</code>) inside <code>loop()</code>, the display will blink and stutter. Keep <code>loop()</code> non-blocking!
</details>

<details>
<summary><b>5. The Serial Monitor shows garbage characters</b></summary>
<br>
Make sure your Serial Monitor baud rate is set to <b>9600 baud</b> (matching <code>Serial.begin(9600)</code>).
</details>

---

## 🚀 Bonus Ideas & Next Steps

Ready to take your clock to the next level? Here are some fun enhancements you can build:
- 🔔 **Buzzer Alarm:** Connect a piezo buzzer to pin `A3` and sound an alarm at a preset wake-up time.
- 🎛️ **Push Buttons:** Add two tactile pushbuttons to adjust hours and minutes manually on the fly.
- 🌙 **Auto-Dimming:** Wire a photoresistor (LDR) to adjust the display brightness between day and night.
- 🌡️ **Temperature Toggle:** Add a DS18B20 or DHT11 temperature sensor to alternate every 10 seconds between showing the time (`14:45`) and room temperature (`22°C`).

---

## 📄 License

This project is licensed under the [MIT License](LICENSE). Feel free to build upon it, modify it, and share it with your fellow makers!

---

*Crafted with ❤️ for beginners and Arduino hobbyists.*
