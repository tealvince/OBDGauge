#include "VDisplayables.h"
#include "VSettings.h"
#include "VObd.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

#define FAKE_FUEL_DATA 1
#define FUEL_ADJUST_MIN 0.10
#define FUEL_ADJUST_MAX 10.00
#define FUEL_ADJUST_DELTA 0.01

///////////////////////////////////////////////////////////////
// DISPLAYABLES
// Displayable OBD Gauges
///////////////////////////////////////////////////////////////

struct DisplayableItem {
  char menuColor;
  char spotColor;
  char colors[14];
  char name[7];
  char unit1[7];
  char unit2[7];
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

#define DISPLAYABLE_ITEM_BATTERY_VOLTS    0
#define DISPLAYABLE_ITEM_TOTAL_TIME       1
#define DISPLAYABLE_ITEM_TOTAL_FUEL       2
#define DISPLAYABLE_ITEM_TOTAL_DISTANCE   3
#define DISPLAYABLE_ITEM_TOTAL_EFFICIENCY 4
#define DISPLAYABLE_ITEM_AVERAGE_SPEED    5

#define DISPLAYABLE_ITEM_SPEED            6
#define DISPLAYABLE_ITEM_TACHOMETER       7
#define DISPLAYABLE_ITEM_COOLANT_TEMP     8
#define DISPLAYABLE_ITEM_INTAKE_AIR_PRES  9
#define DISPLAYABLE_ITEM_INTAKE_AIR_FLOW  10
#define DISPLAYABLE_ITEM_THROTTLE_PERCENT 11

#define DISPLAYABLE_ITEM_ENGINE_LOAD      12
#define DISPLAYABLE_ITEM_TIMING_ADVANCE   13
#define DISPLAYABLE_ITEM_INTAKE_TEMP      14
#define DISPLAYABLE_ITEM_FUEL_TANK_LEVEL  15
#define DISPLAYABLE_ITEM_FUEL_BURN_RATE   16
#define DISPLAYABLE_ITEM_AIR_FUEL_EQRATIO 17
#define DISPLAYABLE_ITEM_OXY_SENSOR_VOLTS 18

#define DISPLAYABLE_ITEM_COUNT 19

static const DisplayableItem menu_rawDisplayables[] PROGMEM = {
  // Sensor
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'Y',  0, "OYYYGGGgggccc",   "Batt",   "Volt",    "",    'V', false, false, 1, 0xff, 0,  0xffff,   0,    1,     1,   0,   1,      1,   10,   14,   10,  14  },

  // Accumulated
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'r','W', "RRRRRRRRRRRRR",  "t.RUN",   "CLOC",     "",     0, false, false, 2, 0xff, 0,  0xffff,   0,    1,     1,   0,   1,     1,    0,   59,   0,   59 },
  { 'o',  0, "OOOOOOOOOOOOO",  "t.gAS",   "Litr", "gALS",     0, false, false, 2, 0xff, 0,  0xffff,   0,    1,     1,   0, 264,  1000,    0,  200,   0,   60 },
  { 'b',  0, "iiiiiiiiiiiii",  "t.DSt",   "Kilo", "MILE",     0, false, false, 1, 0xff, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  800,   0,  500 },
  { 'l','b', "lllllllllllll",  "Av.EF",   "L100",  "MPg",     0, false, false, 2, 0xff, 0,  0xffff,   0,  100,     1,   0, 621,   264,    0,   40,   0,   50 },
  { 'c','g', "bbbbbbbbbbbbb",  "Av.SP",    "KPH",  "MPH",     0, false, false, 1, 0x0d, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  200,   0,  120 },

  // Bank1
  // mnu spt  colors           name       unit1   unit2     suf  plus   center dec pid shft mask     off1  mult1  div1 off2 mult2 div2  min1 max1  min2 max2     
  { 'g',  0, "GGGGGGGGGGGGG",   "SPED",    "KPH",  "MPH",   ' ', false, false, 0, 0x0d, 0,  0xffff,   0,    1,     1,   0, 621,  1000,    0,  200,   0,  120 },
  { 'y',  0, "yyyyyyyyyoorr",   "TACH",    "RPM",     "",     0, false, false, 0, 0x0c, 0,  0xffff,   0,    1,     4,   0,   1,     1,    0, 7000,   0, 7000 },
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
  { 'n',  0, "ppppppwyyyyyy",    "A/F",   "Rtio",     "",     0, false, true,  2, 0x24, 16, 0xffff,   0,    2, 65536,   0,   1,      1,   0,    2,   0,    2 },
  { 'p',  0, "ppppppppppppp",    "O/2",   "Volt",     "",   'V', false, false, 2, 0x24, 0,  0xffff,   0,    8, 65536,   0,   1,      1,   0,    8,   0,    8 },
};

