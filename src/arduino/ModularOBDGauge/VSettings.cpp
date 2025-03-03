#include "VSettings.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VSETTINGS
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
#define SETTINGS_MENU_ITEM_CLEAR          1
#define SETTINGS_MENU_ITEM_BRIGHTNESS     2
#define SETTINGS_MENU_ITEM_UNITS          3
#define SETTINGS_MENU_ITEM_HIDE           4
#define SETTINGS_MENU_ITEM_SHOW           5 
#define SETTINGS_MENU_ITEM_INFO           6
#define SETTINGS_MENU_ITEM_DTC_MONITORS   7
#define SETTINGS_MENU_ITEM_DTC_CODE       8
#define SETTINGS_MENU_ITEM_DTC_CLEAR      9
#define SETTINGS_MENU_ITEM_FUEL_ADJUST    10
#define SETTINGS_MENU_ITEM_DEMO_MODE      11
#define SETTINGS_MENU_ITEM_DEBUG_MODE     12
#define SETTINGS_MENU_ITEM_SNIFF_MODE     13
#define SETTINGS_MENU_ITEM_COUNT          14

static const char *st_titles1[] = { "BACK", "ERAs", "DISP", "UNIt", "HIDE", "SHOW", "SEE",  "CHEK", "READ", "CLR",  "Burn", "Demo", "DBug", "Snif" };
static       char *st_titles2[] = { "BACK", "HISt", "Brte", "TOGL", "gAgE", "gAgE", "Data", "REDY", "CDES", "CDES", "Adj.", "Mode", "Mode", "Mode" };
static const char  st_colors[]  = "bvworgpycnlPNR";

//------------------------------------------------------
// Private (brighness menu support)
//------------------------------------------------------

bool st_brightnessAction(int current, int button);

static const char *st_brightnessTitles[] = { "br 1","br 2","br 3","br 4","br 5" };
static struct MenuDataSource st_brightnessDataSource = {
  5, NULL, NULL, NULL, NULL, NULL, NULL, NULL, st_brightnessAction, 0, st_brightnessTitles, st_brightnessTitles, NULL
};

static struct MenuDataSource st_showHiddenDataSource = {
  1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, "bggggggggggggggggggggggggggggg"
};

static VMenu st_britenessMenu;
static VMenu st_showHiddenMenu;

bool st_brightnessAction(int current, int button) {
  int briteTable[] = { 0, 13, 25, 50, 100 };
  st_dataSource->setBrightness(briteTable[st_brightnessDataSource.alternateCurrentItem]);
  return false;
}

//------------------------------------------------------
// Private (menu support)
//------------------------------------------------------

bool st_isItemHidden(int item) {
  switch(item) {
    case SETTINGS_MENU_ITEM_SHOW:
      return st_showHiddenDataSource.itemCount <= 1;
    case SETTINGS_MENU_ITEM_UNITS:
      return !st_dataSource->isCurrentItemMultiUnit();
    case SETTINGS_MENU_ITEM_INFO:
      return st_dataSource->isCurrentItemComputed();
    default:
      return false;
  }
}

bool st_longPressAction(int current, int button) {
  switch(current) {
    case SETTINGS_MENU_ITEM_BACK:
      return true;
    case SETTINGS_MENU_ITEM_CLEAR:
      st_dataSource->clearHistory();
      break;
    case SETTINGS_MENU_ITEM_SHOW:
      st_showHiddenMenu.showMenu(NULL);
      if (st_showHiddenDataSource.alternateCurrentItem) {
        st_dataSource->showItemByName(st_showHiddenDataSource.alternateTitles1[st_showHiddenDataSource.alternateCurrentItem]);
      }
      break;
    case SETTINGS_MENU_ITEM_HIDE:
      st_dataSource->hideCurrentItem();
      break;
    case SETTINGS_MENU_ITEM_BRIGHTNESS:
      st_britenessMenu.showMenu(NULL);
      st_brightnessAction(st_brightnessDataSource.alternateCurrentItem, button);
      break;
    case SETTINGS_MENU_ITEM_UNITS:
      st_dataSource->toggleCurrentItemUnits();
      break;
    case SETTINGS_MENU_ITEM_INFO:
      st_dataSource->showCurrentItemDetails();
      return true;
    case SETTINGS_MENU_ITEM_DTC_MONITORS:
      st_dataSource->showDtcMonitors();
      return true;
    case SETTINGS_MENU_ITEM_DTC_CODE:
      st_dataSource->showDtcCodes();
      return true;
    case SETTINGS_MENU_ITEM_DTC_CLEAR:
      st_dataSource->clearDtcCodes();
      return true;
    case SETTINGS_MENU_ITEM_FUEL_ADJUST:
      st_dataSource->setFuelAdjustment();
      break;
    case SETTINGS_MENU_ITEM_DEMO_MODE:
      st_dataSource->toggleDemoMode();
      break;
    case SETTINGS_MENU_ITEM_DEBUG_MODE:
      st_dataSource->toggleDebugMode();
      break;
    case SETTINGS_MENU_ITEM_SNIFF_MODE:
      st_dataSource->enterSniffMode(0);
      break;
  }
  st_display->showItemTitle("Done");
  delay(1000);
  return true;
}

static struct MenuDataSource st_menuDataSource = {
  SETTINGS_MENU_ITEM_COUNT, NULL, NULL, NULL, NULL, NULL, st_isItemHidden, st_longPressAction, NULL, 0, st_titles1, st_titles2, st_colors
};

static VMenu st_vmenu;

//------------------------------------------------------
// Public
//------------------------------------------------------

extern void VSettings::setup(struct SettingsDataSource *dataSource, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls) {
  st_dataSource = dataSource;
  st_controls = controls;
  st_display = display;
  st_vmenu.setup(&st_menuDataSource, display, controls);
  st_britenessMenu.setup(&st_brightnessDataSource, display, controls);
}

extern void VSettings::showMenu(void) {
  // Replace unit subtitle with alternate units
  st_titles2[SETTINGS_MENU_ITEM_UNITS] = st_dataSource->getCurrentItemAltUnits();

  // Setup submenu for showing hidden gauges
  st_showHiddenDataSource.alternateCurrentItem = 0;
  st_showHiddenDataSource.alternateTitles1 = st_dataSource->getHiddenItemTitles();
  st_showHiddenDataSource.alternateTitles2 = st_dataSource->getHiddenItemUnits();
  for (st_showHiddenDataSource.itemCount = 0; st_showHiddenDataSource.alternateTitles1[st_showHiddenDataSource.itemCount]; st_showHiddenDataSource.itemCount++) {}; 
  st_showHiddenMenu.setup(&st_showHiddenDataSource, st_display, st_controls);

  st_menuDataSource.alternateCurrentItem = 0;
  st_vmenu.showMenu("MenU");
} 
