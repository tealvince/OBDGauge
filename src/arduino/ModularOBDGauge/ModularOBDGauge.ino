#include "Environment.h"
#include "VDigits.h"
#include "VDisplayables.h"
#include "VRing.h"

//
// MODULAROBDGAUGE.INO
//

#define DIGITS_PIN_CLK      5
#define DIGITS_PIN_DIO      6
#define DIGITS_BRIGHTNESS_MAX 7
#define DIGITS_BRIGHTNESS_MIN 2

#define OBD_OUT_PIN         3
#define OBD_IN_PIN          4

#define POWER_PIN           7
#define POWER_ANALOG_PIN    A0

#define RING_PIN_CONTROL    2
#define RING_LIGHT_COUNT    16
#define RING_ROTATION_OFFSET (WOKWI ? 7 : 0)
#define RING_STATUS_COUNT   3
#define RING_BRIGHTNESS     25
#define RING_BRIGHTNESS_MAX (WOKWI ? 255 : 40)
#define RING_BRIGHTNESS_MIN 4

#define SWITCH_PIN_1        9
#define SWITCH_PIN_2        8

VDigits vdigits;
VDisplayables vdisplayables;
VRing vring;

// Forward declarations
bool ap_isControlsButton1Down();
bool ap_isControlsButton2Down();
void ap_menuItemHighlight(int current, char color, int count, char defaultColor);
void ap_menuItemShowTitle(char *title);
int  ap_getDisplayBarCount(void);
void ap_setDisplayBarColor(int index, unsigned char color);
void ap_showDisplayBar();
void ap_showDisplayFloatValue(float num, int decimals, int suffix, bool addPlus);
void ap_showDisplayStatusState(bool connecting, bool resetting, int errorCount, int connectionErrorCount, int protocolIndex);
void ap_showDisplayStatusString(char *text);
void ap_showDisplayStatusString_P(char *ptext);
void ap_setDisplayBrightness(int brightness);
void ap_smartDelay(unsigned long wait);

struct MenuControlsProvider app_menuControlsProvider = {
  ap_isControlsButton1Down,
  ap_isControlsButton2Down,
  ap_smartDelay
};

struct MenuDisplayProvider app_menuDisplayProvider = {
  ap_menuItemHighlight,
  ap_menuItemShowTitle,
};

struct DisplayablesOutputProvider app_displayablesOutputProvider = {
  ap_getDisplayBarCount,
  ap_setDisplayBarColor,
  ap_showDisplayBar,
  ap_showDisplayFloatValue,
  ap_showDisplayStatusState,
  ap_showDisplayStatusString,
  ap_showDisplayStatusString_P,
  ap_setDisplayBrightness,
};

void setup() {
 
  // Setup
  pinMode(POWER_PIN, INPUT);
  pinMode(SWITCH_PIN_1, INPUT_PULLUP);
  pinMode(SWITCH_PIN_2, INPUT_PULLUP);
  vdigits.setup(DIGITS_PIN_CLK, DIGITS_PIN_DIO);
  vring.setup(RING_PIN_CONTROL, RING_LIGHT_COUNT, RING_BRIGHTNESS, RING_ROTATION_OFFSET);

  ap_showDisplayStatusString_P(PSTR("VObd"));
  vring.showDemo();
  ap_smartDelay(800);
  ap_showDisplayStatusString_P(PSTR(" " BUILD_VERSION));
  ap_smartDelay(800);
  vring.clear();

  vdisplayables.setup(OBD_IN_PIN, OBD_OUT_PIN, POWER_ANALOG_PIN, &app_displayablesOutputProvider, &app_menuDisplayProvider, &app_menuControlsProvider);
}

void loop() {
  vdisplayables.mainLoop();
}

///////////////////////////////////////////////////////////////
// APP GLUE
///////////////////////////////////////////////////////////////

//------------------------------------------------------
// Private
//------------------------------------------------------

#define BUTTON_DEBOUNCE_COUNT 25
static bool ap_button1LastState = false;
static bool ap_button2LastState = false;

bool ap_isControlsButton1Down() {
  bool state = !digitalRead(SWITCH_PIN_1);
  if (state != ap_button1LastState) {
    for (int i=0; i<BUTTON_DEBOUNCE_COUNT; i++) {
      if (!digitalRead(SWITCH_PIN_1) != state) return ap_button1LastState;
      ap_smartDelay(1);
    }
    return ap_button1LastState = state;
  }

  // Kind of hacky, but ping when reading buttons to keep connection alive
  vdisplayables.ping();

  return ap_button1LastState;
}

bool ap_isControlsButton2Down() {
  bool state = !digitalRead(SWITCH_PIN_2);
  if (state != ap_button2LastState) {
    for (int i=0; i<BUTTON_DEBOUNCE_COUNT; i++) {
      if (!digitalRead(SWITCH_PIN_2) != state) return ap_button2LastState;
      ap_smartDelay(1);
    }
    return ap_button2LastState = state;
  }
  return ap_button2LastState;
}

