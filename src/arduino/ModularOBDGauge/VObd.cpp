#include "VObd.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// VOBD.CPP
// Obd protocol handling
///////////////////////////////////////////////////////////////

// KWP Key value bits
#define KWP_KEY1_LENGTH_IN_FORMAT_BYTE_SUPPORTED     0x01
#define KWP_KEY1_ADDITIONAL_LENGTH_BYTE_SUPPORTED    0x02
#define KWP_KEY1_ONE_BYTE_HEADER_SUPPORTED           0x04
#define KWP_KEY1_TARGET_SOURCE_BYTE_SUPPORTED        0x08
#define KWP_KEY1_NORMAL_TIMING_SUPPORTED             0x10
#define KWP_KEY1_EXTENDED_TIMING_SUPPORTED           0x20
#define KWP_KEY1_UNUSED_ALWAYS_ONE                   0x40
#define KWP_KEY1_PARITY                              0x80

#define KWP_MESSAGE_FORMAT_ONE_BYTE_HEADER_LENGTH_IN_FORMAT   0
#define KWP_MESSAGE_FORMAT_ONE_BYTE_HEADER_ADDITIONAL_LENGTH  1
#define KWP_MESSAGE_FORMAT_ADDRESS_HEADER_LENGTH_IN_FORMAT    2
#define KWP_MESSAGE_FORMAT_ADDRESS_HEADER_ADDITIONAL_LENGTH   3

//------------------------------------------------------
// Public (Connect)
//------------------------------------------------------

extern void VObd::setup(int in, int out, void (*delay)(unsigned long), struct ObdOutputProvider *optionalOutputProvider) {
  output = optionalOutputProvider;
  vserial.setup(in, out, delay);
  smartDelay = delay;
}

extern void VObd::connect(int proto, bool demoMode) {
  switch (proto) {
    case OBD_PROTOCOL_ISO_9141:
    case OBD_PROTOCOL_KWP_SLOW:
      if (proto = kwpSlowInit(proto, demoMode)) {
        protocol = proto;
      }
      break;
    case OBD_PROTOCOL_KWP_FAST:
      if (proto = kwpFastInit(proto)) {
        protocol = proto;
      }
      break;
  }
  if (protocol) {
    lastPidRequestTime = millis();
  }
}

extern void VObd::disconnect() {
  protocol = 0;
}

extern bool VObd::isConnected() {
  return protocol != 0;
}

extern void VObd::ping() {
  // Periodically send request to keep connection alive
  if (isConnected() && ((long)(millis() - lastPidRequestTime)) > QUERY_MAX_INTERVAL) {
      sendPidRequest(0x00, 1);
  }
}

extern bool VObd::resetConnection() {
  unsigned long start = micros();

  // Send a bunch of nulls to force the ECU to wait for initialization.
  // By happenstance this was found to work on ECU simulator.  Unsure
  // how effective or necessary this is on a real ECU.
  vserial.sendByteRepeatedly(0, 256, 10400L, 0);
  smartDelay(2600);
}

//------------------------------------------------------
// Public (Query)
//------------------------------------------------------

