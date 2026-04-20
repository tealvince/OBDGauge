#include "VSettings.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VSETTINGS.CPP
// Settings menu
///////////////////////////////////////////////////////////////

struct SettingsMenuItem {
  char title1[6];
  char title2[6];
  char color;
};

static SettingsDataSource *st_dataSource;
static MenuControlsProvider *st_controls;
static MenuDisplayProvider *st_display;

#define SETTINGS_MENU_ITEM_BACK           0
#define SETTINGS_MENU_ITEM_HISTORY        1
#define SETTINGS_MENU_ITEM_DISPLAY        2
#define SETTINGS_MENU_ITEM_GAUGES         3
#define SETTINGS_MENU_ITEM_CODES          4
#define SETTINGS_MENU_ITEM_VALUES         5
#define SETTINGS_MENU_ITEM_MODES          6
#define SETTINGS_MENU_ITEM_COUNT          7

static const char *st_titles1[] = { "BACK", "HISt", "DISP", "gAgE", "CODE", "Vehi", "SPEC" };
static       char *st_titles2[] = { "BACK", "Adj.", "Adj.", "Adj.", "READ", "Vals", "Mode" };
static const char  st_colors[]  = "byowrcv";

bool st_longPressAction(int current, int button);

static struct MenuDataSource st_menuDataSource = {
  SETTINGS_MENU_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, NULL, st_longPressAction, NULL, 'P', 0, st_titles1, st_titles2, st_colors
};

//------------------------------------------------------
// Private (history menu support)
//------------------------------------------------------

#define SETTINGS_MENU_HISTORY_ITEM_BACK         0
#define SETTINGS_MENU_HISTORY_ITEM_ERASE_ALL    1
#define SETTINGS_MENU_HISTORY_ITEM_ERASE_DIST   2
#define SETTINGS_MENU_HISTORY_ITEM_ERASE_FUEL   3
#define SETTINGS_MENU_HISTORY_ITEM_ERASE_TIME   4
#define SETTINGS_MENU_HISTORY_ITEM_ADJUST_DIST  5
#define SETTINGS_MENU_HISTORY_ITEM_ADJUST_FUEL  6
#define SETTINGS_MENU_HISTORY_ITEM_ADJUST_TIME  7
#define SETTINGS_MENU_HISTORY_ITEM_COUNT        8

static const char *st_historyTitles1[] = { "BACK", "ERAs", "RSet", "RSet", "RSet", "Adj.", "Adj.", "Adj." };
static const char *st_historyTitles2[] = { "BACK",  "All", "dISt", "Fuel", "Time", "dISt", "Fuel", "Time" };
static const char  st_historyColors[]  = "brgowGOW";

static struct MenuDataSource st_historyDataSource = {  
  SETTINGS_MENU_HISTORY_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Y', 0, st_historyTitles1, st_historyTitles2, st_historyColors
};

//------------------------------------------------------
// Private (brighness menu support)
//------------------------------------------------------

bool st_brightnessAction(int current, int button);

static const char *st_brightnessTitles[] = { "br 1","br 2","br 3","br 4","br 5" };
static struct MenuDataSource st_brightnessDataSource = {
  5, NULL, NULL, NULL, NULL, NULL, NULL, NULL, st_brightnessAction, 'O', 0, st_brightnessTitles, st_brightnessTitles, NULL
};

static struct MenuDataSource st_showHiddenDataSource = {
  1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Y', 0, NULL, NULL, "bggggggggggggggggggggggggggggg"
};

static VMenu st_historyMenu;
static VMenu st_britenessMenu;
static VMenu st_gaugesMenu;
static VMenu st_codesMenu;
static VMenu st_valuesMenu;
static VMenu st_modesMenu;

static VMenu st_showHiddenMenu;

bool st_brightnessAction(int current, int button) {
  int briteTable[] = { 0, 13, 25, 50, 100 };
  st_dataSource->setBrightness(briteTable[st_brightnessDataSource.alternateCurrentItem]);
  return false;
}

//------------------------------------------------------
// Private (gauges menu support)
//------------------------------------------------------

#define SETTINGS_MENU_GAUGES_ITEM_BACK         0
#define SETTINGS_MENU_GAUGES_ITEM_UNITS        1
#define SETTINGS_MENU_GAUGES_ITEM_UNITS_SI     2
#define SETTINGS_MENU_GAUGES_ITEM_UNITS_US     3
#define SETTINGS_MENU_GAUGES_ITEM_HIDE         4
#define SETTINGS_MENU_GAUGES_ITEM_SHOW         5 
#define SETTINGS_MENU_GAUGES_ITEM_AUTO_SCAN    6
#define SETTINGS_MENU_GAUGES_ITEM_INFO         7
#define SETTINGS_MENU_GAUGES_ITEM_COUNT        8

