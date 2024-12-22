#include "VMenu.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// MENU
// Generalized menu handling
///////////////////////////////////////////////////////////////

//------------------------------------------------------
// Private
//------------------------------------------------------
int   mn_defaultGetCurrentItem      (MenuDataSource *ds) { return ds->alternateCurrentItem; }
void  mn_defaultSetCurrentItem      (int index, MenuDataSource *ds) { ds->alternateCurrentItem = index; }
char *mn_defaultGetCurrentItemTitle1(MenuDataSource *ds) { return ds->alternateTitles1 ? ds->alternateTitles1[ds->getCurrentItem(ds)] : ""; }
char *mn_defaultGetCurrentItemTitle2(MenuDataSource *ds) { return ds->alternateTitles2 ? ds->alternateTitles2[ds->getCurrentItem(ds)] : ""; }
char  mn_defaultGetCurrentItemColor (MenuDataSource *ds) { return ds->alternateColors ? ds->alternateColors[ds->getCurrentItem(ds)] : 'w'; }
bool  mn_defaultIsItemHidden        (int index, MenuDataSource *ds) { return false; }
bool  mn_defaultLongPressAction     (int index, MenuDataSource *ds) { return true; }
 
int mn_showTitles(char *title1, char *title2, struct MenuDisplayProvider *display, struct MenuControlsProvider *optionalControls) {
    bool b1;
    display->showItemTitle(title1);
    long time = millis();
    while (millis() < time + 1000) {
      if (optionalControls && ((b1 = optionalControls->isButton1Down()) || optionalControls->isButton2Down())) return b1 ? 1 : -1;
    }
    display->showItemTitle(title2);
    while (millis() < time + 1750) {
      if (optionalControls && ((b1 = optionalControls->isButton1Down()) || optionalControls->isButton2Down())) return b1 ? 1 : -1;
    }
    return false;
}

int mn_getVisibleItemCount(struct MenuDataSource *dataSource) {
  int result = 0;
  // Count number of visible items
  for (int i = 0; i < dataSource->itemCount; i++) {
    if (!dataSource->isItemHidden || !dataSource->isItemHidden(i, dataSource)) {
      result++;
    }
  }
  return result;
} 

int mn_getCurrentVisibleItem(struct MenuDataSource *dataSource) {
  int result = 0;
  // Count number of non-hidden items before us
  for (int i = 0; i < dataSource->getCurrentItem(dataSource); i++) {
    if (!dataSource->isItemHidden || !dataSource->isItemHidden(i, dataSource)) {
      result++;
    }
  }
  return result;
} 

void mn_setCurrentVisibleItem(struct MenuDataSource *dataSource, int current) {
  // Set index to n'th visible item
  for (int i = 0; i < dataSource->itemCount; i++) {
    if (!dataSource->isItemHidden || !dataSource->isItemHidden(i, dataSource)) {
      if (!current--) {
        dataSource->setCurrentItem(i, dataSource);
        return;
      }
    }
  }

  // Else set first visible one
  for (int i = 0; i < dataSource->itemCount; i++) {
    if (!dataSource->isItemHidden || !dataSource->isItemHidden(i, dataSource)) {
      dataSource->setCurrentItem(i, dataSource);
      return;
    }
  }
} 

//------------------------------------------------------
// Public
//------------------------------------------------------

extern void VMenu::setup(struct MenuDataSource *data, struct MenuDisplayProvider *disp, struct MenuControlsProvider *ctl) {
  dataSource = data;
  display = disp;
  controls = ctl;

  if (!dataSource->getCurrentItem) dataSource->getCurrentItem = mn_defaultGetCurrentItem;
  if (!dataSource->setCurrentItem) dataSource->setCurrentItem = mn_defaultSetCurrentItem;
  if (!dataSource->getCurrentItemTitle1) dataSource->getCurrentItemTitle1 = mn_defaultGetCurrentItemTitle1;
  if (!dataSource->getCurrentItemTitle2) dataSource->getCurrentItemTitle2 = mn_defaultGetCurrentItemTitle2;
  if (!dataSource->getCurrentItemColor) dataSource->getCurrentItemColor = mn_defaultGetCurrentItemColor;
  if (!dataSource->isItemHidden) dataSource->isItemHidden = mn_defaultIsItemHidden;
  if (!dataSource->longPressAction) dataSource->longPressAction = mn_defaultLongPressAction;
  if (!dataSource->shortPressAction) dataSource->shortPressAction = mn_defaultLongPressAction;
}

