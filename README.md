# Wired Wanderer: Light-Activated Dancing LEDs with Dynamic Bicolor Patterns

## Project Description
This Arduino sketch controls a series of single-color LEDs and two bicolor LEDs. It functions as a responsive, ambient lighting system that automatically activates and displays various exciting and dramatic light patterns when ambient light levels drop (i.e., when it gets dark). Each pattern cycles automatically after a set duration.

## Features
* **Light-Activated:** Utilizes an LDR (Light Dependent Resistor) to detect ambient light.
* **Automatic Pattern Cycling:** Seamlessly transitions through a diverse set of 26 unique LED patterns.
* **12 Single-Color LEDs:** Creates various chasing, blinking, and wave effects.
* **2 Bicolor LEDs:** Integrates dynamic red/green/orange effects, often synchronized with the main LED patterns.
* **Memory Optimized:** Uses `PROGMEM` to store pattern names, saving valuable SRAM.
* **Debouncing Logic:** Ensures stable light detection, preventing rapid on/off flickering.

## Hardware Requirements
* Arduino Uno (or compatible board)
* 12 x Single-Color LEDs
* 2 x Bicolor LEDs (Common Cathode assumed for the code)
* 1 x LDR (Light Dependent Resistor)
* Resistors for LEDs (e.g., 220 Ohm for 5V)
* 1 x Resistor for LDR (e.g., 10k Ohm for voltage divider)
* Breadboard and Jumper Wires

## Wiring Diagram (Conceptual)
* **Single-Color LEDs:** Connect positive (long leg) to Arduino Digital Pins 2-13 (through 220 Ohm resistors to limit current). Connect negative (short leg) to GND.
* **LDR:** One leg to 5V. Other leg to A0 AND through a 10k Ohm resistor to GND.
* **Bicolor LED 1:** Red pin (e.g., A4), Green pin (e.g., A5). Common Cathode to GND.
* **Bicolor LED 2:** Red pin (e.g., A1), Green pin (e.g., A2). Common Cathode to GND.

*Please refer to the `ledPins`, `bicolor1RedPin`, `bicolor1GreenPin`, `bicolor2RedPin`, `bicolor2GreenPin`, and `ldrPin` constants in the code for exact pin assignments.*

## Calibration
The `lightThreshold` variable (default 650) in the code may need adjustment for your specific LDR and environment.
* When `INVERT_LDR_LOGIC` is `true`:
    * In bright light, observe the LDR reading in Serial Monitor (e.g., 700-900).
    * In dark light, observe the LDR reading (e.g., 50-200).
    * Set `lightThreshold` to a value between these, closer to the dark reading (e.g., 400-600). Lights will turn ON when LDR reading is *below* this threshold.

## How It Works
The `loop()` function continuously reads the LDR. Based on the `lightThreshold` and debouncing, it determines if it's dark enough to activate the light patterns. When active, it cycles through a predefined list of `PatternType` functions, each displaying a unique animation on the LEDs and bicolors.

## License
This code is provided for educational and personal use. Please attribute 'Wired Wanderer' if used in public projects or derivatives.

---
**Wired Wanderer**
*YouTube Channel:(https://www.youtube.com/@TheWiredWander)