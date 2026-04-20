#include "Environment.h"
#include "VDisplayables.h"
#include "VSettings.h"
#include "VObd.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

#define FLASH_VERSION 5

#define FUEL_ADJUST_MIN 0.10
#define FUEL_ADJUST_MAX 10.00
#define FUEL_ADJUST_DELTA 0.01

#define GEAR_MAX_COUNT 8
#define GEAR_RPK_MIN (1000L/60)
#define GEAR_RPK_MAX (10000L/60)
#define GEAR_RPK_INTERVAL 1L
#define GEAR_DETECT_TIME (60L * 1000L)
#define GEAR_MIN_DIFF 3
#define GEAR_DIVIDEND 200

#define WEIGHT_MIN 500
#define WEIGHT_MAX 3500
#define WEIGHT_DEFAULT 1400

#define LOOP_IDLE_CYCLE_MILLIS  10000

#define PID_SPEED       0x000d
#define PID_MAF         0x0010
#define PID_BURN_VALUE  0x005e

///////////////////////////////////////////////////////////////
// VDISPLAYABLES.CPP
// Displayable OBD Gauges
///////////////////////////////////////////////////////////////

struct DisplayableItem {
  char menuColor;
  char spotColor;
  char colors[14];
  char name[6];
  char unit1[5];
  char unit2[5];
  char suffix;
  bool addPlus;
  bool centered;
  uint8_t decimals;
  uint8_t pid;

  int8_t shift;
  uint16_t mask;
  int16_t  offset1;
  uint16_t multiplier1;
  uint32_t divisor1;
  int16_t  offset2;
  uint16_t multiplier2;
  uint32_t divisor2;
  int16_t min1;
  int16_t max1;
  int16_t min2;
  int16_t max2;
};

#define DISPLAYABLE_ITEM_TOTAL_TIME         0
#define DISPLAYABLE_ITEM_TOTAL_FUEL         1
#define DISPLAYABLE_ITEM_TOTAL_DISTANCE     2
#define DISPLAYABLE_ITEM_AVERAGE_SPEED      3
#define DISPLAYABLE_ITEM_AVERAGE_EFFICIENCY 4

#define DISPLAYABLE_ITEM_GFORCE             5
#define DISPLAYABLE_ITEM_HORSEPOWER         6
#define DISPLAYABLE_ITEM_GEAR               7

#define DISPLAYABLE_ITEM_SPEED              8
#define DISPLAYABLE_ITEM_TACHOMETER         9
#define DISPLAYABLE_ITEM_COOLANT_TEMP       10
#define DISPLAYABLE_ITEM_INTAKE_AIR_PRES    11
#define DISPLAYABLE_ITEM_INTAKE_AIR_FLOW    12
#define DISPLAYABLE_ITEM_THROTTLE_PERCENT   13

#define DISPLAYABLE_ITEM_ENGINE_LOAD        14
#define DISPLAYABLE_ITEM_TIMING_ADVANCE     15
#define DISPLAYABLE_ITEM_INTAKE_TEMP        16
#define DISPLAYABLE_ITEM_FUEL_TANK_LEVEL    17
#define DISPLAYABLE_ITEM_FUEL_BURN_RATE     18
#define DISPLAYABLE_ITEM_AIR_FUEL_TRIM.     19
#define DISPLAYABLE_ITEM_AIR_FUEL_EQRATIO   20
#define DISPLAYABLE_ITEM_OXY_SENSOR_A_VOLTS 21
#define DISPLAYABLE_ITEM_OXY_SENSOR_B_VOLTS 22

#define DISPLAYABLE_ITEM_BATTERY_VOLTS      23

#define DISPLAYABLE_ITEM_COUNT 24

static const DisplayableItem menu_rawDisplayables[] PROGMEM = {
  // Accumulated
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     

  { 'r','W', "RRRRRRRRRRRRR",  "t.RUN",   "CLOC",     "",     0, false, false, 2, 0xff, 0,  0xffff,   0,    1,     1,   0,   1,     1,    0,   59,   0,   59 },
  { 'o',  0, "OOOOOOOOOOOOO",  "t.gAS",   "Litr", "gALS",     0, false, false, 2, 0xff, 0,  0xffff,   0,    1,     1,   0, 264,  1000,    0,  200,   0,   60 },
  { 'b',  0, "iiiiiiiiiiiii",  "t.DSt",   "Kilo", "MILE",     0, false, false, 1, 0xff, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  800,   0,  500 },
  { 'c','g', "bbbbbbbbbbbbb",  "Av.SP",    "KPH",  "MPH",     0, false, false, 1, 0x0d, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  200,   0,  120 },
  { 'l','b', "lllllllllllll",  "Av.EF",   "L100",  "MPg",     0, false, false, 2, 0xff, 0,  0xffff,   0,  100,     1,   0, 621,   264,    0,   40,   0,   50 },

  // Special
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'G','w', "rrrooowgggbbb",   "Forc",    "g's",     "",     0, false,  true, 2, 0x0d, 0,  0xffff,   0,    1,     1,   0,   1,     1,   -1,      1,-1,    1 },
  { 'O','w', "yyyyoooorrrrr",   "POut",   "m HP",   "HP",     0, false, false, 0, 0x0d, 0,  0xffff,   0,    1,     1,   0,   1,     1,    0,    300, 0,  300 },
  { 'w','w', "nnnnnnnnnnnnn",   "GEAr",   "NvSI", "NvUS",     0, false, false, 0, 0x0c, 0,  0xffff,   0,    1,     1,   0, 1000,  621,    1,      8, 1,    8 },

  // Bank1
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'g',  0, "GGGGGGGGGGGGG",   "SPED",    "KPH",  "MPH",   ' ', false, false, 0, 0x0d, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  200,   0,  120 },
  { 'y',  0, "yyyyyyyyyoorr",   "TACH",    "RPM",     "",     0, false, false, 0, 0x0c, 0,  0xffff,   0,    1,     4,   0,   1,     4,    0, 7000,   0, 7000 },
  { 'c',  0, "cccccgggorrrr",   "COOL",   " \1C", " \1F",  '\1', false, false, 0, 0x05, 0,  0xffff, -40,    1,     1, -22,   9,     5,   38,  150, 100,  300 },
  { 'W',  0, "WWWWWWWWWWWWW",  "A.Prs",   "K.PA",   "PSI",  ' ', false, false, 0, 0x0B, 0,  0xffff,   0,    1,     1,   0, 145,  1000,    0,  210,   0,   30 },
  { 'w',  0, "WWWWWWWWWWWWW",  "A.Flo",    "g/S", "Lb/m",     0, false, false, 1, 0x10, 0,  0xffff,   0,    1,   100,   0, 132, 100000,   0,  400,   0,   50 },
  { 'c',  0, "ccccccccccccc",   "THRO",   "pct.",     "",   '%', false, false, 0, 0x11, 0,  0xffff,   0,  100,   255,   0,   1,      1,   0,  100,   0,  100 },

  // Bank2
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'y',  0, "yyyyooooorrrr",   "ENG.",   "LOAD",     "",   '%', false, false, 0, 0x04, 0,  0xffff,   0,  100,   255,   0,   1,      1,   0,  100,   0,  100 },
  { 'n',  0, "nnnnnnwoooooo",   "TIME",  "Advc.",     "",  '\1', false, true,  1, 0x0E, 0,  0xffff,-128,    1,     2,   0,   1,      1, -45,   45, -45,   45 },
  { 'w',  0, "WWWWWWWWWWWWW",  "AirT.",   " \1C", " \1F",  '\1', false, false, 0, 0x0F, 0,  0xffff, -40,    1,     1, -22,   9,      5,  20,  100,  70,  200 },
  { 'o',  0, "ooooooooooooo",   "TANK",   "LevL",     "",   '%', false, false, 0, 0x2F, 0,  0xffff,   0,    1,     1,   0,   1,      1,   0,  100 ,  0,  100 },
  { 'R',  0, "RRRRRRRRRRRRR",   "Burn",    "L/h",  "g/h",   ' ', false, false, 1, 0x5E, 0,  0xffff,   0,    1,    20,   0, 264,  20000,   0,   40,   0,   10 },
  { 'N',  0, "RRRRRRwGGGGGG",   "Fuel",   "Trim",     "",   '%', false, true,  1, 0x06, 0,  0xffff,-128,  100,   128,-128, 100,    128, -10,   10, -10,   10 },
  { 'n',  0, "ppppppwyyyyyy",    "A/F",   "Rtio",     "",     0, false, true,  2, 0x24, 16, 0xffff,   0,    2, 65536,   0,   2,  65536,   0,    2,   0,    2 },
  { 'p',  0, "yyyyyybpppppp",   "O/2A",   "Volt",     "",   'V', false, true,  1, 0x14, 24, 0x00ff,  0,    1,   200,   0,   1,    200,   0,    1,   0,    1 },
  { 'P',  0, "yyyyyycpppppp",   "O/2b",   "Volt",     "",   'V', false, true,  1, 0x15, 24, 0x00ff,  0,    1,   200,   0,   1,    200,   0,    1,   0,    1 },

  // Sensor
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'Y',  0, "OYYYGGGgggccc",   "Batt",   "Volt",    "",    'V', false, false, 1, 0xff, 0,  0xffff,   0,    1,     1,   0,   1,      1,   10,   14,   10,  14  },
};