extern void VMenu::showMenu(char *optionalTitle) {
  if (optionalTitle) {
    display->showItemTitle(optionalTitle);
    controls->smartDelay(1000);
  }
  while (!mainLoop(true)) {}
  while ( (controls->isButton1Down() || controls->isButton2Down())) {}
} 

extern bool VMenu::mainLoop(bool showCurrent) {
  bool button1Down = controls->isButton1Down();
  bool button2Down = controls->isButton2Down();
  bool buttonDown = button1Down || button2Down;
  int advance = button1Down ? 1 : button2Down ? -1 : 0;
  int itemCount = mn_getVisibleItemCount(dataSource);

  if (itemCount) while (advance || showCurrent) {
    // Update current index
    int current = mn_getCurrentVisibleItem(dataSource);

    // Show light for old item
    display->highlightItem(current, dataSource->getCurrentItemColor(dataSource), itemCount);
    if (advance) {
      display->highlightItem(current, 'k', itemCount);
      controls->smartDelay(100);
      display->highlightItem(current, dataSource->getCurrentItemColor(dataSource), itemCount);
    }

    // Wait for button up
    long start = millis();
    if (advance) while (1) {
      button1Down = controls->isButton1Down();
      button2Down = controls->isButton2Down();
      if (!button1Down && !button2Down) break;
      advance = button1Down ? 1 : button2Down ? -1 : 0;

      // Handle long press
      if (millis() > start + 500) {
 
        // Wait for button up flashing menu item
        while ((controls->isButton1Down() || controls->isButton2Down()) && millis() < start + 1500) {
          display->highlightItem(current, millis() > start + 1000 || ((millis()-start)&128) ? dataSource->getCurrentItemColor(dataSource) : 'k', itemCount);
        }

        // Handle super long press (backup, button 1 only)
        if (controls->isButton1Down()) {
          current = (current + itemCount- 1) % itemCount;
          mn_setCurrentVisibleItem(dataSource, current);
          display->highlightItem(current, dataSource->getCurrentItemColor(dataSource), itemCount);
          advance = 0;

          while (controls->isButton1Down() || controls->isButton2Down()) {}
          controls->smartDelay(10);
          break;
        }

        // Convert visible index to raw index
        for (int i = 0; i < dataSource->itemCount; i++) {
          if (!dataSource->isItemHidden || !dataSource->isItemHidden(i, dataSource)) {
            if (!current--) {
              return dataSource->longPressAction(i, button2Down ? 1 : 0, dataSource);
            }
          }
        }
        return false;
      }
    }
    controls->smartDelay(10);

    // Update current
    if (advance) {
      current = (current + advance + itemCount) % itemCount;
      mn_setCurrentVisibleItem(dataSource, current);
    }
    dataSource->shortPressAction(current, button2Down ? 1 : 0, dataSource);

    // Show light and titles for new item
    advance = showCurrentItem(true);

    // Clear ring
    display->highlightItem(current, 'k', itemCount);

    showCurrent = false;
  }
  return false;
}

extern int VMenu::showCurrentItem(bool abortable) {
  int current = mn_getCurrentVisibleItem(dataSource);

  display->highlightItem(current, dataSource->getCurrentItemColor(dataSource), mn_getVisibleItemCount(dataSource));
  return mn_showTitles(dataSource->getCurrentItemTitle1(dataSource), dataSource->getCurrentItemTitle2(dataSource), display, abortable ? controls : NULL);
}

extern void VMenu::highlightCurrentItem() {
  int current = mn_getCurrentVisibleItem(dataSource);

  display->highlightItem(current, dataSource->getCurrentItemColor(dataSource), mn_getVisibleItemCount(dataSource));
}
