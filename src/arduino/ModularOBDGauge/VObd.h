///////////////////////////////////////////////////////////////
// OBD
// Obd protocol handling
///////////////////////////////////////////////////////////////

#ifndef _VOBD
#define _VOBD

#include "VSerial.h"

#define OBD_PROTOCOL_AUTOMATIC 0
#define OBD_PROTOCOL_ISO_9141  1
#define OBD_PROTOCOL_KWP_SLOW  2
#define OBD_PROTOCOL_KWP_FAST  3

#define OBD_PROTOCOL_FIRST     1
#define OBD_PROTOCOL_LAST      3

struct ObdOutputProvider {
  void  (*showStatusString)(char *text);
  void  (*showStatusInteger)(int num);
  void  (*showStatusByte)(int num);
};

class VObd {
  private:
    VSerial vserial;
    int protocol = 0;
    int messageFormat;
    unsigned char keyByte1;
    unsigned char keyByte2;
    unsigned long pidResponseTimeoutMs;
    unsigned long initResponseTimeoutMs;
    struct ObdOutputProvider *output;

    bool kwpSlowInit();
    bool kwpFastInit();
    unsigned char getChecksum(unsigned char *buf, int start, int end);
    void debugBytes(unsigned char *bytes, int byteCount);

  public:
    void setup(int in, int out, void (*smartDelay)(unsigned long), struct ObdOutputProvider *optionalOutputProvider);
    void connect(int proto);
    void disconnect();
    bool isConnected();
    bool resetConnection();
    int  sendPidRequest(unsigned char pid, int mode);
    long receivePidResponse(unsigned char pid, int mode, bool showErrors, bool showDebug);
    int  receivePidResponseData(unsigned char *buf, int maxBytes, unsigned char pid, int mode, bool showErrors, bool showDebug);
};

#endif