//------------------------------------------------------
// Private
//------------------------------------------------------

static VMenu vmenu;
static VSettings vsettings;
static VObd vobd;
static int  ds_requestErrorCount;
static int  ds_totalRequestErrorCount;
static int  ds_connectionErrorCount;
static int  ds_lastItemIndex;
static long ds_lastLoopChangeMillis;
static int  ds_powerAnalogPin;
static bool ds_connecting;
static bool ds_resetting;
static bool ds_persistedStateLoaded = false;
static bool ds_debugModeEnabled = DEBUG_DEFAULT_VALUE;
static int  ds_testProtocol = OBD_PROTOCOL_FIRST - 1;
static struct DisplayablesOutputProvider *ds_output;
static struct MenuControlsProvider *ds_controls;
static DisplayableItem ds_itemObject;

struct DisplayablePersistedState {
  int version;
  int brightness;
  int currentItemIndex;
  long itemsHiddenMask;
  long itemsUsingAltUnitsMask;
  double totalElapsedSeconds;
  double totalDrivenKilometers;
  double totalConsumedFuelLitres;
  double fuelAdjustment;
  int protocol;
  int hasAutoScannedItems;
  int loopModeEnabled;
  int demoModeEnabled;
  double sessionElapsedSeconds;
  long gears[GEAR_MAX_COUNT];
  int gearCount = 0;
  int kgWeight = 1400;
};

static struct DisplayablePersistedState ds_persistedState;

struct DisplayableItem *ds_getDisplayableObject(int index) {
  memcpy_P(&ds_itemObject, (__FlashStringHelper*)&menu_rawDisplayables[index], sizeof(ds_itemObject));  
  return &ds_itemObject;
}
struct DisplayableItem *ds_getCurrentDisplayableObject() {
  return ds_getDisplayableObject(ds_persistedState.currentItemIndex);
}

void ds_loadPersistedState() {
  for (int i=0; i<sizeof(ds_persistedState); i++) {
    ((unsigned char *)&ds_persistedState)[i] = EEPROM.read(i);
  }

  if (ds_persistedState.version != FLASH_VERSION) {
    memset(&ds_persistedState, 0xff, sizeof(ds_persistedState));
    ds_persistedState.version = FLASH_VERSION;
  }

  // Do safety checks
  if (ds_persistedState.brightness < 0 || ds_persistedState.brightness > 100) ds_persistedState.brightness = 100;
  if (ds_persistedState.currentItemIndex < 0 || ds_persistedState.currentItemIndex >= DISPLAYABLE_ITEM_COUNT) ds_persistedState.currentItemIndex = 0;
  if (ds_persistedState.itemsHiddenMask < 0) ds_persistedState.itemsHiddenMask = 0;
  if (ds_persistedState.itemsUsingAltUnitsMask < 0) ds_persistedState.itemsUsingAltUnitsMask = 0x7fffffffL;
  if (ds_persistedState.totalElapsedSeconds < 0 || isnan(ds_persistedState.totalElapsedSeconds)) ds_persistedState.totalElapsedSeconds = 0;
  if (ds_persistedState.totalDrivenKilometers < 0 || isnan(ds_persistedState.totalDrivenKilometers)) ds_persistedState.totalDrivenKilometers = 0;
  if (ds_persistedState.totalConsumedFuelLitres < 0 || isnan(ds_persistedState.totalConsumedFuelLitres)) ds_persistedState.totalConsumedFuelLitres = 0;
  if (ds_persistedState.fuelAdjustment < FUEL_ADJUST_MIN || ds_persistedState.fuelAdjustment > FUEL_ADJUST_MAX || isnan(ds_persistedState.fuelAdjustment)) ds_persistedState.fuelAdjustment = 1.0;
  if (ds_persistedState.protocol < OBD_PROTOCOL_FIRST || ds_persistedState.protocol > OBD_PROTOCOL_LAST) ds_persistedState.protocol = OBD_PROTOCOL_AUTOMATIC;
  if (ds_persistedState.hasAutoScannedItems < 0 || ds_persistedState.hasAutoScannedItems > 1) ds_persistedState.hasAutoScannedItems = 0;
  if (ds_persistedState.loopModeEnabled < 0 || ds_persistedState.loopModeEnabled > 1) ds_persistedState.loopModeEnabled = 0;
  if (ds_persistedState.demoModeEnabled < 0 || ds_persistedState.demoModeEnabled > 1) ds_persistedState.demoModeEnabled = DEMO_DEFAULT_VALUE;
  if (ds_persistedState.gearCount < 0 || ds_persistedState.gearCount > GEAR_MAX_COUNT) {
    ds_persistedState.gearCount = 5;
    ds_persistedState.gears[0] = 200 / 1.6 + 0.8;
    ds_persistedState.gears[1] = 100 / 1.6 + 0.8;
    ds_persistedState.gears[2] = 70 / 1.6 + 0.8;
    ds_persistedState.gears[3] = 50 / 1.6 + 0.8;
    ds_persistedState.gears[4] = 40 / 1.6 + 0.8;
  }
  if (ds_persistedState.kgWeight < WEIGHT_MIN || ds_persistedState.kgWeight > WEIGHT_MAX) ds_persistedState.kgWeight = WEIGHT_DEFAULT;

  ds_persistedState.sessionElapsedSeconds = ds_persistedState.totalElapsedSeconds;
  ds_persistedStateLoaded = true;
}

void ds_savePersistedState() {
  if (ds_persistedStateLoaded) {
    for (int i=0; i<sizeof(ds_persistedState); i++) {
      EEPROM.write(i, ((unsigned char *)&ds_persistedState)[i]);
    }
  }
}

//------------------------------------------------------
// Private (obd output support)
//------------------------------------------------------

void ds_showStatusString(char *text) {
  ds_output->showStatusString(text);
  ds_controls->smartDelay(500);
}

void ds_showStatusString_P(char *ptext) {
  ds_output->showStatusString_P(ptext);
  ds_controls->smartDelay(500);
}

void ds_showStatusInteger(int num) {
  char text[5];
  snprintf_P(text, sizeof(text), PSTR("%-4d"), (int)num);
  ds_output->showStatusString(text);
  ds_controls->smartDelay(1000);
}

void ds_showStatusByte(int num) {
  char text[5];
  snprintf_P(text, sizeof(text), PSTR("= %02X"), (int)num);
  ds_output->showStatusString(text);
  ds_controls->smartDelay(1000);
}

struct ObdOutputProvider ds_outputProvider = {
  ds_showStatusString,
  ds_showStatusString_P,
  ds_showStatusInteger,
  ds_showStatusByte,
};

//------------------------------------------------------
// Private (menu support)
//------------------------------------------------------

float ds_setValue(char *title, float value, float minVal, float maxVal, float interval1, float interval2, float interval3, int digits);

int ds_getDisplayable() { 
  return ds_persistedState.currentItemIndex; 
}

void ds_setDisplayable(int index) {
  ds_persistedState.currentItemIndex = index;
} 

char *ds_getDisplayableTitle1() {
  return ds_getCurrentDisplayableObject()->name;
} 

