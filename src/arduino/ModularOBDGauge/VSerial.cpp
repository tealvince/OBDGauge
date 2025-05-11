#include "VSerial.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VSERIAL.CPP
// Software-driven serial port
///////////////////////////////////////////////////////////////

#define TIME_AFTER(_t1,_t2)   ((long)((_t1) - (_t2)) > 0)

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

extern int VSerial::readBytes(unsigned char **byteBuf, unsigned long timeoutMs, unsigned long inactivityTimeoutMs, unsigned long baud, unsigned long *minByteSpacing, unsigned long *maxByteSpacing) {
  int flipCount = readFlips(flips, SERIAL_MAX_FLIPS, timeoutMs, inactivityTimeoutMs);
  int byteCount = decodeFlipsToBytes(bytes, SERIAL_MAX_BYTES, flips, flipCount, baud, minByteSpacing, maxByteSpacing);

  *byteBuf = bytes;
  return byteCount;
}

extern int VSerial::readFlipIntervals(unsigned long **flipBuf, unsigned long timeoutMs, unsigned long inactivityTimeoutMs) {
  int flipCount = readFlips(flips, SERIAL_MAX_FLIPS, timeoutMs, inactivityTimeoutMs);
  for (int i = flipCount-2; i >= 0; i--) {
    flips[i+1] -= flips[i];
  }
  flips[0] = 0;
  *flipBuf = flips;
  return flipCount;
}

//#define US_BIT_OFFSET(_byte,_bit,_baud,_byteMsDelay)  (1000000L * ((_byte) * 10L + (_bit))/(_baud) + (_byte)*(_byteMsDelay)*1000L);  // start bit

extern void VSerial::sendBytes(unsigned char *bytes, int count, unsigned long baud, int msDelayBetweenBytes) {
  for (int i=0; i<count; i++) {
     sendByte(*bytes++, baud);
    if (msDelayBetweenBytes && i<count-1) {
      delayUntil(micros() + msDelayBetweenBytes * 1000L);
    }
  }
}

extern void VSerial::sendByte(unsigned char val, unsigned long baud) {
    unsigned long totalPulses = 0;
    unsigned int value = (val << 1) | 0x200;  // Add start bit and stop bit
    unsigned long startTime = micros() - 3;   // Adjust time for entering loop, verified by logic analyzer

    for (int bit=0; bit<10; bit++) {
      sendBit(value&1, startTime + (1000000L * (++totalPulses))/baud); // data bits
      value >>= 1;
    }
}

extern void VSerial::sendByteRepeatedly(unsigned char byte, int count, unsigned long baud, int msDelayBetweenBytes) {
  for (int i=0; i<count; i++) {
    sendByte(byte, baud);
  }   
}

extern void VSerial::sendBit(int val, long waitUs) {
    digitalWrite(outPin, val);
    delayUntil(waitUs);
}

extern void VSerial::waitForIdle(unsigned long idleMs, unsigned long timeoutMs) {
  unsigned long endTime = micros() + timeoutMs * 1000L;
  while(1) {
    if (readFlips(flips, 1, idleMs, 0) == 0 || TIME_AFTER(micros(), endTime)) {
      return;
    }
  }
}

//------------------------------------------------------
// Private
//------------------------------------------------------

int VSerial::readFlips(long *buffer, int buflen, long messageTimeoutMs, long byteTimeoutMs) {
  unsigned long startTime = micros();
  unsigned long endTime = startTime + messageTimeoutMs * 1000L;
  unsigned long endTime2 = startTime + byteTimeoutMs * 1000L;
  int last = digitalRead(inPin);
  int index = 0;
  bool foundLow = false;

  // Listen for any state changes
  while(1) {
    unsigned long time = micros();

    if ((TIME_AFTER(time, endTime) && index == 0) || (index > 0 && TIME_AFTER(time, endTime2))) return index;

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
      endTime2 = time + byteTimeoutMs * 1000L;
    }
  }
}

int VSerial::decodeFlipsToBytes(unsigned char *bytes, int bytesLen, unsigned long *flips, int flipCount, long baud, unsigned long *minByteSpacing, unsigned long *maxByteSpacing) {
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

    // Get spacing for first few bytes (in outgoing request if sniffing packets)
    if (startFlip < flipCount && index < 5) {
      unsigned long byteSpacing = flips[startFlip] - flips[startFlip-1];
      if (minByteSpacing && (index == 1 || byteSpacing < *minByteSpacing)) *minByteSpacing = byteSpacing;
      if (maxByteSpacing && (index == 1 || byteSpacing > *maxByteSpacing)) *maxByteSpacing = byteSpacing;
    }
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
      if (TIME_AFTER(time, waitUs)) {
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
