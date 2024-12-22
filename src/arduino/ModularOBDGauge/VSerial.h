///////////////////////////////////////////////////////////////
// VSERIAL
// Software-driven serial port
///////////////////////////////////////////////////////////////

#ifndef _VSERIAL
#define _VSERIAL

// TODO: Revisit to make these dynamic
#define SERIAL_MAX_FLIPS 100
#define SERIAL_MAX_BYTES 10

class VSerial {
  private:
    int inPin, outPin;
    unsigned long flips[SERIAL_MAX_FLIPS];
    unsigned char bytes[SERIAL_MAX_BYTES];

    int  readFlips(long *buffer, int buflen, long startTimeoutMs, long inactivityTimeoutMs);
    int  decodeFlipsToBytes(unsigned char *bytes, int bytesLen, unsigned long *flips, int flipCount, long baud);
    int  decodeFlippedValueAtTime(unsigned long time, unsigned long *flips, int flipCount);
    void delayUntil(unsigned long waitUs);

  public:
    void setup(int inPin, int outPin, void (*smartDelay)(unsigned long));

    int  readBytes(unsigned char **byteBuf, unsigned long timeoutMs, unsigned long baud);
    void sendBytes(unsigned char *bytes, int count, unsigned long baud);
    void sendByte(unsigned char val, unsigned long baud, unsigned long startTime, int bytesSoFar);
    void sendByteRepeatedly(unsigned char byte, int count, unsigned long baud);
    void sendBit(int val, long waitUs);
};

#endif