char *ds_getDisplayableTitle2() {
  if ((ds_persistedState.itemsUsingAltUnitsMask & (1L << ds_persistedState.currentItemIndex)) && ds_getCurrentDisplayableObject()->unit2[0]) {
    return ds_getCurrentDisplayableObject()->unit2;
  } else {
    return ds_getCurrentDisplayableObject()->unit1;
  }
} 

char ds_getDisplayableColor() {
  return ds_getCurrentDisplayableObject()->menuColor;
} 

bool ds_isDisplayableHidden(int index) {
  return !!(ds_persistedState.itemsHiddenMask & (1L << index));
} 

bool ds_displayableLongPressAction(int current, int button, MenuDataSource *ds) {

  if (button == 1) {
    ds_persistedState.brightness = (ds_persistedState.brightness > 80) ? 0 : ds_persistedState.brightness + 25;
    ds_output->setBrightness(ds_persistedState.brightness);
    vmenu.highlightCurrentItem();

    char text[5];
    snprintf_P(text, sizeof(text), PSTR("br %d"), (int)ds_persistedState.brightness/25+1);
    ds_output->showStatusString(text);
    ds_controls->smartDelay(500);

    while(ds_controls->isButton2Down()) {}
    return;
  }

  vsettings.showMenu();
  vmenu.showCurrentItem(false);
  return false;
} 

static struct MenuDataSource ds_menuDataSource = {
  DISPLAYABLE_ITEM_COUNT, 
  ds_getDisplayable, 
  ds_setDisplayable, 
  ds_getDisplayableTitle1, 
  ds_getDisplayableTitle2, 
  ds_getDisplayableColor, 
  ds_isDisplayableHidden, 
  ds_displayableLongPressAction,
  NULL,
  'I'
};

//------------------------------------------------------
// Private (gear support)
//------------------------------------------------------

static long gearRpkTable[(GEAR_RPK_MAX - GEAR_RPK_MIN)/GEAR_RPK_INTERVAL+1];
static long gearRpkSamplingMsRemaining = 0;

void ds_startGearDetect() {
  memset(gearRpkTable, 0, sizeof(gearRpkTable));
  gearRpkSamplingMsRemaining = GEAR_DETECT_TIME;
  ds_persistedState.currentItemIndex = ds_lastItemIndex = DISPLAYABLE_ITEM_GEAR;
  ds_output->showSweep('G');
}

int compareGears(const void *a, const void *b) {
  long diff = (*(long*)a - *(long*)b);
  return diff > 0 ? -1 : diff < 0 ? 1 : 0;
}

void ds_sortGears() {
  qsort(ds_persistedState.gears, ds_persistedState.gearCount, sizeof(ds_persistedState.gears[0]), compareGears); // Standard C sort
}

int ds_showGears(void) {
  bool useAltUnits = (ds_persistedState.itemsUsingAltUnitsMask & (1L << DISPLAYABLE_ITEM_GEAR));
  float mult = useAltUnits ? 1.0/0.6214 : 1;

  ds_output->showStatusString("got");
  ds_controls->smartDelay(700);
  ds_output->showFloatValue((float)ds_persistedState.gearCount, 0, 0, false);
  ds_controls->smartDelay(700);
  ds_output->showStatusString("gear");
  ds_controls->smartDelay(1000);

  for (int i=0; i<ds_persistedState.gearCount; i++) {
    ds_output->showFloatValue((float)ds_persistedState.gears[i] * mult + mult/2, 0, 0, false);
    ds_controls->smartDelay(1000);
  }
}

int ds_getGearCount(void) {
  return ds_persistedState.gearCount;
}

void ds_addGear(void) {
  if (ds_persistedState.gearCount < GEAR_MAX_COUNT) {
    if (ds_persistedState.gearCount > 0) {
      ds_persistedState.gears[ds_persistedState.gearCount] = ds_persistedState.gears[ds_persistedState.gearCount-1] * 0.75;
    }
    ds_persistedState.gearCount++;
    ds_sortGears();
    ds_savePersistedState();
  } else {
    ds_output->showStatusString("Err!");
    ds_controls->smartDelay(1000);
  }
}

void ds_removeGear(void) {
    if (ds_persistedState.gearCount > 0) {
      ds_persistedState.gearCount--;
    }
    ds_savePersistedState();
}

void ds_editGear(int gear) {
  if (gear < 0 || gear >= ds_persistedState.gearCount) return;

  char *title = "nvSI";
  float mult = 1;
  if (ds_persistedState.itemsUsingAltUnitsMask & (1L << DISPLAYABLE_ITEM_GEAR)) {
    title = "nvUS";
    mult = 1.0/0.6214;
  }

  ds_persistedState.gears[gear] = ds_setValue(title, ds_persistedState.gears[gear] * mult + mult/2, 0, 100000, 1, 10, 100, 0) / mult;
  ds_sortGears();
  ds_savePersistedState();
}

void ds_updateGears() {
  memset(ds_persistedState.gears, 0, sizeof(ds_persistedState.gears));
  int count = 0;

  for (long rpk = GEAR_RPK_MAX-GEAR_RPK_INTERVAL-1; rpk >= GEAR_RPK_MIN+GEAR_RPK_INTERVAL; rpk -= GEAR_RPK_INTERVAL) {
    int j = (rpk - GEAR_RPK_MIN + GEAR_RPK_INTERVAL/2) / GEAR_RPK_INTERVAL;

    // Find local maxima
    if (gearRpkTable[j] > gearRpkTable[j-1] && gearRpkTable[j] >= gearRpkTable[j+1] && 
    (gearRpkTable[j] - gearRpkTable[j-1] > GEAR_MIN_DIFF || gearRpkTable[j] - gearRpkTable[j+1] > GEAR_MIN_DIFF)) {
      ds_persistedState.gears[count] = rpk;
      if (++count >= GEAR_MAX_COUNT) break;
    }
  }
  ds_persistedState.gearCount = count;
}

void ds_updateGearTable(float rpk) {
  if (rpk >= GEAR_RPK_MIN && rpk <= GEAR_RPK_MAX) {
    int index = (rpk - GEAR_RPK_MIN + GEAR_RPK_INTERVAL/2) / GEAR_RPK_INTERVAL;
    gearRpkTable[index] ++;
  }
  ds_updateGears();
}

int ds_getGear(float rpk) {
  if (!rpk) return 0;
  long bestDiff = 100000;
  int best = 0;
  for (int i=0; i<ds_persistedState.gearCount; i++) {
    float diff = abs(rpk - ds_persistedState.gears[i]);
    if (diff < bestDiff) {
      bestDiff = diff;
      best = i + 1;
    }
  }
  return best;
}

//------------------------------------------------------
// Private (settings support)
//------------------------------------------------------

void ds_clearHistory(void) {
  ds_persistedState.totalElapsedSeconds = 0;
  ds_persistedState.totalDrivenKilometers = 0;
  ds_persistedState.totalConsumedFuelLitres = 0;
  ds_savePersistedState();
}

void ds_clearDistance(void) {
  ds_persistedState.totalDrivenKilometers = 0;
  ds_savePersistedState();
}

void ds_clearFuel(void) {
  ds_persistedState.totalConsumedFuelLitres = 0;
  ds_savePersistedState();
}

void ds_clearTime(void) {
  ds_persistedState.totalElapsedSeconds = 0;
  ds_savePersistedState();
}

void ds_adjustDistance(void) {
  char *title = "Km.";
  float mult = 1;
  if (ds_persistedState.itemsUsingAltUnitsMask & (1L << DISPLAYABLE_ITEM_TOTAL_DISTANCE)) {
    title = "Mile";
    mult = 0.6214;
  }

  ds_persistedState.totalDrivenKilometers = ds_setValue(title, ds_persistedState.totalDrivenKilometers * mult + mult/2, 0, 100000, 0.1, 1, 10, 1) / mult;
  ds_savePersistedState();
}

void ds_adjustFuel(void) {
  char *title = "Litr";
  float mult = 1;
  if (ds_persistedState.itemsUsingAltUnitsMask & (1L << DISPLAYABLE_ITEM_TOTAL_FUEL)) {
    title = "gALS";
    mult = 0.264;
  }

  ds_persistedState.totalConsumedFuelLitres = ds_setValue(title, ds_persistedState.totalConsumedFuelLitres * mult + mult/2, 0, 500, 0.1, 1.0, 10, 1)/mult;
  ds_savePersistedState();
}