//------------------------------------------------------
// Private
//------------------------------------------------------

static VMenu vmenu;
static VSettings vsettings;
static VObd vobd;
static int  ds_requestErrorCount;
static int  ds_connectionErrorCount;
static int  ds_lastItemIndex;
static int  ds_powerAnalogPin;
static bool ds_connecting;
static bool ds_resetting;
static int ds_testProtocol = OBD_PROTOCOL_FIRST - 1;
static struct DisplayablesOutputProvider *ds_output;
static struct MenuControlsProvider *ds_controls;
static DisplayableItem ds_itemObject;

struct DisplayablePersistedState {
  int brightness;
  int currentItemIndex;
  long itemsHiddenMask;
  long itemsUsingAltUnitsMask;
  double totalElapsedSeconds;
  double totalDrivenKilometers;
  double totalConsumedFuelLitres;
  double fuelAdjustment;
  int protocol;
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

  // Do safety checks
  if (ds_persistedState.brightness < 0 || ds_persistedState.brightness > 100) ds_persistedState.brightness = 100;
  if (ds_persistedState.currentItemIndex < 0 || ds_persistedState.currentItemIndex >= DISPLAYABLE_ITEM_COUNT) ds_persistedState.currentItemIndex = 0;
  if (ds_persistedState.itemsHiddenMask < 0) ds_persistedState.itemsHiddenMask = 0;
  if (ds_persistedState.itemsUsingAltUnitsMask < 0) ds_persistedState.itemsUsingAltUnitsMask = (long)((unsigned long)-1L >> 1);
  if (ds_persistedState.totalElapsedSeconds < 0 || isnan(ds_persistedState.totalElapsedSeconds)) ds_persistedState.totalElapsedSeconds = 0;
  if (ds_persistedState.totalDrivenKilometers < 0 || isnan(ds_persistedState.totalDrivenKilometers)) ds_persistedState.totalDrivenKilometers = 0;
  if (ds_persistedState.totalConsumedFuelLitres < 0 || isnan(ds_persistedState.totalConsumedFuelLitres)) ds_persistedState.totalConsumedFuelLitres = 0;
  if (ds_persistedState.fuelAdjustment < FUEL_ADJUST_MIN || ds_persistedState.fuelAdjustment > FUEL_ADJUST_MAX || isnan(ds_persistedState.fuelAdjustment)) ds_persistedState.fuelAdjustment = 1.0;
  if (ds_persistedState.protocol < OBD_PROTOCOL_FIRST || ds_persistedState.protocol > OBD_PROTOCOL_LAST) ds_persistedState.protocol = OBD_PROTOCOL_AUTOMATIC;
}

void ds_savePersistedState() {
  for (int i=0; i<sizeof(ds_persistedState); i++) {
    EEPROM.write(i, ((unsigned char *)&ds_persistedState)[i]);
  }
}

//------------------------------------------------------
// Private (obd output support)
//------------------------------------------------------

void ds_showStatusString(char *text) {
  ds_output->showStatusString(text);
  ds_controls->smartDelay(500);
}

void ds_showStatusInteger(int num) {
  char text[5];
  snprintf(text, sizeof(text), "%-4d", (int)num);
  ds_output->showStatusString(text);
  ds_controls->smartDelay(1000);
}

void ds_showStatusByte(int num) {
  char text[5];
  snprintf(text, sizeof(text), "= %02X", (int)num);
  ds_output->showStatusString(text);
  ds_controls->smartDelay(1000);
}

