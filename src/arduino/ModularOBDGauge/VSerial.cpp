#include "VSerial.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VSERIAL
// Software-driven serial port
///////////////////////////////////////////////////////////////

//------------------------------------------------------
// Public
//------------------------------------------------------

void (*ser_smartDelay)(unsigned long);

extern void VSerial::setup(int in, int out, void (*smartDelay)(unsigned long)) {
  inPin = in;
  outPin = out;
  ser_smartDelay = smartDelay;

  if (inPin >= 0) pinMode(inPin, INPUT_PULLUP);
  if (outPin >= 0) {
    pinMode(outPin, OUTPUT);
    digitalWrite(outPin, 1);
  }
}

extern int VSerial::readBytes(unsigned char **byteBuf, unsigned long timeoutMs, unsigned long baud) {
  int flipCount = readFlips(flips, SERIAL_MAX_FLIPS, timeoutMs, 250000L/baud);
  int byteCount = decodeFlipsToBytes(bytes, SERIAL_MAX_BYTES, flips, flipCount, baud);

  *byteBuf = bytes;

  return byteCount;
}

extern void VSerial::sendBytes(unsigned char *bytes, int count, unsigned long baud) {
  unsigned long startTime = micros();

  for (int i=0; i<count; i++) {
    sendByte(*bytes++, baud, startTime, i);
  }   
}

extern void VSerial::sendByte(unsigned char val, unsigned long baud, unsigned long startTime, int bytesSoFar) {

    sendBit(0, startTime + (1000000L * (long)(bytesSoFar*10+1))/baud);  // start bit

    for (int bit=0; bit<8; bit++) {
      sendBit((val>>bit)&1, startTime + (1000000L * (long)(bytesSoFar*10+2+bit))/baud);
    }

    sendBit(1, startTime + (1000000L * (long)(bytesSoFar*10+10))/baud);  // stop bit
}

extern void VSerial::sendByteRepeatedly(unsigned char byte, int count, unsigned long baud) {
  unsigned long startTime = micros();

  for (int i=0; i<count; i++) {
    sendByte(byte, baud, startTime, i);
  }   
}

extern void VSerial::sendBit(int val, long waitUs) {
    digitalWrite(outPin, val);
    delayUntil(waitUs);
}

//------------------------------------------------------
// Private
//------------------------------------------------------

int VSerial::readFlips(long *buffer, int buflen, long startTimeoutMs, long inactivityTimeoutMs) {
  unsigned long startTime = micros();
  unsigned long endTime = startTime + startTimeoutMs * 1000L;
  unsigned long endTime2 = startTime + inactivityTimeoutMs * 1000L;
  int last = digitalRead(inPin);
  int index = 0;
  bool foundLow = false;

  // Listen for any state changes
  while(1) {
    unsigned long time = micros();

    if ((time > endTime && index == 0) || (index > 0 && time > endTime2)) return index;

    // Call this to detect interruptions
    ser_smartDelay(0);

    int val = digitalRead(inPin);
    if (val != last) {
      if (index >= buflen) {
        return buflen;
      }
      if (!val) {
        foundLow = true;
      }
      if (foundLow) {
        buffer[index++] = time;
      }
      last = val;
      endTime2 = time + inactivityTimeoutMs * 1000L;
    }
  }
}

int VSerial::decodeFlipsToBytes(unsigned char *bytes, int bytesLen, unsigned long *flips, int flipCount, long baud) {
  int index = 0;
  int startFlip = 0;

  while(1) {
    if (startFlip >= flipCount) return index;
    if (index >= bytesLen) return bytesLen;
    
    unsigned long start = flips[startFlip];

    // read bytes, at 1.5, 2.5, 3.5... 8.5 x period after start time
    bytes[index] = 0;
    for (int i=0; i<=7; i++) {
      bytes[index] |= decodeFlippedValueAtTime(start + (i * 1000000L + 1500000L)/baud, flips+startFlip, flipCount-startFlip) ? (1 << i) : 0;
    }
    index++;

    // find next start bit (first down transition after stop bit)
    while (startFlip < flipCount && flips[startFlip] < start + 9500000L/baud) startFlip += 2;
  }
}

int VSerial::decodeFlippedValueAtTime(unsigned long time, unsigned long *flips, int flipCount) {
  for (int i=0; i<flipCount; i++) {
    if (flips[i] > time) {
      return 1 - (i & 1);
    }
  }
  return -1;
}

void VSerial::delayUntil(unsigned long waitUs) {
    while (1) {
      unsigned long time = micros();
      if (time > waitUs && time < waitUs + 1000000L) {
        return;
      }
    }
}

// extern void VSerial::debugListen(unsigned long timeoutMs, unsigned long baud, void (*debugFlips)(unsigned long *flips, int flipCount), void (*ebugBytes)(unsigned char *bytes, int byteCount)) {
//   unsigned long flips[100];
//   int flipCount = readFlips(flips, 100, timeoutMs, 10000);
//   unsigned char bytes[100];
//   int byteCount = decodeFlipsToBytes(bytes, 100, flips, flipCount, baud);

//   if (debugFlips) debugFlips(flips, flipCount);
//   if (debugBytes) debugBytes(bytes, byteCount);
// }