void ds_adjustTime(void) {
  ds_persistedState.totalElapsedSeconds = 60*ds_setValue("Mins", ds_persistedState.totalElapsedSeconds/60, 0, 10000, 1, 10, 100, 1);
  ds_savePersistedState();
}

void ds_clearPersistedState(void) {
  for (int i=0; i<sizeof(ds_persistedState); i++) {
    ((unsigned char *)&ds_persistedState)[i] = -1;
  }
  ds_savePersistedState();
  ds_loadPersistedState();
}

void ds_setBrightness(int brightness) {
  ds_persistedState.brightness = brightness;
  ds_output->setBrightness(brightness);
}

void ds_toggleUnits(void) {
  ds_persistedState.itemsUsingAltUnitsMask ^= (1L << ds_persistedState.currentItemIndex);
  ds_savePersistedState();
}

void ds_setAllUnits(bool useAlt) {
  ds_persistedState.itemsUsingAltUnitsMask = useAlt ? 0x7fffffff : 0;
  ds_savePersistedState();
}

void ds_autoScanItemsAt(int base) {
  unsigned char buf[4];

  if (ds_persistedState.demoModeEnabled) {
    *(long *)buf = 0xffffffff;
  } else {
    vobd.sendPidRequest(base, 1);
    if (4 > vobd.receivePidResponseData(buf, 4, base, 1, true, false)) {
       return;
    }
  }

  unsigned long mask = ((unsigned long)buf[0] << 24) | ((unsigned long)buf[1] << 16) | ((unsigned long)buf[2] << 8) | (unsigned long)buf[3];

  for (int i=0; i<DISPLAYABLE_ITEM_COUNT; i++) {
    struct DisplayableItem *item = ds_getDisplayableObject(i);

    if (item->pid >= base + 1 && item->pid <= base + 0x20) {
      if (mask & (0x80000000L >> (item->pid - base - 1))) {
        ds_persistedState.itemsHiddenMask &= ~(1L << i);
      } else {
        ds_persistedState.itemsHiddenMask |= (1L << i);
      }
    } else if (item->pid == 0xff) {
      ds_persistedState.itemsHiddenMask &= ~(1L << i);
    }
  }

  for (int i=0; i<4; i++) {
     char tbuf[6];
     snprintf_P(tbuf, sizeof(tbuf), PSTR("%02X.%02X"), (int)(base + i*8 + 1), (int)buf[i]);
     ds_showStatusString(tbuf);
  }
}

void ds_autoScanItems(void) {
  ds_autoScanItemsAt(0x00);
  ds_autoScanItemsAt(0x20);
  ds_autoScanItemsAt(0x40);

  // If speed and airflow are available, enable burn rate since we can estimate it
  if (!(ds_persistedState.itemsHiddenMask & (1L << DISPLAYABLE_ITEM_SPEED)) &&
      !(ds_persistedState.itemsHiddenMask & (1L << DISPLAYABLE_ITEM_INTAKE_AIR_FLOW))) {
    ds_persistedState.itemsHiddenMask &= ~(1L << DISPLAYABLE_ITEM_FUEL_BURN_RATE);
  }

  // Hide calculated fields requiring burn rate if unavailable
  if (ds_persistedState.itemsHiddenMask & (1L << DISPLAYABLE_ITEM_FUEL_BURN_RATE)) {
    ds_persistedState.itemsHiddenMask |= ((1L << DISPLAYABLE_ITEM_TOTAL_FUEL) | (1L << DISPLAYABLE_ITEM_AVERAGE_EFFICIENCY));
  }
  ds_savePersistedState();

  int count = 0;
  for (int i=0; i<DISPLAYABLE_ITEM_COUNT; i++) {
    if ((ds_persistedState.itemsHiddenMask & (1L << i)) == 0) {
      count ++;
    }
  }
  ds_showStatusString_P(PSTR("Got"));
  ds_showStatusInteger(count);
  ds_showStatusString_P(PSTR("Gges"));
  ds_controls->smartDelay(500);
}

void ds_hideCurrentItem(void) {
  ds_persistedState.itemsHiddenMask |= (1L << ds_persistedState.currentItemIndex);

  // Find next visible item  
  for (ds_persistedState.currentItemIndex++; ds_persistedState.currentItemIndex < DISPLAYABLE_ITEM_COUNT; ds_persistedState.currentItemIndex++) {
    if (!(ds_persistedState.itemsHiddenMask & (1L << ds_persistedState.currentItemIndex))) {
      ds_savePersistedState();
      return;
    }
  }
  ds_persistedState.currentItemIndex = 0;
  ds_savePersistedState();
  return;
}

void ds_showItemByName(char *name) {
  for (int i=0; i<DISPLAYABLE_ITEM_COUNT; i++) {
    if (!strcmp(name, ds_getDisplayableObject(i)->name) && (ds_persistedState.itemsHiddenMask & (1L << i))) {
      ds_persistedState.itemsHiddenMask &= ~(1L << i);
      ds_savePersistedState();
      return;
    }
  }
  ds_showStatusString_P(PSTR("404!"));
  ds_showStatusString(name);
}

void ds_showDetails(void) {
  int pid = ds_getCurrentDisplayableObject()->pid;

  ds_showStatusString_P(PSTR("PID"));
  ds_showStatusByte(pid);
  vobd.sendPidRequest(pid, 1);
  vobd.receivePidResponse(pid, 1, true, 1);
}

void ds_showDtcValue(int byte1, int mask1, int byte2, int mask2, char *name_p) {
    char *ready = PSTR("Redy");
    char *nope = PSTR("Nope");
    if (byte1 & mask1) { ds_showStatusString_P(name_p); ds_showStatusString_P(byte2 & mask2 ? ready : nope); ds_controls->smartDelay(500); }
}

