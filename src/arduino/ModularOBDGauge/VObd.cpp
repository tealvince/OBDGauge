#include "VObd.h"
#include <Arduino.h>

///////////////////////////////////////////////////////////////
// OBD
// Obd protocol handling
///////////////////////////////////////////////////////////////

//------------------------------------------------------
// Public
//------------------------------------------------------

extern void VObd::setup(int in, int out, void (*smartDelay)(unsigned long), struct ObdOutputProvider *optionalOutputProvider) {
  output = optionalOutputProvider;
  vserial.setup(in, out, smartDelay);
  pidResponseTimeoutMs = 1000L;
  initResponseTimeoutMs = 3000L;
}

extern void VObd::connect(int proto) {
  switch (proto) {
    case OBD_PROTOCOL_ISO_9141:
    case OBD_PROTOCOL_KWP_SLOW:
      if (kwpSlowInit()) {
        protocol = proto;
      }
      break;
    case OBD_PROTOCOL_KWP_FAST:
      if (kwpFastInit()) {
        protocol = proto;
      }
      break;
  }
}

extern void VObd::disconnect() {
  protocol = 0;
}

extern bool VObd::isConnected() {
  return protocol != 0;
}

extern bool VObd::resetConnection() {
  unsigned long start = micros();

  // Send a bunch of nulls to force the ECU to wait for initialization.
  // By happenstance this was found to work on ECU simulator.  Unsure
  // how effective or necessary this is on a real ECU.
  vserial.sendByteRepeatedly(0, 256, 10400L);
  delay(2600);
}

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

extern int VObd::sendPidRequest(unsigned char pid, int mode) {
  char bytes[20];
  int count = 0;
  int length = pid ? 2 : 1;

  switch (protocol) {
    case OBD_PROTOCOL_ISO_9141:
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
  if (pid) {
    bytes[count++] = pid;   // data2: pid
  }
  bytes[count] = getChecksum(bytes, 0, count-1);
  vserial.sendBytes(bytes, count+1, 10400);
}

extern long VObd::receivePidResponse(unsigned char pid, int mode, bool showErrors, bool showDebug) {
  unsigned char bytes[4];
  int byteCount = receivePidResponseData(bytes, 4, pid, mode, showErrors, showDebug);
  long value = 0;

  if (byteCount > 0) {
    for (int i = 0; i<byteCount && i<4; i++) {
      value = (long)bytes[i] | (value << 8);
    }
  } else {
    return byteCount;
  }
  return value;
}

extern int VObd::receivePidResponseData(unsigned char *outbuf, int maxBytes, unsigned char pid, int mode, bool showErrors, bool showDebug) {
  unsigned char *bytes;
  int byteCount = vserial.readBytes(&bytes, pidResponseTimeoutMs, 10400);

  if (showDebug) {
    debugBytes(bytes, byteCount);
  }

  if (byteCount < 3) {
    if (output && showErrors) { output->showStatusString("Cnt!"); output->showStatusInteger(byteCount); }
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
    if (output && showErrors) { output->showStatusString("Cnt!"); output->showStatusInteger(byteCount); }
    return -1;
  }

  // Error - wrong SID
  if (bytes[headerSize] != (0x40 + mode)) {
    if (output && showErrors) { output->showStatusString("SID!"); output->showStatusByte(bytes[headerSize]); }
    return -1;
  }

  int valueStart = headerSize+1;
  int valueEnd = byteCount-1;
  int count = valueEnd - valueStart;
  int outCount = 0;

  // Read and confirm PID byte for mode 1 requests
  if (mode == 1) {
    if (bytes[valueStart++] != pid) {
      if (output && showErrors) { output->showStatusString("PID!"); output->showStatusByte(bytes[headerSize+1]); }
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

bool VObd::kwpSlowInit() {
  vserial.sendBytes("\x33", 1, 5);

  unsigned char *bytes;
  int byteCount = vserial.readBytes(&bytes, initResponseTimeoutMs, 10400);

  // Error - wrong # of bytes
  if (byteCount != 3) {
    if (output) output->showStatusString("Cnt!");
    return false;
  }
  
  // Error - wrong keyword
  if (bytes[0] != 0x55) {
    if (output) { output->showStatusString("RES!"); output->showStatusByte(bytes[0]); }
    return false;
  }

  char response = ~bytes[2];
  vserial.sendBytes(&response, 1, 10400);

  // Output key bytes, which define types of headers supported
  keyByte2 = bytes[2];
  keyByte1 = bytes[1];

  if (output) { 
    char buf[6];
    snprintf(buf, 6, "%02X%02X", (int)keyByte2, (int)keyByte1);
    output->showStatusString(buf);
  }

  return true;
}

bool VObd::kwpFastInit() {
  unsigned long start = micros();

  // Send 25ms low, 25ms high
  vserial.sendBit(0, start+25000L);
  vserial.sendBit(1, start+50000L);

  // Send start-communication request
  unsigned char req[] = { 0xc1, 0x33, 0xf1, 0x81, 0x66 };
  vserial.sendBytes(req, 5, 10400);

  // Receive start-communication response
  unsigned char *bytes;
  int byteCount = vserial.readBytes(&bytes, initResponseTimeoutMs, 10400);

  // Error - wrong # of bytes
  if (byteCount < 5) {
    if (output) { output->showStatusString("Cnt!"); output->showStatusInteger(byteCount); }
    return false;
  }
  
  // Error - wrong keyword
  if (bytes[3] != 0xC1) {
    if (output) { output->showStatusString("RES!"); output->showStatusByte(bytes[3]); }
    return false;
  }
  return true;
}

unsigned char VObd::getChecksum(unsigned char *buf, int start, int end) {
  unsigned char sum = 0;
  for (int i=start; i<=end; i++) sum += buf[i];
  return sum;
}

void VObd::debugBytes(unsigned char *bytes, int byteCount) {
  if (output) { 
    output->showStatusString("CNT=");
    output->showStatusInteger(byteCount);

    delay(1000);

    for (int i; i<byteCount; i++) {
      char text[5];
      snprintf(text, sizeof(text), "%-2d%02X", i, (int)bytes[i]);
      output->showStatusString(text);
      delay(500);
    }
    delay(1000);
  }
}


