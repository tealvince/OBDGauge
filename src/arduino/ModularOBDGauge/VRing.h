///////////////////////////////////////////////////////////////
// VRING
// Simple class to write colors to an Adafruit_NeoPixel
///////////////////////////////////////////////////////////////

#ifndef _VRING
#define _VRING

#define VRING_BROWN   'n'
#define VRING_RED     'r'
#define VRING_ORANGE  'o'
#define VRING_YELLOW  'y'
#define VRING_GREEN   'g'
#define VRING_CYAN    'c'
#define VRING_BLUE.   'b'
#define VRING_INDIGO  'i'
#define VRING_VIOLET  'v'
#define VRING_PURPLE  'p'
#define VRING_BLACK   'k'
#define VRING_WHITE   'w'

#define VRING_DIM_BROWN   'N'
#define VRING_DIM_RED     'R'
#define VRING_DIM_ORANGE  'O'
#define VRING_DIM_YELLOW  'Y'
#define VRING_DIM_GREEN   'G'
#define VRING_DIM_CYAN    'C'
#define VRING_DIM_BLUE.   'B'
#define VRING_DIM_INDIGO  'I'
#define VRING_DIM_VIOLET  'V'
#define VRING_DIM_PURPLE  'P'
#define VRING_DIM_BLACK   'K'
#define VRING_DIM_WHITE   'W'

#define VRING_DIM_GRAY   VRING_DIM_WHITE

class VRing {
  public:
    void setup(int pin, int count, int brightness, int rotationOffset);
    int  getBrightness();
    void setBrightness(int brightness);
    void setPixelColor(int i, char color);
    void show();
    void showDemo();
    void showColors(bool dim);
    void clear();
};

#endif
