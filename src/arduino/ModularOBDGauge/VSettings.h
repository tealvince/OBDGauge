#ifndef _VSETTINGS
#define _VSETTINGS

#include "VMenu.h"

///////////////////////////////////////////////////////////////
// VSETTINGS
// Settings menu
///////////////////////////////////////////////////////////////

struct SettingsDataSource {
  void (*clearHistory)(void);
  void (*setBrightness)(int brightness);
  void (*toggleCurrentItemUnits)(void);
  void (*hideCurrentItem)(void);
  void (*showItemByName)(char *name);
  void (*showCurrentItemDetails)(void);
  void (*showDtcMonitors)(void);
  void (*showDtcCodes)(void);
  void (*clearDtcCodes)(void);
  void (*setFuelAdjustment)(void);

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
    void setup(struct SettingsDataSource *dataSource, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls);
    void showMenu();
};

#endif