extern int VObd::sendPidRequest(unsigned char pid, int mode) {
  char bytes[20];
  int count = 0;
  int length = (pid || (mode == 1)) ? 2 : 1;
  unsigned long time = millis();
  long wait = lastPidRequestTime + QUERY_MIN_INTERVAL - time;

  // Enforce minimum time between receiving last request and sending new one
  if (wait > 0 && wait <= QUERY_MIN_INTERVAL) {
    smartDelay(wait);
  }
  // Save time for pinging
  lastPidRequestTime = time;

  switch (protocol) {
    case OBD_PROTOCOL_ISO_9141:
      // Example: 68 6A F1 01 00 C4 

      bytes[count++] = 0x68;  // Diagnostic request
      bytes[count++] = 0x6A;  // Target (ECU)
      bytes[count++] = 0xF1;  // Source (Diagnostic device)
      break;

    case OBD_PROTOCOL_KWP_SLOW:
    case OBD_PROTOCOL_KWP_FAST:

      // One byte header with length in format
      if ((keyByte1 & KWP_KEY1_ONE_BYTE_HEADER_SUPPORTED) && (keyByte1 & KWP_KEY1_LENGTH_IN_FORMAT_BYTE_SUPPORTED)) {
        messageFormat = KWP_MESSAGE_FORMAT_ONE_BYTE_HEADER_LENGTH_IN_FORMAT;
        bytes[count++] = length;
      }

      // One byte header with external length byte
      else if ((keyByte1 & KWP_KEY1_ONE_BYTE_HEADER_SUPPORTED)) {
        messageFormat = KWP_MESSAGE_FORMAT_ONE_BYTE_HEADER_ADDITIONAL_LENGTH;
        bytes[count++] = 0x00;  // empty
        bytes[count++] = length;
      }

      // Address header with format length byte
      else if ((keyByte1 & KWP_KEY1_LENGTH_IN_FORMAT_BYTE_SUPPORTED)) {
        messageFormat = KWP_MESSAGE_FORMAT_ADDRESS_HEADER_LENGTH_IN_FORMAT;
        bytes[count++] = 0xC0 | length;  // 80 = address information, 40 = functional addressing, length = 2
        bytes[count++] = 0x33;  // target (engine)
        bytes[count++] = 0xf1;  // source (diagnostic tools F0-FD)
      }

      // Address header with external length byte
      else {
        messageFormat = KWP_MESSAGE_FORMAT_ADDRESS_HEADER_ADDITIONAL_LENGTH;
        bytes[count++] = 0xc0;  // 80 = address information, 40 = functional addressing
        bytes[count++] = 0x33;  // target
        bytes[count++] = 0xf1;  // source (diagnostic tools F0-FD)
        bytes[count++] = length;
      }
      break;
  }

  bytes[count++] = mode;  // data1: mode
  if (pid || (mode == 1)) {
    bytes[count++] = pid;   // data2: pid
  }
  bytes[count] = getChecksum(bytes, 0, count-1);
  vserial.sendBytes(bytes, count+1, 10400, QUERY_SEND_DELAY_BETWEEN_BYTES);
}

extern long VObd::receivePidResponse(unsigned char pid, int mode, bool showErrors, int debugMode) {
  unsigned char bytes[4];
  int byteCount = receivePidResponseData(bytes, 4, pid, mode, showErrors, debugMode);
  long value = 0;

  lastPidRequestTime = millis();

  if (byteCount > 0) {
    for (int i = 0; i<byteCount && i<4; i++) {
      value = (long)bytes[i] | (value << 8);
    }
  } else {
    return byteCount;
  }
  return value;
}

extern int VObd::receivePidResponseData(unsigned char *outbuf, int maxBytes, unsigned char pid, int mode, bool showErrors, int debugMode) {
  unsigned char *bytes;
  unsigned long minByteSpacing = 0;
  unsigned long maxByteSpacing = 0;
  bool isSniffing = (!pid && !mode);

  // Debug mode 2 (dump flips)
  if (debugMode == 2) {
    unsigned long *flips;
    int flipCount = vserial.readFlipIntervals(&flips, QUERY_RECEIVE_MESSAGE_TIMEOUT, isSniffing ? QUERY_RECEIVE_BYTE_TIMEOUT_SNIFFING : QUERY_RECEIVE_BYTE_TIMEOUT);
    if (flipCount > 2) debugLongs(flips, flipCount);
    return;
  }

  int byteCount = vserial.readBytes(&bytes, QUERY_RECEIVE_MESSAGE_TIMEOUT, isSniffing ? QUERY_RECEIVE_BYTE_TIMEOUT_SNIFFING : QUERY_RECEIVE_BYTE_TIMEOUT, 10400, &minByteSpacing, &maxByteSpacing);


  // Debug
  if (debugMode) {
    if (byteCount) debugBytes(bytes, byteCount, minByteSpacing/1000, maxByteSpacing/1000);
    if (isSniffing) return;
  }
  
  if (byteCount == 0) {
    if (output && showErrors) { output->showStatusString_P(PSTR(" -- ")); smartDelay(400); }
    return -1;
  }
  if (byteCount < 3) {
    if (output && showErrors) { output->showStatusString_P(PSTR("Cnt!")); smartDelay(400); output->showStatusInteger(byteCount); smartDelay(100); }
    return -1;
  }
  int headerSize;
  switch (protocol) {
    case OBD_PROTOCOL_ISO_9141:
      // [48 6B 10] [41 0D] 7F [90]
      headerSize = 3;
      break;

    case OBD_PROTOCOL_KWP_SLOW:
    case OBD_PROTOCOL_KWP_FAST:
      // Sample: [83 F1 11] [41 0D] 78 [4B]
      headerSize = 1;
      if (bytes[0] & 0x80) headerSize += 2;       // target, source addresses in header
      if ((bytes[0] & 0x3f)==0) headerSize += 1;  // extra length byte at end of header
      break;
  }

  // Error - wrong # of bytes (minimum response should include mode + pid + data + checksum)
  if (byteCount < headerSize + (mode==3 ? 0 : pid ? 2 : 1) + 1) {
    if (output && showErrors) { output->showStatusString_P(PSTR("Cnt!")); smartDelay(400); output->showStatusInteger(byteCount); smartDelay(100); }
    return -1;
  }

  // Negative Acknowledgement
  if (bytes[headerSize] == 0x7f) {
    if (output && showErrors) { output->showStatusString_P(PSTR("NACK")); smartDelay(400); output->showStatusByte(bytes[headerSize+2]); smartDelay(100); }
    return 0;
  }

  // Error - wrong SID
  if (bytes[headerSize] != (0x40 + mode)) {
    if (output && showErrors) { output->showStatusString_P(PSTR("SID!")); smartDelay(400); output->showStatusByte(bytes[headerSize]); smartDelay(100); }
    return -1;
  }

  int valueStart = headerSize + 1;
  int valueEnd = byteCount-1;
  int count = valueEnd - valueStart;
  int outCount = 0;

  // Read and confirm PID byte for mode 1 requests
  if (mode == 1) {
    if (bytes[valueStart++] != pid) {
      if (output && showErrors) { output->showStatusString_P(PSTR("PID!")); smartDelay(400); output->showStatusByte(bytes[headerSize+1]); smartDelay(100); }
      return -1;
    }
  } 
  
  // Mode 3 does not return a header or checksum apparently, so just read raw bytes(?)
  else if (mode == 3) {
    // count = 0;
    valueEnd++;
    count++;
  }

  // Loop thru data
  for (int i=valueStart; i<valueEnd; i++) {
    if (count > 0) {
      count--;
      if (outCount < maxBytes) {
        outbuf[outCount++] = bytes[i];
      } else {
        break;
      }
    } else {
      // Process ISO-15765 frame header
      switch(bytes[i] & 0xf0) {
        case 0x00: count = bytes[i] & 0x0f; break;
        case 0x10: count = (((long)bytes[i] & 0x0f) << 8) | bytes[i+1]; i++; break; // just use first frame for now
        case 0x20: break; // ignore consecutive frames for now
        case 0x30: break; // ignore flow control frames for now
      }
    }
  }
  return outCount;
}