void ap_menuItemHighlight(int current, char color, int count, char defaultColor) {
  // Show current entry in ring
  for (int i = 0; i < RING_LIGHT_COUNT; i++) {
    if (i == (current + RING_STATUS_COUNT) % RING_LIGHT_COUNT) {
      vring.setPixelColor(i, color);
    } else if (i >= RING_STATUS_COUNT && i < RING_STATUS_COUNT + count || (RING_STATUS_COUNT + count > RING_LIGHT_COUNT && i < (RING_STATUS_COUNT + count) % RING_LIGHT_COUNT)) {
      vring.setPixelColor(i, defaultColor);
    } else {
      vring.setPixelColor(i, 'k');
    }
  }
  vring.show();
}

void ap_menuItemShowTitle(char *title) {
  vdigits.showString(title, true);
}

int ap_getDisplayBarCount(void) {
  return RING_LIGHT_COUNT - RING_STATUS_COUNT;
}

void ap_setDisplayBarColor(int index, unsigned char color) {
  vring.setPixelColor(index + RING_STATUS_COUNT, color);
}

void ap_showDisplayBar() {
  vring.show();
}

void ap_showDisplayFloatValue(float num, int dig, int suf, bool addPlus) {
  char sufBuf[2];
  char preBuf[2];
  char *prefix = preBuf; prefix[0] = (num < 0) ? '-' : addPlus ? '+' : 0;

  char suffix = 0;
  unsigned long whole = 0;
  unsigned long frac = 0;
  int fracdig = 0;

  sufBuf[1] = 0;
  preBuf[1] = 0;
  num = abs(num);

  if (num > 999000000) {
    whole = num/1000000000;
    frac = (num-whole*1000000000)/100000000;
    fracdig = 1;
    suffix = 'B';
  } else if (num > 999000) {
    whole = num/1000000;
    frac = (num-whole*1000000)/100000;
    fracdig = 1;
    suffix = 'M';
  } else if (num > 9999) {
    whole = num/1000;
    frac = (num-whole*1000)/100;
    fracdig = 1;
    suffix = 'k';
  } else {
    whole = num;
    if (dig > 0) {
      float diff = abs(num) - floor(abs(num));
      for (int i=0; i<dig; i++) {
        diff *= 10;
      }
      frac = diff;
      fracdig = dig;
    }
  }

  char buf[30];
  char tmp[30];

  strcpy(buf, prefix);
  itoa(whole, buf+strlen(buf), 10);

  int maxDigits = 4;

  // Override default suffix if set
  sufBuf[0] = suffix ?: suf;

  if (strlen(buf) + strlen(sufBuf) + 1 <= maxDigits && ((strlen(sufBuf) && suffix) || dig)) {
    maxDigits++;
    strncat(buf, ".", sizeof(buf)-1);
    itoa(frac, tmp, 10);
    for (int i=strlen(tmp); i<fracdig; i++) {
      strncat(buf, "0", sizeof(buf)-1);
    }
    strncat(buf, tmp, sizeof(buf)-1);
  }

  // Right justify
  int space = maxDigits - strlen(buf) - strlen(sufBuf);
  space = max(0,space);
  tmp[0] = tmp[1] = tmp[2] = tmp[3] = ' ';
  tmp[4] = 0;
  strcpy(tmp+space, buf);

  strncat(tmp, sufBuf, sizeof(tmp)-1);
  vdigits.showString(tmp, true);
}

void ap_showDisplayStatusState(bool connecting, bool resetting, int errorCount, int connectionErrorCount, int protocolIndex) {
  bool powerOn = digitalRead(POWER_PIN);

  for (int i=0; i<RING_STATUS_COUNT; i++) {
    if (!powerOn) {
      vring.setPixelColor(i, 'o');
    } else if (connecting) {
      vring.setPixelColor(RING_STATUS_COUNT-i-1, (i == protocolIndex % RING_STATUS_COUNT) ? 'b' : 'I');
    } else if (resetting) {
      vring.setPixelColor(i, i <= RING_STATUS_COUNT-connectionErrorCount ? 'V' : 'p');
    } else {
      vring.setPixelColor(RING_STATUS_COUNT-i-1, i < errorCount ? 'R' : 'k');
    }
  }
  vring.show();
}

void ap_showDisplayStatusString(char *text) {
  vdigits.showString(text, true);
}

void ap_showDisplayStatusString_P(char *ptext) {
  char text[20];
  strcpy_P(text, ptext);
  ap_showDisplayStatusString(text);
}

void ap_setDisplayBrightness(int brightness) {
  vring.setBrightness(RING_BRIGHTNESS_MIN + (RING_BRIGHTNESS_MAX - RING_BRIGHTNESS_MIN) * brightness/100);
  vdigits.setBrightness(DIGITS_BRIGHTNESS_MIN + (DIGITS_BRIGHTNESS_MAX - DIGITS_BRIGHTNESS_MIN) * brightness/100);
}

static bool ap_lastPowerOn;
void ap_smartDelay(unsigned long wait) {
  unsigned long start = millis();
  while (1) {
    // Detect changes in input power
    bool powerOn = digitalRead(POWER_PIN);
    if (ap_lastPowerOn != powerOn) {
      ap_lastPowerOn = powerOn;
      if (!powerOn) {
        vdisplayables.savePersistedState();
        vdigits.showString("Save", true);
        delay(1000);
      }
    }

    unsigned long time = millis();
    if (time < start || time >= start + wait) return;

    // Don't block UI for extended periods of time
    if (time >= start + 100 && ap_isControlsButton1Down()) return;
  }
}