void ds_showDtcMonitors(void) {
  vobd.sendPidRequest(0x01, 1);
  unsigned long value = vobd.receivePidResponse(0x01, 1, true, 0);

  if (value != -1) {
    int a = value >> 24;
    int b = (value >> 16) & 0xff;
    int c = (value >> 8) & 0xff;
    int d = value & 0xff;

    ds_showStatusString_P(PSTR("CEL"));
    ds_showStatusString_P(a & 0xf0 ? PSTR("ON") : PSTR("OFF"));
    ds_controls->smartDelay(500);
    ds_showStatusString_P(PSTR("CDES"));
    ds_showStatusInteger(a & 0x7f);
    ds_controls->smartDelay(500);

    // char *ready = PSTR("Redy");
    // char *nope = PSTR("Nope");

    // Common tests
    // if (b & 0x40) { ds_showStatusString_P(PSTR("COMP")); ds_showStatusString_P(b & 0x04 ? ready : nope); ds_controls->smartDelay(500); }
    // if (b & 0x20) { ds_showStatusString_P(PSTR("FUEL")); ds_showStatusString_P(b & 0x02 ? ready : nope); ds_controls->smartDelay(500); }
    // if (b & 0x10) { ds_showStatusString_P(PSTR("MFIR")); ds_showStatusString_P(b & 0x01 ? ready : nope); ds_controls->smartDelay(500); }
    ds_showDtcValue(b, 0x40, b, 0x04, PSTR("COMP"));
    ds_showDtcValue(b, 0x20, b, 0x02, PSTR("FUEL"));
    ds_showDtcValue(b, 0x10, b, 0x01, PSTR("MFIR"));

    // Diesel
    if (b & 0x08) {
      // if (d & 0x08) { ds_showStatusString_P(PSTR("Boos")); ds_showStatusString_P(c & 0x08 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x01) { ds_showStatusString_P(PSTR("Caty")); ds_showStatusString_P(c & 0x01 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x20) { ds_showStatusString_P(PSTR("EGas")); ds_showStatusString_P(c & 0x20 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x80) { ds_showStatusString_P(PSTR("EGR" )); ds_showStatusString_P(c & 0x80 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x40) { ds_showStatusString_P(PSTR("Filt")); ds_showStatusString_P(c & 0x40 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x02) { ds_showStatusString_P(PSTR("NOS" )); ds_showStatusString_P(c & 0x02 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x10) { ds_showStatusString_P(PSTR("res1")); ds_showStatusString_P(c & 0x10 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x04) { ds_showStatusString_P(PSTR("res2")); ds_showStatusString_P(c & 0x04 ? ready : nope); ds_controls->smartDelay(500); }
      ds_showDtcValue(d, 0x08, c, 0x08, PSTR("Boos"));
      ds_showDtcValue(d, 0x01, c, 0x01, PSTR("Caty"));
      ds_showDtcValue(d, 0x20, c, 0x20, PSTR("EGas"));
      ds_showDtcValue(d, 0x80, c, 0x80, PSTR("EGR"));
      ds_showDtcValue(d, 0x40, c, 0x40, PSTR("Filt"));
      ds_showDtcValue(d, 0x02, c, 0x02, PSTR("NOS"));
      ds_showDtcValue(d, 0x10, c, 0x10, PSTR("res1"));
      ds_showDtcValue(d, 0x04, c, 0x04, PSTR("res2"));
    } 
    
    // Spark
    else {
      // if (d & 0x08) { ds_showStatusString_P(PSTR("Air2")); ds_showStatusString_P(c & 0x08 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x01) { ds_showStatusString_P(PSTR("Caty")); ds_showStatusString_P(c & 0x01 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x02) { ds_showStatusString_P(PSTR("CtHt")); ds_showStatusString_P(c & 0x02 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x80) { ds_showStatusString_P(PSTR("EGR" )); ds_showStatusString_P(c & 0x80 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x04) { ds_showStatusString_P(PSTR("Evap")); ds_showStatusString_P(c & 0x04 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x10) { ds_showStatusString_P(PSTR("Filt")); ds_showStatusString_P(c & 0x10 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x20) { ds_showStatusString_P(PSTR("O2"  )); ds_showStatusString_P(c & 0x20 ? ready : nope); ds_controls->smartDelay(500); }
      // if (d & 0x40) { ds_showStatusString_P(PSTR("O2Ht")); ds_showStatusString_P(c & 0x40 ? ready : nope); ds_controls->smartDelay(500); }
      ds_showDtcValue(d, 0x08, c, 0x08, PSTR("Air2"));
      ds_showDtcValue(d, 0x01, c, 0x01, PSTR("Caty"));
      ds_showDtcValue(d, 0x02, c, 0x02, PSTR("CtHt"));
      ds_showDtcValue(d, 0x80, c, 0x80, PSTR("EGR"));
      ds_showDtcValue(d, 0x04, c, 0x04, PSTR("Evap"));
      ds_showDtcValue(d, 0x10, c, 0x10, PSTR("Filt"));
      ds_showDtcValue(d, 0x20, c, 0x20, PSTR("O2"));
      ds_showDtcValue(d, 0x40, c, 0x40, PSTR("O2Ht"));
    }
  }
  ds_controls->smartDelay(1500);
}

void ds_showDtcCode(long value) {
  if (!value) return;
  switch(value & 0xc0) {
    case 0x00: ds_showStatusString_P(PSTR(" P- ")); break;
    case 0x40: ds_showStatusString_P(PSTR(" C- ")); break;
    case 0x80: ds_showStatusString_P(PSTR(" B- ")); break;
    case 0xc0: ds_showStatusString_P(PSTR(" U- ") ); break;
  }
  char buf[6];
  snprintf_P(buf, sizeof(buf), PSTR("%04lx"), value & 0x3fff);
  ds_showStatusString(buf);
  ds_controls->smartDelay(2000);
}

void ds_showDtcCodes(void) {
  vobd.sendPidRequest(0x00, 3);
  unsigned char bytes[10];
  int byteCount = vobd.receivePidResponseData(bytes, 10, 0x0, 3, true, 0);
  bool foundCode = false;

  for (int i = 0; i+1 < byteCount; i += 2) {
    long code = (((long)bytes[i])<<8) | bytes[i+1];
    if (code) {
      ds_showDtcCode(code);
      ds_controls->smartDelay(1500);
      foundCode = true;
    }
  }
  if (!foundCode) {
    ds_showStatusString_P(PSTR("None"));
    ds_controls->smartDelay(2000);
  }

  // SAMPLE: 87 f1 11 43   20 11 20 12 20 13
}

void ds_clearDtcCodes(void) {
  vobd.sendPidRequest(0x00, 4);
  unsigned long value = vobd.receivePidResponse(0x0, 4, true, 0);
  if (value != -1) {
    ds_showStatusString_P(PSTR("Done"));
    ds_controls->smartDelay(2000);
  }
}

void ds_toggleLoopMode(void) {
  ds_persistedState.loopModeEnabled = !ds_persistedState.loopModeEnabled;
  ds_savePersistedState();
}

void ds_toggleDemoMode(void) {
  ds_persistedState.demoModeEnabled = !ds_persistedState.demoModeEnabled;
  ds_savePersistedState();
}

void ds_toggleDebugMode(void) {
  ds_debugModeEnabled = !ds_debugModeEnabled;
}

void ds_enterSniffMode(int mode) {
  ds_debugModeEnabled = true; // disable pings

  while (!ds_controls->isButton1Down()) {
    vobd.receivePidResponse(0x0, 0, false, mode + 1);

    // Example: c2 33 F1 0d 01 F4 -> 83 F1 11 41 0d 64 37
  }
  unsigned long end = millis();
  while (ds_controls->isButton1Down() && ds_controls->isButton2Down() && millis() < end+2000L) {}

  ds_debugModeEnabled = false; // disable pings
}

void ds_setFuelAdjustment(void) {
  ds_persistedState.fuelAdjustment = ds_setValue("Mult", ds_persistedState.fuelAdjustment, FUEL_ADJUST_MIN, FUEL_ADJUST_MAX, FUEL_ADJUST_DELTA, FUEL_ADJUST_DELTA*5, FUEL_ADJUST_DELTA*20, 2);
  ds_savePersistedState();
}

void ds_setKgWeight(void) {
  char *title = "Kgs.";
  float mult = 1;
  if (ds_persistedState.itemsUsingAltUnitsMask & (1L << DISPLAYABLE_ITEM_HORSEPOWER)) {
    title = "Lbs.";
    mult = 2.204;
  }

  ds_persistedState.kgWeight = ds_setValue(title, ds_persistedState.kgWeight * mult + mult/2, WEIGHT_MIN * mult, WEIGHT_MAX * mult, 1, 10, 10, 0) / mult;
  ds_savePersistedState();
}

bool ds_isCurrentItemHidden() {
  return ds_isDisplayableHidden(ds_persistedState.currentItemIndex);
}

bool ds_isCurrentItemMultiUnit(void) {
  return ds_getCurrentDisplayableObject()->unit2[0] != 0;
}

bool ds_isCurrentItemComputed(void) {
  switch (ds_persistedState.currentItemIndex) {
    case DISPLAYABLE_ITEM_TOTAL_DISTANCE:
    case DISPLAYABLE_ITEM_TOTAL_TIME:
    case DISPLAYABLE_ITEM_TOTAL_FUEL:
    case DISPLAYABLE_ITEM_AVERAGE_SPEED:
    case DISPLAYABLE_ITEM_AVERAGE_EFFICIENCY:
    case DISPLAYABLE_ITEM_BATTERY_VOLTS:
    case DISPLAYABLE_ITEM_GEAR:
    case DISPLAYABLE_ITEM_GFORCE:
    case DISPLAYABLE_ITEM_HORSEPOWER:
      return true;
  }
  return false;
}

char *ds_getCurrentItemAltUnits(void) {
  if ((ds_persistedState.itemsUsingAltUnitsMask & (1L << ds_persistedState.currentItemIndex)) && ds_getCurrentDisplayableObject()->unit2[0]) {
    return ds_getCurrentDisplayableObject()->unit2;
  } else {
    return ds_getCurrentDisplayableObject()->unit1;
  }
}

static char dsHiddenItemTitlesBuf[DISPLAYABLE_ITEM_COUNT*8];
static char *dsHiddenItemTitles[DISPLAYABLE_ITEM_COUNT+1];

