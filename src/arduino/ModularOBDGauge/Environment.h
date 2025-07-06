#define BUILD_VERSION "1.5"

#define WOKWI false

#define DEBUG_DEFAULT_VALUE false // Whether app launches in debug mode
#define DEMO_DEFAULT_VALUE WOKWI  // Whether app launches in demo mode

#define DIGITS_TYPE_CLOCK WOKWI   // A clock-type digits display with a single colon instead of 4 decimal points

#define BOARD_REV1 true           // Older board with different battery resistor values
#define BOARD_REV2 false

// TODO: Revisit to make these dynamic
#define SERIAL_MAX_BYTES 13
#define SERIAL_MAX_FLIPS (SERIAL_MAX_BYTES*10)
