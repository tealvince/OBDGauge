# vOBD: The K-line Super Gauge
By Vince Lee (c)2024-2025

## Description

This project is a custom OBD cluster gauge and experimentation platform for ECUs with a k-line interface
driven by an Arduino nano and intended
to be installed in a 2" diameter circular pod.

![prototype](https://github.com/tealvince/OBDGauge/blob/main/gauge.jpg?raw=true)

## Feature overview

* 19 displayable parameters, including instaneous and cumulative stats
* Metric/Imperial units
* Ability to hide/show individual gauges
* Built-in readiness monitor reader
* Built-in engine code reader
* Built-in menu to clear engine codes
* Adjustable brightness
* Iso-9140, Kwp-2000 (slow) and Kwp-2009 (fast) interfaces
* Numeric display with color led ring
* Persisting of settings with auto-save on power-off
* Auto-detect and cycling of protocols on startup or disconnect
* Custom circuit board layout

## Supported gauges (functionality is vehicle-dependent)

* Battery voltage level
* Accumulated runtime
* Accumulated distance
* Average fuel efficiency
* Average speed
* Speed
* Tachometer
* Coolant temp
* Air intake pressure
* Air intake flow
* Throttle percentage
* Engine load
* Timing advance
* Fuel tank level
* Fuel consumption rate
* Air/Fuel equivalence ratio
* O2 sensor voltage

## General usage

* Press button 2 to advance to the next gauge.
* Press button 1 to see the previous gauge.
* Long press button 1 to change brightness
* Long press button 2 to open the settings menu.
* Press button 2 to advance to the next item in settings menu.
* Press button 1 to backup to previous item in settings menu.
* Long press button 2 to select an item in the settings menu.

## Settings menu items

* Back (return to main display)
* Erase history (clear cumulative/average gauges)
* Display brightness
* Unit toggle (metric/imperial)
* Hide current gauge
* Show gauge (select from list)
* Auto hide/show gauges based on supported PIDs
* See data (dump raw hex data from current gauge)
* Check readiness status
* Read codes
* Clear codes
* Burn adjustment (set multiplier to fine-tune fuel usage/efficiency stats)
* Toggle Demo mode (simulate ECU reponses)
* Toggle Debug mode (show received bytes)
* Enter Sniff mode (listen to K+ line and show received data; use with Y cable and other device)

## Hardware

![circuit board](https://github.com/tealvince/OBDGauge/blob/main/circuit-board.png?raw=true)

* Custom circuit board
* Arduino nano
* Neopixel 16-LED ring light
* TM1637 4-Digit LED numeric display
* LM2903 Dual comparator IC
* 2N3906 Transistor
* DROK Mini voltage regulator (set to 9V)

## Circuit

![schematic](https://github.com/tealvince/OBDGauge/blob/main/schematic.png?raw=true)

To read to and write from the K-Line, a dual comparator chip does the work of converting 
between 5V and 12V logic levels, using voltage dividers to yield 6V and 2.5V reference
voltages.  The output of the "write" comparator goes to a PNP transistor to pull the k-line 
low when sending data back to the PCM, safely sinking more current than the comparator 
can on its own.

To power the arduino, the 12V OBD output is fed to a 9V voltage regulator, which in turn 
feeds in built-in regulator on the Arduino nano.  the extra regulator adds a layer of
safety against power fluctions over 12V.  A large capacitor briefly keeps the unit
running long enough to save state to persistent memory on power-off.

## Software

The gauge software is written in C++ with minimal libraries to drive the LED displays.
Serial communication is done with raw bit banging, implementing serial port communication 
in software timed off the arduino microsecond timer.

## Testing

Software can be tested in simulation (no ECU or ECU interface) in the following WOKWI
link.  Recommend enabling Demo Mode from the settings menu:

https://wokwi.com/projects/430523461712229377

## Case

The case and internal spacers and supports are 3D printed.  Stl files are included, as 
well as an svg for the front glass with laser cut holes for the buttons.


