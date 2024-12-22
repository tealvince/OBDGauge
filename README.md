# OBD Gauge
By Vince Lee (c)2024

## Description

This project is a custom OBD cluster gauge for ECUs with a k-line interface
driven by an Arduino nano and intended
to be installed in a 2" diameter circular pod.

This project is a work in progress.

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

## Supported gauges (support is vehicle-dependent)

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
* See data (dump raw hex data from current gauge)
* Check readiness status
* Read codes
* Clear codes
* Burn adjustment (set multiplier to fine-tune fuel usage/efficiency stats)