//------------------------------------------------------
// Private
//------------------------------------------------------

int VObd::kwpSlowInit(int proto, bool demoMode) {

  // W0
  vserial.waitForIdle(SLOW_INIT_IDLE_WAIT, SLOW_INIT_IDLE_TIMEOUT);

  // Address  
  vserial.sendBytes("\x33", 1, 5, QUERY_SEND_DELAY_BETWEEN_BYTES);

  unsigned char *bytes;
  char bytesBuf[3];
  int byteCount = 0;
  
  if (demoMode) {
    byteCount = 3;
    bytes = bytesBuf;
    bytes[0] = 0x55;
    bytes[1] = 0x08;
    bytes[2] = 0x08;
  } else {
    byteCount = vserial.readBytes(&bytes, SLOW_INIT_SYNC_MESSAGE_TIMEOUT, SLOW_INIT_SYNC_BYTE_TIMEOUT, 10400, NULL, NULL);
  }

  if (byteCount != 3) {
    if (output) { output->showStatusString_P(PSTR("Cnt!")); smartDelay(400); output->showStatusInteger(byteCount); }
    return 0;
  }
  
  // Error - wrong synchronization key
  if (bytes[0] != 0x55) {
    if (output) { output->showStatusString_P(PSTR("Key!")); smartDelay(400); output->showStatusByte(bytes[0]); }
    return 0;
  }

  // Delay before sending inverted response, excluding timeout from previous fetch
  smartDelay(SLOW_INIT_INVERSION_DELAY - SLOW_INIT_SYNC_BYTE_TIMEOUT);

  unsigned char response[1];
  response[0] = ~bytes[2];
  vserial.sendBytes(response, 1, 10400, QUERY_SEND_DELAY_BETWEEN_BYTES);

  // Save key bytes, which define types of headers/byte intervals supported
  keyByte1 = bytes[1];
  keyByte2 = bytes[2];

  // Wait for final ready ($CC for 9141.  Simulator returns $FF)
  if (demoMode) {
    byteCount = 1;
    bytes[0] = 0xcc;
  } else {
    byteCount = vserial.readBytes(&bytes, SLOW_INIT_FINAL_MESSAGE_TIMEOUT, SLOW_INIT_FINAL_MESSAGE_TIMEOUT, 10400, NULL, NULL);
  }

  if (output) { 
    char buf[6];
    snprintf(buf, 6, "%02X%02X", (int)keyByte1, (int)keyByte2);
    output->showStatusString(buf);
    smartDelay(500);
    snprintf(buf, 6, "\x03%02X\x03", (int)response[0]);
    output->showStatusString(buf);
    smartDelay(500);
    if (byteCount < 1) {
      output->showStatusString_P(PSTR("Ack!")); smartDelay(500); output->showStatusInteger(byteCount);      
    } else {
      snprintf(buf, 5, "_%02X_", (int)bytes[0]);
      output->showStatusString(buf);
    }
  }
  
  smartDelay(100);

  // Override requested protocol based on keywords received
  // Can only go KWP to 9141 as ECU simulator sends the wrong
  // keyword for 9141
  //
  // B1 B2
  // 08 08 9141
  // 94 94 9141
  // 8F E9 KWP2000 (or 9141 on simulator)
  // 8F 6B KWP2000
  // 8F 6D KWP2000
  // 8F EF KWP2000
  //
  if (keyByte1 == keyByte2 && proto == OBD_PROTOCOL_KWP_SLOW) {
    if (output) { output->showStatusString_P(PSTR("9141")); }
    smartDelay(100);
    return OBD_PROTOCOL_ISO_9141;
  }
  return proto;
}