static const char *st_gaugesTitles1[] = { "BACK", "UNIt", "All-", "All-", "HIDE", "SHOW", "Auto", "SEE"  };
static const char *st_gaugesTitles2[] = { "BACK", "TOGL", "-SI-", "-US-", "gAgE", "gAgE", "gAgE", "Data" };
static const char  st_gaugesColors[]  = "bycnrgvo";

bool st_isGaugesItemHidden(int item);

static struct MenuDataSource st_gaugesDataSource = {  
  SETTINGS_MENU_GAUGES_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, st_isGaugesItemHidden, NULL, NULL, 'W', 0, st_gaugesTitles1, st_gaugesTitles2, st_gaugesColors
};

bool st_isGaugesItemHidden(int item) {
  switch(item) {
    case SETTINGS_MENU_GAUGES_ITEM_SHOW:
      return st_showHiddenDataSource.itemCount <= 1;
    case SETTINGS_MENU_GAUGES_ITEM_UNITS:
      return !st_dataSource->isCurrentItemMultiUnit();
    case SETTINGS_MENU_GAUGES_ITEM_INFO:
      return st_dataSource->isCurrentItemComputed();
    default:
      return false;
  }
}

//------------------------------------------------------
// Private (codes menu support)
//------------------------------------------------------

#define SETTINGS_MENU_CODES_ITEM_BACK           0
#define SETTINGS_MENU_CODES_ITEM_DTC_MONITORS   1
#define SETTINGS_MENU_CODES_ITEM_DTC_CODE       2
#define SETTINGS_MENU_CODES_ITEM_DTC_CLEAR      3
#define SETTINGS_MENU_CODES_ITEM_COUNT          4

static const char *st_codesTitles1[] = { "BACK", "CHEK", "READ", "CLR" };
static const char *st_codesTitles2[] = { "BACK", "REDY", "CDES", "CDES" };
static const char  st_codesColors[]  = "bcpv";

static struct MenuDataSource st_codesDataSource = {  
  SETTINGS_MENU_CODES_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'R', 0, st_codesTitles1, st_codesTitles2, st_codesColors
};

//------------------------------------------------------
// Private (values menu support)
//------------------------------------------------------

#define SETTINGS_MENU_VALUES_ITEM_BACK           0
#define SETTINGS_MENU_VALUES_ITEM_FUEL_ADJUST    1
#define SETTINGS_MENU_VALUES_ITEM_WEIGHT_ADJUST  2
#define SETTINGS_MENU_VALUES_ITEM_GEAR_SHOW      3
#define SETTINGS_MENU_VALUES_ITEM_GEAR_DETECT    4
#define SETTINGS_MENU_VALUES_ITEM_GEAR_ADD       5
#define SETTINGS_MENU_VALUES_ITEM_GEAR_REMOVE    6
#define SETTINGS_MENU_VALUES_ITEM_GEAR_1         7
#define SETTINGS_MENU_VALUES_ITEM_GEAR_2         8
#define SETTINGS_MENU_VALUES_ITEM_GEAR_3         9
#define SETTINGS_MENU_VALUES_ITEM_GEAR_4         10
#define SETTINGS_MENU_VALUES_ITEM_GEAR_5         11
#define SETTINGS_MENU_VALUES_ITEM_GEAR_6         12
#define SETTINGS_MENU_VALUES_ITEM_GEAR_7         13
#define SETTINGS_MENU_VALUES_ITEM_GEAR_8         14
#define SETTINGS_MENU_VALUES_ITEM_COUNT          15

static const char *st_valuesTitles1[] = { "BACK", "Burn", "Mass", "gear", "Auto", "Add.", "Rem.", "1 st", "2 nd", "3 rd", "4 th", "5 th", "6 th", "7 th", "8 th" };
static const char *st_valuesTitles2[] = { "BACK", "Adj.", "Kg.",  "List", "gear", "gear", "gear", "gear", "gear", "gear", "gear", "gear", "gear", "gear", "gear" };
static const char  st_valuesColors[]  = "bOYWngrvvvvvvvv";

bool st_isValuesItemHidden(int item);

static struct MenuDataSource st_valuesDataSource = {  
  SETTINGS_MENU_VALUES_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, st_isValuesItemHidden, NULL, NULL, 'C', 0, st_valuesTitles1, st_valuesTitles2, st_valuesColors
};

