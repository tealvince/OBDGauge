///////////////////////////////////////////////////////////////
// VDIGITS.H
// Simple class to write alphanumeric characters to a TM1637
///////////////////////////////////////////////////////////////

#ifndef _VDIGITS
#define _VDIGITS

class VDigits {
  public:
    void setup(int clockPin, int dataPin);
    void showChar(int pos, unsigned char c, bool addDot);
    void showString(unsigned char *text, bool addDots);
    void setBrightness(int);
};

#endif