int VObd::kwpFastInit(int proto) {
  unsigned long start = micros();

  // Send 25ms low, 25ms high
  vserial.sendBit(0, start+25000L);
  vserial.sendBit(1, start+50000L);

  // Send start-communication request
  unsigned char req[] = { 0xc1, 0x33, 0xf1, 0x81, 0x66 };
  vserial.sendBytes(req, 5, 10400, QUERY_SEND_DELAY_BETWEEN_BYTES);

  // Receive start-communication response
  unsigned char *bytes;
  int byteCount = vserial.readBytes(&bytes,  QUERY_RECEIVE_MESSAGE_TIMEOUT, QUERY_RECEIVE_BYTE_TIMEOUT, 10400, NULL, NULL);

  // Error - wrong # of bytes
  if (byteCount == 0) {
    if (output) { output->showStatusString_P(PSTR(" -- ")); smartDelay(400); }
    return 0;
  }
  if (byteCount < 5) {
    if (output) { output->showStatusString_P(PSTR("Cnt!")); smartDelay(400); output->showStatusInteger(byteCount); smartDelay(100); }
    return 0;
  }
  
  // Error - wrong keyword
  if (bytes[3] != 0xC1) {
    if (output) { output->showStatusString_P(PSTR("Kwd!")); smartDelay(400); output->showStatusByte(bytes[3]); smartDelay(100); }
    return 0;
  }
  return proto;
}

unsigned char VObd::getChecksum(unsigned char *buf, int start, int end) {
  unsigned char sum = 0;
  for (int i=start; i<=end; i++) sum += buf[i];
  return sum;
}

void VObd::debugBytes(unsigned char *bytes, int byteCount, int minByteSpacing, int maxByteSpacing) {
  if (output) { 
    output->showStatusString_P(PSTR("CNT="));
    output->showStatusInteger(byteCount);

    autoPidRequestDisabled = true;
    smartDelay(1000);

    // Show initial byte in buffer if response is empty (debug code)
    for (int i=0; i<byteCount; i++) {
      char text[6];
      snprintf(text, sizeof(text), "%-2d.%02X", i, (int)bytes[i]);
      output->showStatusString(text);
      smartDelay(500);
    }
    smartDelay(500);

    // if (minByteSpacing >= 0) {
    //   output->showStatusString_P(PSTR("LoP="));
    //   output->showStatusInteger(minByteSpacing);
    // }
    // if (maxByteSpacing >= 0) {
    //   output->showStatusString_P(PSTR("HiP="));
    //   output->showStatusInteger(maxByteSpacing);
    // }
    // smartDelay(500);

    lastPidRequestTime = millis();  // Note, ECU may time us out
    autoPidRequestDisabled = false;
  }
}

void VObd::debugLongs(unsigned long *longs, int longCount) {
  if (output) { 
    output->showStatusString_P(PSTR("CNT="));
    output->showStatusInteger(longCount);

    autoPidRequestDisabled = true;
    smartDelay(1000);

    // Show initial byte in buffer if response is empty (debug code)
    for (int i=0; i<longCount && i< 20; i++) {
      int value = longs[i] > 9999 ? 9999 : longs[i];
      output->showStatusInteger(value);
      smartDelay(500);
    }
    smartDelay(500);

    lastPidRequestTime = millis();  // Note, ECU may time us out
    autoPidRequestDisabled = false;
  }
}