bool st_isValuesItemHidden(int item) {
  if (item >= SETTINGS_MENU_VALUES_ITEM_GEAR_1 && item <= SETTINGS_MENU_VALUES_ITEM_GEAR_8) {
    return st_dataSource->getGearCount() <= item - SETTINGS_MENU_VALUES_ITEM_GEAR_1;
  }
  return false;
}

//------------------------------------------------------
// Private (modes menu support)
//------------------------------------------------------

#define SETTINGS_MENU_MODES_ITEM_BACK           0
#define SETTINGS_MENU_MODES_ITEM_LOOP_MODE      1
#define SETTINGS_MENU_MODES_ITEM_DEMO_MODE      2
#define SETTINGS_MENU_MODES_ITEM_DEBUG_MODE     3
#define SETTINGS_MENU_MODES_ITEM_SNIFF_MODE     4
#define SETTINGS_MENU_MODES_ITEM_COUNT          5

static const char *st_modesTitles1[] = { "BACK", "Loop", "Demo", "DBug", "Snif" };
static const char *st_modesTitles2[] = { "BACK", "Mode", "Mode", "Mode", "Mode" };
static const char  st_modesColors[]  = "bCiNR";

static struct MenuDataSource st_modesDataSource = {  
  SETTINGS_MENU_MODES_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'V', 0, st_modesTitles1, st_modesTitles2, st_modesColors
};

//------------------------------------------------------
// Private (menu support)
//------------------------------------------------------