struct ObdOutputProvider ds_outputProvider = {
  ds_showStatusString,
  ds_showStatusInteger,
  ds_showStatusByte,
};

//------------------------------------------------------
// Private (menu support)
//------------------------------------------------------

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
    snprintf(text, sizeof(text), "br %d", (int)ds_persistedState.brightness/25+1);
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
  NULL
};

//------------------------------------------------------
// Private (settings support)
//------------------------------------------------------

void ds_clearHistory(void) {
  ds_persistedState.totalElapsedSeconds = 0;
  ds_persistedState.totalDrivenKilometers = 0;
  ds_persistedState.totalConsumedFuelLitres = 0;
  ds_savePersistedState();
}

void ds_setBrightness(int brightness) {
  ds_persistedState.brightness = brightness;
  ds_output->setBrightness(brightness);
}

void ds_toggleUnits(void) {
  ds_persistedState.itemsUsingAltUnitsMask ^= (1L << ds_persistedState.currentItemIndex);
  ds_savePersistedState();
}

void ds_hideCurrentItem(void) {
  ds_persistedState.itemsHiddenMask |= (1L << ds_persistedState.currentItemIndex);
  for (int i=0; i<DISPLAYABLE_ITEM_COUNT-1; i++) {
    ds_persistedState.currentItemIndex++;
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
  ds_showStatusString("404!");
  ds_showStatusString(name);
}

void ds_showDetails(void) {
  int pid = ds_getCurrentDisplayableObject()->pid;

  ds_showStatusString("PID");
  ds_showStatusByte(pid);
  vobd.sendPidRequest(pid, 1);
  vobd.receivePidResponse(pid, 1, true, true);

}

void ds_showDtcMonitors(void) {
  vobd.sendPidRequest(0x01, 1);
  unsigned long value = vobd.receivePidResponse(0x01, 1, true, false);

  if (value != -1) {
    int a = value >> 24;
    int b = (value >> 16) & 0xff;
    int c = (value >> 8) & 0xff;
    int d = value & 0xff;

    ds_showStatusString("CEL");
    ds_showStatusString(a & 0xf0 ? "ON" : "OFF");
    ds_controls->smartDelay(500);
    ds_showStatusString("CDES");
    ds_showStatusInteger(a & 0x7f);
    ds_controls->smartDelay(500);

    // Common tests
    if (b & 0x40) { ds_showStatusString("COMP"); ds_showStatusString(b & 0x04 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
    if (b & 0x20) { ds_showStatusString("FUEL"); ds_showStatusString(b & 0x02 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
    if (b & 0x10) { ds_showStatusString("MFIR"); ds_showStatusString(b & 0x01 ? "Redy": "Nope"); ds_controls->smartDelay(500); }

    // Diesel
    if (b & 0x08) {
      if (d & 0x08) { ds_showStatusString("Boos"); ds_showStatusString(c & 0x08 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x01) { ds_showStatusString("Caty"); ds_showStatusString(c & 0x01 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x20) { ds_showStatusString("EGas"); ds_showStatusString(c & 0x20 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x80) { ds_showStatusString("EGR" ); ds_showStatusString(c & 0x80 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x40) { ds_showStatusString("Filt"); ds_showStatusString(c & 0x40 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x02) { ds_showStatusString("NOS" ); ds_showStatusString(c & 0x02 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x10) { ds_showStatusString("res1"); ds_showStatusString(c & 0x10 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x04) { ds_showStatusString("res2"); ds_showStatusString(c & 0x04 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
    } 
    
    // Spark
    else {
      if (d & 0x08) { ds_showStatusString("Air2"); ds_showStatusString(c & 0x08 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x01) { ds_showStatusString("Caty"); ds_showStatusString(c & 0x01 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x02) { ds_showStatusString("CtHt"); ds_showStatusString(c & 0x02 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x80) { ds_showStatusString("EGR" ); ds_showStatusString(c & 0x80 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x04) { ds_showStatusString("Evap"); ds_showStatusString(c & 0x04 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x10) { ds_showStatusString("Filt"); ds_showStatusString(c & 0x10 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x20) { ds_showStatusString("O2"  ); ds_showStatusString(c & 0x20 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
      if (d & 0x40) { ds_showStatusString("O2Ht"); ds_showStatusString(c & 0x40 ? "Redy": "Nope"); ds_controls->smartDelay(500); }
    }
  }
  ds_controls->smartDelay(1500);
}

void ds_showDtcCode(long value) {
  if (!value) return;
  switch(value & 0xc0) {
    case 0x00: ds_showStatusString(" P- "); break;
    case 0x40: ds_showStatusString(" C- "); break;
    case 0x80: ds_showStatusString(" B- "); break;
    case 0xc0: ds_showStatusString(" U- " ); break;
  }
  char buf[6];
  sprintf(buf, "%04lx", value & 0x3fff);
  ds_showStatusString(buf);
  ds_controls->smartDelay(2000);
}

void ds_showDtcCodes(void) {
  vobd.sendPidRequest(0x00, 3);
  unsigned char bytes[10];
  int byteCount = vobd.receivePidResponseData(bytes, 10, 0x0, 3, true, false);
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
    ds_showStatusString("None" );
    ds_controls->smartDelay(2000);
  }

  // SAMPLE: 87 f1 11 43   20 11 20 12 20 13
}

void ds_clearDtcCodes(void) {
  vobd.sendPidRequest(0x00, 4);
  unsigned long value = vobd.receivePidResponse(0x0, 4, true, false);
  if (value != -1) {
    ds_showStatusString("Done");
    ds_controls->smartDelay(2000);
  }
}

void ds_setFuelAdjustment(void) {
    char tbuf[10];
    int count = 0;

    while(1) {
      bool b1 = ds_controls->isButton1Down();
      bool b2 = ds_controls->isButton2Down();
      float delta = count > 20 ? FUEL_ADJUST_DELTA * 20 : count > 10 ? FUEL_ADJUST_DELTA * 5 : FUEL_ADJUST_DELTA;

      if (b1 && b2) break;
      else if (b1) { ds_persistedState.fuelAdjustment += delta; count++; }
      else if (b2) { ds_persistedState.fuelAdjustment -= delta; count++; }
      else count = 0;

      ds_persistedState.fuelAdjustment = min(max(ds_persistedState.fuelAdjustment, FUEL_ADJUST_MIN), FUEL_ADJUST_MAX);

      sprintf(tbuf, "%d.%02d", (int)ds_persistedState.fuelAdjustment, (int)(ds_persistedState.fuelAdjustment*100)%100);
      ds_showStatusString(tbuf);
    }
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
    case DISPLAYABLE_ITEM_TOTAL_EFFICIENCY:
    case DISPLAYABLE_ITEM_BATTERY_VOLTS:
      return true;
  }
  return false;
}

char *ds_getCurrentItemAltUnits(void) {
  if (!(ds_persistedState.itemsUsingAltUnitsMask & (1L << ds_persistedState.currentItemIndex)) && ds_getCurrentDisplayableObject()->unit2[0]) {
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
  ds_setBrightness,
  ds_toggleUnits,
  ds_hideCurrentItem,
  ds_showItemByName,
  ds_showDetails,
  ds_showDtcMonitors,
  ds_showDtcCodes,
  ds_clearDtcCodes,
  ds_setFuelAdjustment,

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
  fvalue += (useAltUnits) ? disp->offset2 : disp->offset1;
  fvalue = fvalue * (useAltUnits ? disp->multiplier2 : disp->multiplier1);
  fvalue /= (useAltUnits ? disp->divisor2 : disp->divisor1);

  return fvalue;
}

void ds_showDisplayableNumber(float fvalue, struct DisplayableItem *disp, bool useAltUnits, char suffix) {
  fvalue = ds_scaleDisplayableValue(fvalue, disp, useAltUnits);
  ds_output->showFloatValue(fvalue, disp->decimals, suffix ?: disp->suffix, disp->addPlus);
}

void ds_showDisplayableBar(float fvalue, struct DisplayableItem *disp, bool useAltUnits, bool showAsSpot) {
  fvalue = ds_scaleDisplayableValue(fvalue, disp, useAltUnits);

  int16_t min = (useAltUnits) ? disp->min2 : disp->min1;
  int16_t max = (useAltUnits) ? disp->max2 : disp->max1;
  int lightCount = ds_output->getBarCount();

  if (showAsSpot) {
    int index = ((fvalue - min) * lightCount - 1) / (max - min);
    ds_output->setBarColor(max(min(index, lightCount - 1), 0), disp->spotColor);
    return;
  }

  // Update meter ring
  int index = ((fvalue - min) * lightCount - 1) / (max - min);

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
}

extern void VDisplayables::mainLoop() {
  vmenu.mainLoop(false);

  // Connect if needed
  if (!vobd.isConnected()) {

    ds_output->showStatusString("Init");

    // Reset current protocol to auto if enough failures
    if (ds_connectionErrorCount >= 4 && ds_persistedState.protocol != OBD_PROTOCOL_AUTOMATIC) {
      ds_persistedState.protocol = OBD_PROTOCOL_AUTOMATIC;
      ds_savePersistedState();
    }    

    // Connect
    ds_testProtocol = (ds_persistedState.protocol != OBD_PROTOCOL_AUTOMATIC) ? ds_persistedState.protocol : ds_testProtocol + 1;
    if (ds_testProtocol > OBD_PROTOCOL_LAST) ds_testProtocol = OBD_PROTOCOL_FIRST;

    ds_connecting = true; ds_showStatusState();

    ds_controls->smartDelay(500);
    switch(ds_testProtocol) {
      case OBD_PROTOCOL_ISO_9141: ds_output->showStatusString("9141"); break;
      case OBD_PROTOCOL_KWP_SLOW: ds_output->showStatusString("S-2k"); break;
      case OBD_PROTOCOL_KWP_FAST: ds_output->showStatusString("F-2k"); break;
    }

    vobd.connect(ds_testProtocol);

    ds_connecting = false; ds_showStatusState();

    if (vobd.isConnected()) {
      ds_requestErrorCount = 0; ds_showStatusState();
      showCurrentItem();
    } else { 
      ds_output->showStatusString("Rset");
      ds_resetting = true; ds_showStatusState();
      vobd.resetConnection();
      ds_resetting = false; ds_showStatusState();
    }
  } 
  
  // Update value
  else if (updateCurrentItemValue()) {
    // Save current protocol if success
    if (ds_persistedState.protocol != ds_testProtocol) {
      ds_persistedState.protocol = ds_testProtocol;
      ds_savePersistedState();
    }

    if (ds_requestErrorCount || ds_connectionErrorCount) {        
      ds_requestErrorCount = 0; ds_connectionErrorCount = 0;
    }
    ds_showStatusState();      
  }

  // Handle failure
  else {
    ++ds_requestErrorCount; ds_showStatusState();

    if (ds_requestErrorCount >= 3) {
      vobd.disconnect();
      ds_connectionErrorCount++; ds_showStatusState();
      ds_output->showStatusString("Rset");
      ds_resetting = true; ds_showStatusState();
      vobd.resetConnection();
      ds_resetting = false; ds_showStatusState();
    }
  } 
}

static unsigned long lastMillis = 0;
float foo;
extern bool VDisplayables::updateCurrentItemValue() {
  struct DisplayableItem *disp = ds_getCurrentDisplayableObject();
  bool useAltUnits = (ds_persistedState.itemsUsingAltUnitsMask & (1L << ds_persistedState.currentItemIndex)) && disp->unit2[0];
  float fvalue = 0;
  float fvalue2 = -1;
  char suffix = 0;
  long ms = millis();

  // Always fetch speed (in km/hr) and fuel consumption (in l/20hr) for accumulated values
  vobd.sendPidRequest(0x0d, 1);
  long speedValue = vobd.receivePidResponse(0x0d, 1, true, false);
  if (speedValue == -1) return false;

  vobd.sendPidRequest(0x5e, 1);
  long burnValue = vobd.receivePidResponse(0x5e, 1, false, false); // swallow error silently as not critical

  // Update accumulated values
  if (lastMillis && ms > lastMillis) {
    ds_persistedState.totalElapsedSeconds += ((double)(ms - lastMillis))/1000.0;
    if (speedValue > 0) {
      ds_persistedState.totalDrivenKilometers += ((double)(ms - lastMillis))*speedValue/3600000.0;
    }
    if (FAKE_FUEL_DATA && burnValue <= 0) {
      burnValue = 100 + speedValue * speedValue / 150;
    }
    if (burnValue > 0) {
      ds_persistedState.totalConsumedFuelLitres += ((double)(ms - lastMillis))*burnValue/3600000.0/20.0;
    }
  }
  lastMillis = ms;

  switch (ds_persistedState.currentItemIndex) {
    case DISPLAYABLE_ITEM_TOTAL_DISTANCE:
      fvalue = ds_persistedState.totalDrivenKilometers;
      break;

    case DISPLAYABLE_ITEM_TOTAL_TIME:
      fvalue = ds_persistedState.totalElapsedSeconds;
      fvalue2 = ((long)ds_persistedState.totalElapsedSeconds % 13) * 60.0 / 13;
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
      fvalue = floor(fvalue) + (fvalue-floor(fvalue))*60.0/100.0;
      break;
      
    case DISPLAYABLE_ITEM_TOTAL_FUEL:
      fvalue = ds_persistedState.totalConsumedFuelLitres * ds_persistedState.fuelAdjustment;
      break;

    case DISPLAYABLE_ITEM_AVERAGE_SPEED:
      fvalue = ds_persistedState.totalDrivenKilometers/ds_persistedState.totalElapsedSeconds*3600.0;
      fvalue2 = speedValue;
      break;

    case DISPLAYABLE_ITEM_TOTAL_EFFICIENCY:
      if (useAltUnits) {
        fvalue = (ds_persistedState.totalConsumedFuelLitres <= 0) ? 0 : ds_persistedState.totalDrivenKilometers / ds_persistedState.totalConsumedFuelLitres / ds_persistedState.fuelAdjustment;
        fvalue2 = (burnValue <= 0) ? 0 : speedValue / (burnValue / 20.0) / ds_persistedState.fuelAdjustment;
      } else {
        fvalue = (ds_persistedState.totalDrivenKilometers <= 0) ? 0 : ds_persistedState.totalConsumedFuelLitres / ds_persistedState.totalDrivenKilometers * ds_persistedState.fuelAdjustment;
        fvalue2 = (speedValue <= 0) ? 100000.0 : (burnValue / 20.0) / speedValue * ds_persistedState.fuelAdjustment;
      }
      break;

    case DISPLAYABLE_ITEM_BATTERY_VOLTS:
      fvalue = analogRead(ds_powerAnalogPin) * 5.0 * 3.0 / 1023.0;
      break;

    default:
      // Use accumulated speed value is available
      if (disp->pid == 0x0d && speedValue >= 0) {
        fvalue = (float)speedValue;
        break;
      }

      // Use accumulated burn value if available
      if (disp->pid == 0x5e && burnValue >= 0) {
        fvalue = (float)burnValue * ds_persistedState.fuelAdjustment;
        break;
      }

      // Else get PID value
      vobd.sendPidRequest(disp->pid, 1);

      long value = vobd.receivePidResponse(disp->pid, 1, true, false);
      if (value == -1) return false;

      // Offset and scale value
      value = (unsigned long)value >> disp->shift;
      value &= disp->mask;
      fvalue = (float)value;
  }

  // Show primary number and graph
  ds_showDisplayableNumber(fvalue, disp, useAltUnits, suffix);
  ds_showDisplayableBar(fvalue, disp, useAltUnits, false);

  // Show spot for secondary "instantaneous" value
  if (fvalue2 >= 0) {
    ds_showDisplayableBar(fvalue2, disp, useAltUnits, true);
  }

  ds_output->showBar();

  return true;
}

extern bool VDisplayables::showCurrentItem() {
  return vmenu.showCurrentItem(true);
}

extern void VDisplayables::savePersistedState() {
  ds_savePersistedState();
}