char **ds_getHiddenItemTitles(void) {
  char *buf = dsHiddenItemTitlesBuf;
  char **titles = dsHiddenItemTitles;

  // Add initial menu (a little odd that it is here)
  strcpy(buf, "BACK");
  *titles++ = buf;
  buf += 5;

  for (int i=0; i<DISPLAYABLE_ITEM_COUNT; i++) {
    if (ds_isDisplayableHidden(i)) {
      *titles++ = buf;
      strcpy(buf, ds_getDisplayableObject(i)->name);
      buf += strlen(buf);
      *buf++ = '\0';
    }
  }
  *titles = NULL;

  return dsHiddenItemTitles;
} 

static char dsHiddenItemUnitsBuf[DISPLAYABLE_ITEM_COUNT*8];
static char *dsHiddenItemUnits[DISPLAYABLE_ITEM_COUNT+1];

char **ds_getHiddenItemUnits(void) {
  char *buf = dsHiddenItemUnitsBuf;
  char **titles = dsHiddenItemUnits;

  // Add initial menu (a little odd that it is here)
  strcpy(buf, "BACK");
  *titles++ = buf;
  buf += 5;

  for (int i=0; i<DISPLAYABLE_ITEM_COUNT; i++) {
    if (ds_isDisplayableHidden(i)) {
      *titles++ = buf;
      if ((ds_persistedState.itemsUsingAltUnitsMask & (1L << i)) && ds_getDisplayableObject(i)->unit2[0]) {
        strcpy(buf, ds_getDisplayableObject(i)->unit2);
      } else {
        strcpy(buf, ds_getDisplayableObject(i)->unit1);
      }
      buf += strlen(buf);
      *buf++ = '\0';
    }
  }
  *titles = NULL;

  return dsHiddenItemUnits;
} 

static struct SettingsDataSource ds_settingsDataSource = {
  ds_clearHistory,
  ds_clearDistance,
  ds_clearFuel,
  ds_clearTime,
  ds_adjustDistance,
  ds_adjustFuel,
  ds_adjustTime,
  ds_setBrightness,
  ds_toggleUnits,
  ds_setAllUnits,
  ds_autoScanItems,
  ds_hideCurrentItem,
  ds_showItemByName,
  ds_showDetails,
  ds_showDtcMonitors,
  ds_showDtcCodes,
  ds_clearDtcCodes,
  ds_setFuelAdjustment,
  ds_setKgWeight,
  ds_startGearDetect,
  ds_getGearCount,
  ds_showGears,
  ds_addGear,
  ds_removeGear,
  ds_editGear,
  ds_toggleLoopMode,
  ds_toggleDemoMode,
  ds_toggleDebugMode,
  ds_enterSniffMode,

  ds_isCurrentItemHidden,
  ds_isCurrentItemMultiUnit,
  ds_isCurrentItemComputed,
  ds_getCurrentItemAltUnits,
  ds_getHiddenItemTitles,
  ds_getHiddenItemUnits,
};

void ds_showStatusState() {
  ds_output->showStatusState(ds_connecting, ds_resetting, ds_requestErrorCount, ds_connectionErrorCount, ds_testProtocol - OBD_PROTOCOL_FIRST);
}

float ds_scaleDisplayableValue(float fvalue, struct DisplayableItem *disp, bool useAltUnits) {
  if (!ds_persistedState.demoModeEnabled) {
    fvalue += (useAltUnits) ? disp->offset2 : disp->offset1;
    fvalue = fvalue * (useAltUnits ? disp->multiplier2 : disp->multiplier1);
    fvalue /= (useAltUnits ? disp->divisor2 : disp->divisor1);
  }
  return fvalue;
}

void ds_showDisplayableNumber(float fvalue, struct DisplayableItem *disp, bool useAltUnits, char suffix) {
  fvalue = ds_scaleDisplayableValue(fvalue, disp, useAltUnits);
  ds_output->showFloatValue(fvalue, disp->decimals, suffix ?: disp->suffix, disp->addPlus);
}

void ds_showDisplayableBar(float fvalue, struct DisplayableItem *disp, bool useAltUnits, bool showAsSpot) {
  fvalue = ds_scaleDisplayableValue(fvalue, disp, useAltUnits);

  int16_t minVal = (useAltUnits) ? disp->min2 : disp->min1;
  int16_t maxVal = (useAltUnits) ? disp->max2 : disp->max1;
  int lightCount = ds_output->getBarCount();

  if (showAsSpot) {
    int index = ((fvalue - minVal) * lightCount - 1) / (maxVal - minVal);
    ds_output->setBarColor(max(min(index, lightCount - 1), 0), disp->spotColor);
    return;
  }

  // Update meter ring
  int index = ((fvalue - minVal) * lightCount - 1) / (maxVal - minVal);

  // Show bar for primary value
  for (int i = 0; i < lightCount; i++) {    
    int center = lightCount/2;

    if (!disp->centered ? (i == 0 || i <= index) : 
        i == center || (index < center ? (i >= index && i <= center) : (i <= index && i >= center))
    ) {
      ds_output->setBarColor(i, disp->colors[i]);
    } else {
      ds_output->setBarColor(i, 'k');
    }
  }
}

float ds_setValue(char *title, float value, float minVal, float maxVal, float interval1, float interval2, float interval3, int digits) {
    int count = 0;

    maxVal = max(maxVal, value);
    minVal = min(minVal, value);

    if (title) {
      ds_showStatusString(title);
      ds_controls->smartDelay(1000);
    }

    while(1) {
      bool b1 = ds_controls->isButton1Down();
      bool b2 = ds_controls->isButton2Down();

      float delta = count > 20 ? interval3 : count > 10 ? interval2 :interval1;

      if (b1 && b2) break;
      else if (b1) { value += delta; count++; }
      else if (b2) { value -= delta; count++; }
      else count = 0;

      value = min(max(value, minVal), maxVal);
      ds_controls->smartDelay(1);

      ds_output->showFloatValue((float)value, digits, 0, false);
    }
    return value;
}

//------------------------------------------------------
// Public
//------------------------------------------------------

extern void VDisplayables::setup(int inPin, int outPin, int powerAnalogPin, struct DisplayablesOutputProvider *output, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls) {
  ds_loadPersistedState();
  ds_output = output;
  ds_controls = controls;
  ds_powerAnalogPin = powerAnalogPin;
  vmenu.setup(&ds_menuDataSource, display, controls);
  vsettings.setup(&ds_settingsDataSource, display, controls);
  vobd.setup(inPin, outPin, controls->smartDelay, &ds_outputProvider);
  ds_output->setBrightness(ds_persistedState.brightness);
  gearRpkSamplingMsRemaining = 0;

  // Startup overrides
  long start = millis();
  long end = start + 3000;
  int barCount = ds_output->getBarCount();

  // Hold buttons for hard memory reset
#if !WOKWI
  while(1) {
    long time = millis();
    bool b1 = ds_controls->isButton1Down();
    bool b2 = ds_controls->isButton2Down();

    if (!b1 && !b2) break;

    int bar = (time - start) * barCount / (end - start);
    for (int i=0; i<barCount; i++) ds_output->setBarColor(i, i>bar ? 'k' : 'r'); 
    ds_output->showBar(); 
    ds_output->showStatusString_P(b1 && b2 ? PSTR("FRes") : PSTR("Snif"));

    if (time > end) {
      for (int i=0; i<barCount; i++) ds_output->setBarColor(i, 'k'); 
      ds_output->showBar(); 

      while ((ds_controls->isButton1Down() || ds_controls->isButton2Down()) && millis() < end+2000L) {}

      if (b1 && b2) {
        ds_clearPersistedState();
        ds_output->showSweep('b');
      } else if (b1) {
        ds_enterSniffMode(1);
        ds_output->showSweep('v');
      } else {
        ds_enterSniffMode(0);
        ds_output->showSweep('y');
      }
      break;
    }
  }
#endif
}