bool st_longPressAction(int current, int button) {
  switch(current) {

    case SETTINGS_MENU_ITEM_BACK:
      return true;

reshowHistory:
      st_display->showItemTitle("Done");
      delay(1000);
    case SETTINGS_MENU_ITEM_HISTORY:
      st_historyDataSource.alternateCurrentItem = 0;
      switch (st_historyMenu.showMenu(NULL)) {
        case SETTINGS_MENU_HISTORY_ITEM_BACK:        return true;
        case SETTINGS_MENU_HISTORY_ITEM_ERASE_ALL:   st_dataSource->clearHistory();   break;
        case SETTINGS_MENU_HISTORY_ITEM_ERASE_DIST:  st_dataSource->clearDistance();  goto reshowHistory;
        case SETTINGS_MENU_HISTORY_ITEM_ERASE_FUEL:  st_dataSource->clearFuel();      goto reshowHistory;
        case SETTINGS_MENU_HISTORY_ITEM_ERASE_TIME:  st_dataSource->clearTime();      goto reshowHistory;
        case SETTINGS_MENU_HISTORY_ITEM_ADJUST_DIST: st_dataSource->adjustDistance(); goto reshowHistory;
        case SETTINGS_MENU_HISTORY_ITEM_ADJUST_FUEL: st_dataSource->adjustFuel();     goto reshowHistory;
        case SETTINGS_MENU_HISTORY_ITEM_ADJUST_TIME: st_dataSource->adjustTime();     goto reshowHistory;
          return true;
      }
      break;

    case SETTINGS_MENU_ITEM_DISPLAY:
      st_britenessMenu.showMenu(NULL);
      st_brightnessAction(st_brightnessDataSource.alternateCurrentItem, button);
      break;

    case SETTINGS_MENU_ITEM_GAUGES:
      // Replace unit subtitle with alternate units
      st_gaugesTitles2[SETTINGS_MENU_GAUGES_ITEM_UNITS] = st_dataSource->getCurrentItemAltUnits();

      // Setup submenu for showing hidden gauges
      st_showHiddenDataSource.alternateCurrentItem = 0;
      st_showHiddenDataSource.alternateTitles1 = st_dataSource->getHiddenItemTitles();
      st_showHiddenDataSource.alternateTitles2 = st_dataSource->getHiddenItemUnits();
      for (st_showHiddenDataSource.itemCount = 0; st_showHiddenDataSource.alternateTitles1[st_showHiddenDataSource.itemCount]; st_showHiddenDataSource.itemCount++) {}; 
      st_showHiddenMenu.setup(&st_showHiddenDataSource, st_display, st_controls);

      st_gaugesDataSource.alternateCurrentItem = 0;
      switch (st_gaugesMenu.showMenu(NULL)) {
        case SETTINGS_MENU_GAUGES_ITEM_BACK:      return true;
        case SETTINGS_MENU_GAUGES_ITEM_UNITS:     st_dataSource->toggleCurrentItemUnits();      break;
        case SETTINGS_MENU_GAUGES_ITEM_UNITS_SI:  st_dataSource->setAllCurrentItemUnits(false); break;
        case SETTINGS_MENU_GAUGES_ITEM_UNITS_US:  st_dataSource->setAllCurrentItemUnits(true);  break;
        case SETTINGS_MENU_GAUGES_ITEM_HIDE:      st_dataSource->hideCurrentItem();             break;
        case SETTINGS_MENU_GAUGES_ITEM_AUTO_SCAN: st_dataSource->autoScanItems();               break;
        case SETTINGS_MENU_GAUGES_ITEM_INFO:      st_dataSource->showCurrentItemDetails();      return true;
        case SETTINGS_MENU_GAUGES_ITEM_SHOW:      
          st_showHiddenMenu.showMenu(NULL);
          if (st_showHiddenDataSource.alternateCurrentItem) {
            st_dataSource->showItemByName(st_showHiddenDataSource.alternateTitles1[st_showHiddenDataSource.alternateCurrentItem]);
          }
          break;
      }
      break;

    case SETTINGS_MENU_ITEM_CODES:
      st_codesDataSource.alternateCurrentItem = 0;
      switch (st_codesMenu.showMenu(NULL)) {
        case SETTINGS_MENU_CODES_ITEM_BACK:         return true;
        case SETTINGS_MENU_CODES_ITEM_DTC_MONITORS: st_dataSource->showDtcMonitors(); return true;
        case SETTINGS_MENU_CODES_ITEM_DTC_CODE:     st_dataSource->showDtcCodes();    return true;
        case SETTINGS_MENU_CODES_ITEM_DTC_CLEAR:    st_dataSource->clearDtcCodes();   return true;
      }
      break;

reshowValues:
      st_display->showItemTitle("Done");
      delay(1000);
    case SETTINGS_MENU_ITEM_VALUES:
      st_valuesDataSource.alternateCurrentItem = 0;
      switch (st_valuesMenu.showMenu(NULL)) {
        case SETTINGS_MENU_VALUES_ITEM_BACK:          return true;
        case SETTINGS_MENU_VALUES_ITEM_FUEL_ADJUST:   st_dataSource->setFuelAdjustment(); goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_WEIGHT_ADJUST: st_dataSource->setKgWeight();       goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_DETECT:   st_dataSource->startGearDetect();   return true;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_SHOW:     st_dataSource->showGears();         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_ADD:      st_dataSource->addGear();           goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_REMOVE:   st_dataSource->removeGear();        goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_1:        st_dataSource->editGear(0);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_2:        st_dataSource->editGear(1);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_3:        st_dataSource->editGear(2);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_4:        st_dataSource->editGear(3);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_5:        st_dataSource->editGear(4);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_6:        st_dataSource->editGear(5);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_7:        st_dataSource->editGear(6);         goto reshowValues;
        case SETTINGS_MENU_VALUES_ITEM_GEAR_8:        st_dataSource->editGear(7);         goto reshowValues;
      }
      break;

    case SETTINGS_MENU_ITEM_MODES:
      st_modesDataSource.alternateCurrentItem = 0;
      switch (st_modesMenu.showMenu(NULL)) {
        case SETTINGS_MENU_MODES_ITEM_BACK:       return true;
        case SETTINGS_MENU_MODES_ITEM_LOOP_MODE:  st_dataSource->toggleLoopMode();  break;
        case SETTINGS_MENU_MODES_ITEM_DEMO_MODE:  st_dataSource->toggleDemoMode();  break;
        case SETTINGS_MENU_MODES_ITEM_DEBUG_MODE: st_dataSource->toggleDebugMode(); break;
        case SETTINGS_MENU_MODES_ITEM_SNIFF_MODE: st_dataSource->enterSniffMode(0); break;
      }
      break;
  }
  st_display->showItemTitle("Done");
  delay(1000);
  return true;
}

static VMenu st_vmenu;

//------------------------------------------------------
// Public
//------------------------------------------------------

extern void VSettings::setup(struct SettingsDataSource *dataSource, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls) {
  st_dataSource = dataSource;
  st_controls = controls;
  st_display = display;
  st_vmenu.setup(&st_menuDataSource, display, controls);
  st_historyMenu.setup(&st_historyDataSource, display, controls);
  st_britenessMenu.setup(&st_brightnessDataSource, display, controls);
  st_gaugesMenu.setup(&st_gaugesDataSource, display, controls);
  st_codesMenu.setup(&st_codesDataSource, display, controls);
  st_valuesMenu.setup(&st_valuesDataSource, display, controls);
  st_modesMenu.setup(&st_modesDataSource, display, controls);
}

extern void VSettings::showMenu(void) {
  st_menuDataSource.alternateCurrentItem = 0;
  st_vmenu.showMenu("MenU");
} 
