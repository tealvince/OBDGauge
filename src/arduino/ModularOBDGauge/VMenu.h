///////////////////////////////////////////////////////////////
// VMENU
// Generalized menu handling
///////////////////////////////////////////////////////////////

#ifndef _VMENU
#define _VMENU

struct MenuDataSource {
  int   itemCount;
  int   (*getCurrentItem)(MenuDataSource *);              // optional
  void  (*setCurrentItem)(int index, MenuDataSource *);   // optional
  char *(*getCurrentItemTitle1)(MenuDataSource *);        // optional
  char *(*getCurrentItemTitle2)(MenuDataSource *);        // optional
  char  (*getCurrentItemColor)(MenuDataSource *);         // optional
  bool  (*isItemHidden)(int index, MenuDataSource *);     // optional
  bool  (*longPressAction)(int index, int button, MenuDataSource *);  // optional
  bool  (*shortPressAction)(int index, int button, MenuDataSource *);  // optional
  int alternateCurrentItem;
  char **alternateTitles1;
  char **alternateTitles2;
  char *alternateColors;
};

struct MenuDisplayProvider {
  void  (*highlightItem)(int index, char color, int count);
  void  (*showItemTitle)(char *title);
};

struct MenuControlsProvider {
  bool  (*isButton1Down)(void);
  bool  (*isButton2Down)(void);
  void  (*smartDelay)(unsigned long waitMs);
};

class VMenu {  
  private:
    struct MenuDataSource *dataSource;
    struct MenuDisplayProvider *display;
    struct MenuControlsProvider *controls;

  public:
    void setup(struct MenuDataSource *data, struct MenuDisplayProvider *disp, struct MenuControlsProvider *ctl);
    void showMenu(char *optionalTitle);
    bool mainLoop(bool showCurrent);
    int  showCurrentItem(bool abortable);
    void highlightCurrentItem();
};

#endif