extern void VDisplayables::mainLoop() {
  if (!vobd.isConnected()) {
    showCurrentItem();
    vmenu.mainLoop(false);
    vmenu.highlightCurrentItem();
  } else {
    long time = millis();
    bool saveTime = false;

    // Cycle to next gauge
    if (ds_persistedState.loopModeEnabled && (unsigned long)(time - ds_lastLoopChangeMillis) > LOOP_IDLE_CYCLE_MILLIS) {
      for (int i = DISPLAYABLE_ITEM_COUNT; i--;) {
        ds_persistedState.currentItemIndex = (ds_persistedState.currentItemIndex + 1) % DISPLAYABLE_ITEM_COUNT;
        if (!ds_menuDataSource.isItemHidden(ds_persistedState.currentItemIndex, &ds_menuDataSource)) break;
      }
      vmenu.highlightCurrentItem();
      showCurrentItem();
      saveTime = true;
    } else {
      saveTime = vmenu.mainLoop(false);
    }

    if (saveTime) {
      ds_lastLoopChangeMillis = time;
    } 
  }

  // Connect if needed
  if (!vobd.isConnected()) {
    ds_output->showStatusString_P(PSTR("Init"));
    ds_output->showSweep('r');

    // Show error counts
    if (ds_debugModeEnabled) {
      char counts[4];
      counts[0] = 'E';
      counts[1] = ds_connectionErrorCount + '0';
      counts[2] = ds_totalRequestErrorCount + '0';
      counts[3] = ds_requestErrorCount + '0';
      counts[4] = 0;
      ds_output->showStatusString(counts);
      ds_controls->smartDelay(500);
    }

    // Reset current protocol to auto if enough failures
    // Since kwp/9141 can have false positives (at least on sim), base switching to automatic based on request error
    if ((ds_totalRequestErrorCount >= 9 || ds_connectionErrorCount >= 3) && ds_persistedState.protocol != OBD_PROTOCOL_AUTOMATIC) {
      ds_totalRequestErrorCount = 0;
      ds_persistedState.protocol = OBD_PROTOCOL_AUTOMATIC;
      ds_savePersistedState();
      ds_output->showStatusString_P(PSTR("Auto")); ds_controls->smartDelay(300); ds_output->showStatusString_P(PSTR("Scan"));
      ds_controls->smartDelay(500);
    }    

    // Connect
    ds_testProtocol = (ds_persistedState.protocol != OBD_PROTOCOL_AUTOMATIC) ? ds_persistedState.protocol : ds_testProtocol + 1;
    if (ds_testProtocol > OBD_PROTOCOL_LAST) ds_testProtocol = OBD_PROTOCOL_FIRST;

    ds_connecting = true; ds_showStatusState();

    switch(ds_testProtocol) {
      case OBD_PROTOCOL_ISO_9141: ds_output->showStatusString_P(PSTR("9141")); break;
      case OBD_PROTOCOL_KWP_SLOW: ds_output->showStatusString_P(PSTR("Slow")); ds_controls->smartDelay(300); ds_output->showStatusString_P(PSTR("2000")); break;
      case OBD_PROTOCOL_KWP_FAST: ds_output->showStatusString_P(PSTR("Fast")); ds_controls->smartDelay(300); ds_output->showStatusString_P(PSTR("2000")); break;
    }

    vobd.connect(ds_testProtocol,  ds_persistedState.demoModeEnabled);

    ds_connecting = false; ds_showStatusState();

    if (vobd.isConnected()) {
      if (!ds_persistedState.hasAutoScannedItems) {
        ds_persistedState.hasAutoScannedItems = 1;
        ds_savePersistedState();
        ds_autoScanItems();
      }

      ds_connectionErrorCount = 0; ds_requestErrorCount = 0; ds_showStatusState();
      showCurrentItem();
    }
  } 
  
  // Fetch value and update display if success
  else if (updateCurrentItemValue()) {
    // Save current protocol if success
    if (ds_persistedState.protocol != ds_testProtocol) {
      ds_persistedState.protocol = ds_testProtocol;
      ds_savePersistedState();
    }

    ds_requestErrorCount = 0; ds_totalRequestErrorCount = 0; ds_showStatusState();      
  }

  // Handle failure - disconnect after enough request failures
  else {
    ++ds_totalRequestErrorCount;
    ++ds_requestErrorCount; 
    ds_showStatusState();

    if (ds_requestErrorCount >= 3) {
      vobd.disconnect();
    }
  } 

  // Reset if not connected at end of loop
  if (!vobd.isConnected()) {
      ds_connectionErrorCount++; ds_showStatusState();
      ds_output->showStatusString_P(PSTR("\x02\x02\x02\x02"));
      ds_resetting = true; ds_showStatusState();
      vobd.resetConnection();
      ds_resetting = false; ds_showStatusState();
  }
}

static unsigned long lastMillis = 0;
static float lastDemoValue = 0;
static float lastSpeedValue = 0;
static float demoValue = 0;
static bool demoValueIncrements;

