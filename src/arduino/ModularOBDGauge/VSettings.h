#ifndef _VSETTINGS
#define _VSETTINGS

#include "VMenu.h"

///////////////////////////////////////////////////////////////
// VSETTINGS.H
// Settings menu
///////////////////////////////////////////////////////////////

struct SettingsDataSource {
  void (*clearHistory)(void);
  void (*setBrightness)(int brightness);
  void (*toggleCurrentItemUnits)(void);
  void (*autoScanItems)(void);
  void (*hideCurrentItem)(void);
  void (*showItemByName)(char *name);
  void (*showCurrentItemDetails)(void);
  void (*showDtcMonitors)(void);
  void (*showDtcCodes)(void);
  void (*clearDtcCodes)(void);
  void (*setFuelAdjustment)(void);
  void (*toggleLoopMode)(void);
  void (*toggleDemoMode)(void);
  void (*toggleDebugMode)(void);
  void (*enterSniffMode)(int mode);

  bool (*isCurrentItemHidden)(void);
  bool (*isCurrentItemMultiUnit)(void);
  bool (*isCurrentItemComputed)(void);
  char *(*getCurrentItemAltUnits)(void);

  char **(*getHiddenItemTitles)(void); // null terminated list
  char **(*getHiddenItemUnits)(void);  // null terminated list
};

class VSettings {
  private:

  public:
    bool demoModeEnabled;
    bool debugModeEnabled;
    void setup(struct SettingsDataSource *dataSource, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls);
    void showMenu();
};

#endif
