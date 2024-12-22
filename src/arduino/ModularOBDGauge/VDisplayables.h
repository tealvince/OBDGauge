#ifndef _VDISPLAYABLES
#define _VDISPLAYABLES

#include "VMenu.h"

///////////////////////////////////////////////////////////////
// DISPLAYABLES
// Displayable OBD Gauges
///////////////////////////////////////////////////////////////

struct DisplayablesOutputProvider {
  int   (*getBarCount)(void);
  int   (*setBarColor)(int index, unsigned char color);
  int   (*showBar)();
  void  (*showFloatValue)(float value, int decimals, char *suffix, bool addPlus);
  void  (*showStatusState)(bool connecting, bool resetting, int errorCount, int connectionErrorCount, int protocolIndex);
  void  (*showStatusString)(char *text);
  void  (*setBrightness)(int brightness);
};

class VDisplayables {
  private:
    int   ds_getDisplayable();
    void  ds_setDisplayable(int index);
    char *ds_getDisplayableTitle1();
    char *ds_getDisplayableTitle2();
    char  ds_getDisplayableColor();
    bool  ds_isDisplayableHidden(int index);
    bool  ds_displayableLongPressAction(int current);

  public:
    void setup(int inPin, int outPin, int powerAnalogPin, struct DisplayablesOutputProvider *output, struct MenuDisplayProvider *display, struct MenuControlsProvider *controls);
    void mainLoop();
    bool updateCurrentItemValue();
    bool showCurrentItem();
    void savePersistedState();
};

#endif
