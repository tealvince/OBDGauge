#include "TM1637Display.h"
#include "VDigits.h"

#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VDIGITS
// Simple class to write alphanumeric characters to a TM1637
///////////////////////////////////////////////////////////////

#define MAX_DIGITS 4

//
//    --a--
//   |     |
//   f     b
//   |     |
//    --g--
//   |     |
//   e     c
//   |     |
//    --d--   h.
//

#ifndef SEG_DOT
#define SEG_DOT (SEG_G << 1)
#endif

#define TABLE_SIZE 128

static const uint8_t dg_font[TABLE_SIZE] PROGMEM = {
  0,
  /* degree */ SEG_A | SEG_B | SEG_F | SEG_G,
      0,0,0,  0,0,0,0,0,  0,0,0,0,0,
  0,0,0,0,0,  0,0,0,0,0,  0,0,0,0,0,
  0,0,0,

  /* ! */ SEG_A | SEG_B | SEG_D | SEG_F | SEG_G,
  /* " */ SEG_B | SEG_F,
  /* # */ 0,
  /* $ */ 0,
  /* % */ 0,
  /* & */ 0,
  /* ' */ SEG_F,
  /* ( */ SEG_A | SEG_D | SEG_E | SEG_F,
  /* ) */ SEG_A | SEG_B | SEG_C | SEG_D,
  /* * */ 0,
  /* + */ SEG_B | SEG_C | SEG_G, 
  /* , */ SEG_C | SEG_D,
  /* - */ SEG_G,
  /* . */ SEG_DOT,
  /* / */ SEG_B | SEG_E | SEG_G, 
  /* 0 */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  /* 1 */ SEG_E | SEG_F,
  /* 2 */ SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
  /* 3 */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
  /* 4 */ SEG_B | SEG_C | SEG_F | SEG_G,
  /* 5 */ SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
  /* 6 */ SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* 7 */ SEG_A | SEG_B | SEG_C,
  /* 8 */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* 9 */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
  /* : */ SEG_D | SEG_G,
  /* ; */ SEG_B | SEG_D,
  /* < */ SEG_A | SEG_D | SEG_E | SEG_F,
  /* = */ SEG_D | SEG_G,
  /* > */ SEG_A | SEG_B | SEG_C | SEG_D,
  /* ? */ SEG_A | SEG_B | SEG_E | SEG_G,
  /* @ */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
  /* A */ SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
  /* b */ SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* C */ SEG_A | SEG_D | SEG_E | SEG_F,
  /* d */ SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
  /* E */ SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
  /* F */ SEG_A | SEG_E | SEG_F | SEG_G,
  /* G */ SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* H */ SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
  /* I */ SEG_E | SEG_F,
  /* J */ SEG_B | SEG_C | SEG_D | SEG_E,
  /* K */ SEG_B | SEG_DOT | SEG_E | SEG_F | SEG_G,
  /* L */ SEG_D | SEG_E | SEG_F,
  /* 3 */ SEG_A | SEG_D | SEG_G,
  /* n */ SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,
  /* O */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  /* P */ SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
  /* q */ SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,
  /* r */ SEG_A | SEG_E | SEG_F,
  /* S */ SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
  /* 7 */ SEG_A | SEG_B | SEG_C,
  /* U */ SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  /* V */ SEG_B | SEG_D | SEG_F | SEG_G,
  /* ū */ SEG_A | SEG_C | SEG_D | SEG_E,
  /* X */ SEG_B | SEG_C | SEG_E | SEG_F,
  /* y */ SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
  /* 2 */ SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
  /* [ */ SEG_A | SEG_D | SEG_E | SEG_F,
  /* \ */ SEG_C | SEG_F | SEG_G,
  /* ] */ SEG_A | SEG_B | SEG_C | SEG_D,
  /* ^ */ SEG_A | SEG_B | SEG_F,
  /* _ */ SEG_D,
  /* ` */ SEG_F,
  /* a */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
  /* b */ SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* c */ SEG_D | SEG_E | SEG_G,
  /* d */ SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
  /* E */ SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
  /* F */ SEG_A | SEG_E | SEG_F | SEG_G,
  /* g */ SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
  /* h */ SEG_C | SEG_E | SEG_F | SEG_G,
  /* i */ SEG_E,
  /* J */ SEG_B | SEG_C | SEG_D,
  /* K */ SEG_B | SEG_DOT | SEG_E | SEG_F | SEG_G,
  /* l */ SEG_E | SEG_F,
  /* 3 */ SEG_A | SEG_D | SEG_G,
  /* n */ SEG_C | SEG_E | SEG_G,
  /* o */ SEG_C | SEG_D | SEG_E | SEG_G,
  /* P */ SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
  /* q */ SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,
  /* r */ SEG_E | SEG_G,
  /* S */ SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
  /* t */ SEG_D | SEG_E | SEG_F | SEG_G,
  /* u */ SEG_C | SEG_D | SEG_E,
  /* V */ SEG_B | SEG_D | SEG_F | SEG_G,
  /* ū */ SEG_A | SEG_C | SEG_D | SEG_E,
  /* x */ SEG_C | SEG_E,
  /* y */ SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
  /* 2 */ SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,

  /* { */ SEG_A | SEG_D | SEG_E | SEG_F,
  /* | */ SEG_E | SEG_F,
  /* } */ SEG_A | SEG_B | SEG_C | SEG_D,
  /* ~ */ 0,
};

static TM1637Display *dg_display;

extern void VDigits::setup(int clockPin, int dataPin) {
    dg_display = new TM1637Display(clockPin, dataPin);
    dg_display->setBrightness(7, true);
    dg_display->setSegments("\xff\xff\xff\xff", 4, 0);
    delay(500);
    dg_display->clear();
    delay(500);
}

extern void VDigits::showChar(int pos, unsigned char c, bool addDot) {
    uint8_t segment = pgm_read_byte_near(dg_font + c);
    if (addDot) {
      segment |= SEG_DOT;
    }
    dg_display->setSegments(&segment, 1, pos);
}

extern void VDigits::showString(unsigned char *text, bool addDots) {
  for (int i=0; i<MAX_DIGITS; i++) {
    unsigned char c = *text;
    if (c) {
      text++;
    }

    if (c<TABLE_SIZE) {
      unsigned char font = pgm_read_byte_near(dg_font+c);
      if (addDots && c != '.' && *text == '.') {
        text++;
        font |= pgm_read_byte_near(dg_font+'.');
      }
      dg_display->setSegments(&font, 1, i);
    }
  }
}

extern void VDigits::setBrightness(int brite) {
  dg_display->setBrightness(brite);
}


