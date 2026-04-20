#define BUILD_VERSION "1.7"

#define WOKWI false

#define DEBUG_DEFAULT_VALUE false // Whether app launches in debug mode
#define DEMO_DEFAULT_VALUE WOKWI  // Whether app launches in demo mode

#define DIGITS_TYPE_CLOCK WOKWI   // A clock-type digits display with a single colon instead of 4 decimal points

#define BOARD_REV 3

// TODO: Revisit to make these dynamic
#define SERIAL_MAX_BYTES 13
#define SERIAL_MAX_FLIPS (SERIAL_MAX_BYTES*10)

#if BOARD_REV == 1
  #define BATTERY_VOLTAGE_DIVIDE (30+10)/10 // REV 1 board
#elif BOARD_REV == 2
  #define BATTERY_VOLTAGE_DIVIDE (47+10)/10 // REV 2 board (updates 47/10 voltage divider, wires analog volt in, adds 510 pullup)
#else
  #define BATTERY_VOLTAGE_DIVIDE (47/2+10)/10 // REV 3 board (updates 23.5/10 voltage divider)
#endif
