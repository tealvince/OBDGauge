///////////////////////////////////////////////////////////////
// VOBD.H
// Obd protocol handling
///////////////////////////////////////////////////////////////

#ifndef _VOBD
#define _VOBD

#include "VSerial.h"

#define OBD_PROTOCOL_AUTOMATIC 0
#define OBD_PROTOCOL_KWP_SLOW  1
#define OBD_PROTOCOL_ISO_9141  2
#define OBD_PROTOCOL_KWP_FAST  3

#define OBD_PROTOCOL_FIRST     1
#define OBD_PROTOCOL_LAST      3

// Timing values from ISO-9141-2 specification

#define ISO_9141_W0_MIN_IDLE_BEFORE_INIT_ADDRESS        2
#define ISO_9141_W1_MIN_DELAY_BEFORE_SYNCHRONIZATION    60
#define ISO_9141_W1_MAX_DELAY_BEFORE_SYNCHRONIZATION    300
#define ISO_9141_W2_MIN_DELAY_BEFORE_KEYWORDS           5
#define ISO_9141_W2_MAX_DELAY_BEFORE_KEYWORDS           20
#define ISO_9141_W3_MIN_DELAY_BETWEEN_KEYWORDS          0
#define ISO_9141_W3_MAX_DELAY_BETWEEN_KEYWORDS          20
#define ISO_9141_W4_MIN_DELAY_BEFORE_INVERSION          25
#define ISO_9141_W4_MAX_DELAY_BEFORE_INVERSION          50
#define ISO_9141_W4_MIN_IDLE_BEFORE_RESEND_ADDRESS      300

#define ISO_9141_PO_MIN_BYTE_SPACING_FROM_VEHICLE           0
#define ISO_9141_PO_MAX_BYTE_SPACING_FROM_VEHICLE           20
#define ISO_9141_P1_MIN_MESSAGE_SPACING_FROM_VEHICLE_94     0
#define ISO_9141_P1_MAX_MESSAGE_SPACING_FROM_VEHICLE_94     50
#define ISO_9141_P2_MIN_MESSAGE_SPACING_FROM_VEHICLE_08     25
#define ISO_9141_P2_MAX_MESSAGE_SPACING_FROM_VEHICLE_08     50
#define ISO_9141_P3_MIN_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE 55
#define ISO_9141_P3_MAX_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE 5000
#define ISO_9141_P4_MIN_BYTE_SPACING_TO_VEHICLE             5
#define ISO_9141_P4_MAX_BYTE_SPACING_TO_VEHICLE             20

// Timing values from ISO-14320-2 specification

#define KWP_P1_MIN_BYTE_SPACING_FROM_VEHICLE            0
#define KWP_P1_MAX_BYTE_SPACING_FROM_VEHICLE            20
#define KWP_P2_MIN_MESSAGE_SPACING_FROM_VEHICLE         25
#define KWP_P2_MAX_MESSAGE_SPACING_FROM_VEHICLE         50
#define KWP_P3_MIN_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE  55
#define KWP_P3_MAX_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE  5000
#define KWP_P4_MIN_BYTE_SPACING_TO_VEHICLE              5
#define KWP_P4_MAX_BYTE_SPACING_TO_VEHICLE              50

// Timings we'll use based on the above ranges

#define SLOW_INIT_IDLE_WAIT                (ISO_9141_W0_MIN_IDLE_BEFORE_INIT_ADDRESS+1)
#define SLOW_INIT_IDLE_TIMEOUT             300
#define SLOW_INIT_SYNC_MESSAGE_TIMEOUT     (ISO_9141_W1_MAX_DELAY_BEFORE_SYNCHRONIZATION*2)
#define SLOW_INIT_SYNC_BYTE_TIMEOUT        (ISO_9141_W3_MAX_DELAY_BETWEEN_KEYWORDS*3/2) // 30
#define SLOW_INIT_INVERSION_DELAY          (ISO_9141_W4_MIN_DELAY_BEFORE_INVERSION+ISO_9141_W4_MAX_DELAY_BEFORE_INVERSION)/2  // 37
#define SLOW_INIT_FINAL_MESSAGE_TIMEOUT    (ISO_9141_W4_MIN_IDLE_BEFORE_RESEND_ADDRESS+1)

#define QUERY_RECEIVE_MESSAGE_TIMEOUT       (KWP_P2_MAX_MESSAGE_SPACING_FROM_VEHICLE+1)
#define QUERY_RECEIVE_BYTE_TIMEOUT          (KWP_P1_MAX_BYTE_SPACING_FROM_VEHICLE+1)
#define QUERY_RECEIVE_BYTE_TIMEOUT_SNIFFING 300
#define QUERY_SEND_DELAY_BETWEEN_BYTES      (ISO_9141_P4_MIN_BYTE_SPACING_TO_VEHICLE+1)
#define QUERY_MIN_INTERVAL                  (KWP_P3_MIN_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE+5)
#define QUERY_MAX_INTERVAL                  (KWP_P3_MAX_DELAY_BEFORE_NEW_MESSAGE_TO_VEHICLE-1000)

struct ObdOutputProvider {
  void  (*showStatusString)(char *text);
  void  (*showStatusString_P)(char *text);
  void  (*showStatusInteger)(int num);
  void  (*showStatusByte)(int num);
};

class VObd {
  private:
    VSerial vserial;
    int protocol = 0;
    int messageFormat;
    unsigned char keyByte1; // Used for kwp header format
    unsigned char keyByte2; // Used for kwp header format
    unsigned long lastPidRequestTime;
    bool autoPidRequestDisabled;
    void (*smartDelay)(unsigned long);

    struct ObdOutputProvider *output;

    int kwpSlowInit(int protocol, bool demoMode);
    int kwpFastInit(int protocol);
    unsigned char getChecksum(unsigned char *buf, int start, int end);
    void debugBytes(unsigned char *bytes, int byteCount, int minByteSpacing, int maxByteSpacing);
    void debugLongs(unsigned long *longs, int longCount);

  public:
    void setup(int in, int out, void (*smartDelay)(unsigned long), struct ObdOutputProvider *optionalOutputProvider);
    void connect(int proto, bool demoMode);
    void disconnect();
    bool isConnected();
    void ping();
    bool resetConnection();
    int  sendPidRequest(unsigned char pid, int mode);
    long receivePidResponse(unsigned char pid, int mode, bool showErrors, int debugMode);  // pid0 + mode0 is a special sniffer mode
    int  receivePidResponseData(unsigned char *buf, int maxBytes, unsigned char pid, int mode, bool showErrors, int debugMode);
};

#endif