extern bool VDisplayables::updateCurrentItemValue() {
  struct DisplayableItem *disp = ds_getCurrentDisplayableObject();
  bool useAltUnits = (ds_persistedState.itemsUsingAltUnitsMask & (1L << ds_persistedState.currentItemIndex)) && disp->unit2[0];
  float fvalue = 0;
  float fvalue2 = -1;
  char suffix = 0;
  long ms = millis();
  long deltaMs = ms - lastMillis;

  // Update accumulated values
  if (deltaMs > 0) {
    ds_persistedState.totalElapsedSeconds += ((double)(deltaMs))/1000.0;
  } 
  lastMillis = ms;

  //
  // DEMO MODE
  //

  if (ds_persistedState.demoModeEnabled) {
    int16_t minVal = (useAltUnits) ? min(disp->min2, disp->max2) : min(disp->min1, disp->max1);
    int16_t maxVal = (useAltUnits) ? max(disp->min2, disp->max2) : max(disp->min1, disp->max1);
    float step = (maxVal-minVal)/50.0;

    ds_controls->smartDelay(100);
    if (!random(50)) {
      demoValueIncrements = !demoValueIncrements;
    }

    switch (ds_persistedState.currentItemIndex) {
      case DISPLAYABLE_ITEM_TOTAL_TIME:
        demoValue = ds_persistedState.totalElapsedSeconds;
        break;
      case DISPLAYABLE_ITEM_TOTAL_DISTANCE:
      case DISPLAYABLE_ITEM_TOTAL_FUEL:
        demoValue += step/700;
        break;
      case DISPLAYABLE_ITEM_BATTERY_VOLTS:
        demoValue = analogRead(ds_powerAnalogPin) * (5.0 * BATTERY_VOLTAGE_DIVIDE / 1023.0);
        break;
      default:
        if (demoValueIncrements) {
          if (demoValue > maxVal-step) { demoValue = maxVal-step; demoValueIncrements = false; }
          else demoValue += step;
        } else {
          if (demoValue < minVal+step) { demoValue = minVal+step; demoValueIncrements = true; }
          else demoValue -= step;
        }
        break;
    }

    if (ds_persistedState.currentItemIndex == DISPLAYABLE_ITEM_AVERAGE_SPEED ||
        ds_persistedState.currentItemIndex == DISPLAYABLE_ITEM_AVERAGE_EFFICIENCY) {
      lastDemoValue = (20 * lastDemoValue + demoValue)/21;
      fvalue = lastDemoValue;
      fvalue2 = demoValue;
    } else {
      fvalue = demoValue;
    }
  } 
  
  //
  // LIVE MODE
  //

  else {

    // Always fetch speed (in km/hr) and fuel consumption (in l/20hr) for accumulated values
    vobd.sendPidRequest(PID_SPEED, 1);
    long speedValue = vobd.receivePidResponse(PID_SPEED, 1, true, 0);
    long mafValue = -1;

    float deltaSpeedValue = speedValue - lastSpeedValue;
    lastSpeedValue = speedValue;

    // Speed should be supported by everything, so return error if no
    if (speedValue == -1) return false;

    vobd.sendPidRequest(PID_BURN_VALUE, 1);
    long burnValue = vobd.receivePidResponse(PID_BURN_VALUE, 1, false, 0); // swallow error silently as not critical

    // If burn value is not available, estimate from MAF
    // - divide by 14.7 (ideal air/fuel ratio) to get g/s of gas
    // - multiply by 3600 to get g/hour of gas
    // - divide by 740g/l to get l/hour of gas
    // - divide by 5 for difference between burn/maf rate formula divisors
    if (burnValue < 0 || (burnValue == 0 && speedValue > 0)) {
      vobd.sendPidRequest(PID_MAF, 1);
      mafValue = vobd.receivePidResponse(PID_MAF, 1, false, 0);
      if (mafValue >= 0) {
        burnValue = (float)mafValue * (3600.0 / (14.7 * 740 * 5));
      }
    }

    // Update accumulated values
    if (deltaMs > 0) {
      if (speedValue > 0) {
        ds_persistedState.totalDrivenKilometers += ((double)(deltaMs))*speedValue/3600000.0;
      }
      if (burnValue > 0) {
        ds_persistedState.totalConsumedFuelLitres += ((double)(deltaMs))*burnValue/(3600000.0*20.0);
      }
    }

    switch (ds_persistedState.currentItemIndex) {
      case DISPLAYABLE_ITEM_TOTAL_DISTANCE:
        fvalue = ds_persistedState.totalDrivenKilometers;
        break;

      case DISPLAYABLE_ITEM_TOTAL_TIME:
        fvalue = ds_persistedState.totalElapsedSeconds;
        fvalue2 = ((long)ds_persistedState.totalElapsedSeconds % 13) * (60.0 / 13);
        suffix = 's';
        if (fvalue > 60) {
          fvalue /= 60;
          suffix = 'm';
          if (fvalue > 60) {
            fvalue /= 60;
            suffix = 'h';
          }
        }
        // Convert fraction to fraction of 60
        fvalue = floor(fvalue) + (fvalue-floor(fvalue))*(60.0/100.0);
        break;
        
      case DISPLAYABLE_ITEM_TOTAL_FUEL:
        fvalue = ds_persistedState.totalConsumedFuelLitres * ds_persistedState.fuelAdjustment;
        break;

      case DISPLAYABLE_ITEM_AVERAGE_SPEED:
        fvalue = ds_persistedState.totalDrivenKilometers/ds_persistedState.totalElapsedSeconds*3600.0;
        fvalue2 = (speedValue > 0) ? speedValue : 0;
        break;

      case DISPLAYABLE_ITEM_AVERAGE_EFFICIENCY:
        if (useAltUnits) {
          fvalue = (ds_persistedState.totalDrivenKilometers <= 0) ? 0 : ds_persistedState.totalConsumedFuelLitres / ds_persistedState.totalDrivenKilometers * ds_persistedState.fuelAdjustment;
          fvalue2 = (speedValue <= 0) ? 100000.0 : (burnValue / 20.0) / speedValue * ds_persistedState.fuelAdjustment;
        } else {
          fvalue = (ds_persistedState.totalConsumedFuelLitres <= 0) ? 0 : ds_persistedState.totalDrivenKilometers / ds_persistedState.totalConsumedFuelLitres / ds_persistedState.fuelAdjustment;
          fvalue2 = (speedValue < 0) ? 0 : (burnValue <= 0) ? 0 : speedValue / (burnValue / 20.0) / ds_persistedState.fuelAdjustment;
        }
        break;

      case DISPLAYABLE_ITEM_BATTERY_VOLTS:
        fvalue = analogRead(ds_powerAnalogPin) * (5.0 * BATTERY_VOLTAGE_DIVIDE / 1023.0);
        break;

      case DISPLAYABLE_ITEM_GFORCE:
        // Km/H / ms * 1000m/Km * H/3600s * 1000ms/s * G*s*s/9.8m
        fvalue = (deltaMs > 0) ? deltaSpeedValue / deltaMs * 1000.0 / 3600.0 * 1000.0 / 9.8 : 0;
        break;

      case DISPLAYABLE_ITEM_HORSEPOWER:
        {
          float g = (deltaMs > 0) ? deltaSpeedValue / deltaMs * 1000.0 / 3600.0 * 1000.0 / 9.8 : 0;
          float mass = ds_persistedState.kgWeight;

          fvalue = g * mass * speedValue * (1000.0/3600.0) / 75.0 / (useAltUnits ? 1.014 : 1.0);
          fvalue = fabs(fvalue);
        }
        break;

      default:
        long value = -1;

        // Use previously fetched speed value if available
        if (disp->pid == PID_SPEED && speedValue >= 0) {
          value = speedValue;
        }

        // Use previously fetched maf value if available
        else if (disp->pid == PID_MAF && mafValue >= 0) {
          value = mafValue;
        }

        // Use previously fetched burn value if available
        else if (disp->pid == PID_BURN_VALUE && burnValue >= 0) {
          value = (float)burnValue * ds_persistedState.fuelAdjustment;
        }

        // Else get PID value
        else {
          vobd.sendPidRequest(disp->pid, 1);
          // Just return 0 if PID is not supported, as some are optional
          value = vobd.receivePidResponse(disp->pid, 1, true, ds_debugModeEnabled);
          if (value == -1) return 0;
        }

        // Offset and scale value
        value = (unsigned long)value >> disp->shift;
        value &= disp->mask;
        fvalue = (float)value;

        // Do special gear calculation
        if (ds_persistedState.currentItemIndex == DISPLAYABLE_ITEM_GEAR) {
          // NV Ratio is RPM/KPH
          float rpm = fvalue / 4;
          if (speedValue == 0) fvalue = 0;
          else fvalue = rpm / speedValue;

          // Calibrate
          if (gearRpkSamplingMsRemaining > 0 && deltaMs > 0 && speedValue > 0) {
            ds_updateGearTable(fvalue);            
            gearRpkSamplingMsRemaining -= deltaMs;
            if (gearRpkSamplingMsRemaining <= 0) {
              ds_savePersistedState();

              // Show final gear values
              ds_output->showSweep('R');

              ds_showGears();
            }
          }
        }
    }
  }

  // Show Gear
  if (ds_persistedState.currentItemIndex == DISPLAYABLE_ITEM_GEAR) {
    // Gear number
    if (gearRpkSamplingMsRemaining <= 0 && ds_persistedState.gearCount > 1) {
      int val = ds_getGear(fvalue);
      char *gears[GEAR_MAX_COUNT+1] = { " -- ", "1 st", "2 nd", "3 rd", "4 th", "5 th", "6 th", "7 th", "8 th" };

      ds_output->showStatusString(gears[val]);
    }
    // NV Ratio
    else {
      ds_showDisplayableNumber(fvalue, disp, useAltUnits, suffix);
    }
    // Bar is always in max - (k/NV - min) to match gears
    if (fvalue > 0) {
      ds_showDisplayableBar(GEAR_DIVIDEND/fvalue, disp, false, false);
    } else {
      ds_showDisplayableBar(0, disp, false, false);
    }

    // Show detection countdown
    if (gearRpkSamplingMsRemaining > 0) {
        int lightCount = ds_output->getBarCount();
        ds_output->setBarColor(((float)gearRpkSamplingMsRemaining * lightCount - 1) / GEAR_DETECT_TIME, 'G');
    }

    // Show detected gear points
    for (int i=0; i<ds_persistedState.gearCount; i++) {
      if (ds_persistedState.gears[i] > 0) {
        ds_showDisplayableBar(GEAR_DIVIDEND/(float)ds_persistedState.gears[i], disp, false, true);
      }
    }
  } 

  // Show primary number and graph
  else {
    ds_showDisplayableNumber(fvalue, disp, useAltUnits, suffix);
    ds_showDisplayableBar(fvalue, disp, useAltUnits, false);
  }

  // Show spot for secondary "instantaneous" value
  if (fvalue2 >= 0) {
    ds_showDisplayableBar(fvalue2, disp, useAltUnits, true);
  }

  ds_output->showBar();

  return true;
}

extern void VDisplayables::ping() {
  if (!ds_persistedState.demoModeEnabled) {
    vobd.ping();
  }
}

extern bool VDisplayables::showCurrentItem() {
  return vmenu.showCurrentItem(true);
}

extern bool VDisplayables::savePersistedState() {
  bool longEnuf = ds_persistedState.totalElapsedSeconds > ds_persistedState.sessionElapsedSeconds + 10.0;
  if (longEnuf) {
    ds_savePersistedState();
    ds_persistedState.sessionElapsedSeconds = ds_persistedState.totalElapsedSeconds;
  }
  return longEnuf;
}
