#include "Environment.h"
#include "VRing.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VRING.CPP
// Simple class to write colors to an Adafruit_NeoPixel
///////////////////////////////////////////////////////////////

#define RING_FLAGS  NEO_RGB + NEO_KHZ800

static Adafruit_NeoPixel ring;
static int rn_brightness;
static int rn_rotationOffset;

extern void VRing::setup(int pin, int count, int brightness, int rotationOffset) {
  ring = Adafruit_NeoPixel(count, pin, RING_FLAGS);
  ring.begin();

  rn_rotationOffset = rotationOffset;

  for (int i = 0; i < count; i++) {
    setPixelColor(i, 'k');
  }

  ring.show();            

  setBrightness(brightness);
}

extern void VRing::setBrightness(int brightness) {
  rn_brightness = brightness;
  ring.setBrightness(brightness*6);
}

extern void VRing::setPixelColor(int i, char color) {
#if WOKWI
  float dim = 1.2;
#else
  int dim = 4;
#endif
  i = (i+rn_rotationOffset) % ring.numPixels();

  switch (color) {               // g     r     b
    case 'r': ring.setPixelColor(i, 0x00, 0xff, 0x00, 0); break;
    case 'n': ring.setPixelColor(i, 0x0d, 0x50, 0x02, 0); break;
    case 'o': ring.setPixelColor(i, 0x50, 0xff, 0x00, 0); break;
    case 'y': ring.setPixelColor(i, 0x77, 0x77, 0x10, 0); break;
    case 'l': ring.setPixelColor(i, 0xaa, 0x44, 0x00, 0); break;
    case 'g': ring.setPixelColor(i, 0xff, 0x00, 0x00, 0); break;
    case 'c': ring.setPixelColor(i, 0xcc, 0x00, 0x50, 0); break;
    case 'b': ring.setPixelColor(i, 0x20, 0x00, 0xff, 0); break;
    case 'i': ring.setPixelColor(i, 0x00, 0x00, 0xff, 0); break;
    case 'v': ring.setPixelColor(i, 0x00, 0xff, 0x80, 0); break;
    case 'p': ring.setPixelColor(i, 0x00, 0x20, 0xaa, 0); break;
    case 'k': ring.setPixelColor(i, 0x00, 0x00, 0x00, 0); break;

    case 'R': ring.setPixelColor(i, 0x00/dim, 0xaa/dim, 0x00/dim, 0); break;
    case 'N': ring.setPixelColor(i, 0x20/dim, 0x50/dim, 0x02/dim, 0); break;
    case 'O': ring.setPixelColor(i, 0x50/dim, 0xff/dim, 0x00/dim, 0); break;
    case 'Y': ring.setPixelColor(i, 0x77/dim, 0x77/dim, 0x10/dim, 0); break;
    case 'L': ring.setPixelColor(i, 0xaa/dim, 0x44/dim, 0x00/dim, 0); break;
    case 'G': ring.setPixelColor(i, 0xff/dim, 0x00/dim, 0x00/dim, 0); break;
    case 'C': ring.setPixelColor(i, 0xcc/dim, 0x00/dim, 0x50/dim, 0); break;
    case 'B': ring.setPixelColor(i, 0x40/dim, 0x00/dim, 0xff/dim, 0); break;
    case 'I': ring.setPixelColor(i, 0x00/dim, 0x00/dim, 0xff/dim, 0); break;
    case 'V': ring.setPixelColor(i, 0x00/dim, 0xff/dim, 0x80/dim, 0); break;
    case 'P': ring.setPixelColor(i, 0x00/dim, 0x40/dim, 0xaa/dim, 0); break;
    case 'K': ring.setPixelColor(i, 0x00/dim, 0x00/dim, 0x00/dim, 0); break;

    default:
    case 'w': ring.setPixelColor(i, 0x88, 0x88, 0x88, 0); break;
    case 'W': ring.setPixelColor(i, 0x33, 0x33, 0x33, 0); break;
  }
 }

extern void VRing::show() {
  ring.show();
}

extern void VRing::showDemo() {
  int num = ring.numPixels();

  for (int i=0; i<num; i++) {
    setPixelColor(i, i < 3 ? 'k' : (i & 1) ? 'g' : 'c');
  }
  ring.show();            
}

extern void VRing::clear() {
  for (int i=0; i<ring.numPixels(); i++) {
    setPixelColor(i, 'k');
  }
}
