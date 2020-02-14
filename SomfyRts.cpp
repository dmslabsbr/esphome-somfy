#include "SomfyRts.h"
#include <FS.h>

//String realSize = String(ESP.getFlashChipRealSize());
//String ideSize = String(ESP.getFlashChipSize());
//bool flashCorrectlyConfigured = realSize.equals(ideSize);


SomfyRts::SomfyRts(uint32_t remoteID, bool debug) {
    _debug = debug;
    _remoteId = remoteID;
}

SomfyRts::SomfyRts(uint32_t remoteID) {
  _debug = false;
  _remoteId = remoteID;
}

void SomfyRts::init() {
  pinMode(REMOTE_TX_PIN, OUTPUT);
  digitalWrite(REMOTE_TX_PIN, LOW);

  rollingCode = _readRemoteRollingCode();
  if (rollingCode < newRollingCode) {
    _writeRemoteRollingCode(newRollingCode);
  }
  if (Serial) {
      Serial.print("Simulated remote number : "); Serial.println(_remoteId, HEX);
      Serial.print("Current rolling code    : "); Serial.println(rollingCode);
  }
}

void SomfyRts::buildFrame(unsigned char *frame, unsigned char button) {
    unsigned int code = _readRemoteRollingCode();
    frame[0] = 0xA7;            // Encryption key. Doesn't matter much
    frame[1] = button << 4;     // Which button did  you press? The 4 LSB will be the checksum
    frame[2] = code >> 8;       // Rolling code (big endian)
    frame[3] = code;            // Rolling code
    frame[4] = _remoteId >> 16; // Remote address
    frame[5] = _remoteId >>  8; // Remote address
    frame[6] = _remoteId;       // Remote address

    if (Serial) {
        Serial.print("Frame         : ");
        for(byte i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) { // Displays leading zero in case the most significant
                Serial.print("0");       // nibble is a 0.
            }
                Serial.print(frame[i],HEX); Serial.print(" ");
        }
    }

    // Checksum calculation: a XOR of all the nibbles
    checksum = 0;
    for(byte i = 0; i < 7; i++) {
        checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
    }
    checksum &= 0b1111; // We keep the last 4 bits only


    //Checksum integration
    frame[1] |= checksum;   //  If a XOR of all the nibbles is equal to 0, the blinds will
                            // consider the checksum ok.

    if (Serial) {
        Serial.println(""); Serial.print("With checksum : ");
        for(byte i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) {
                Serial.print("0");
            }
                Serial.print(frame[i],HEX); Serial.print(" ");
        }
    }

    // Obfuscation: a XOR of all the bytes
    for(byte i = 1; i < 7; i++) {
        frame[i] ^= frame[i-1];
    }

    if (Serial) {
        Serial.println(""); Serial.print("Obfuscated    : ");
        for(byte i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) {
                Serial.print("0");
            }
                Serial.print(frame[i],HEX); Serial.print(" ");
        }
        Serial.println("");
        Serial.print("Rolling Code  : "); Serial.println(code);
    }

    //  We store the value of the rolling code in the FS
    _writeRemoteRollingCode(code + 1);
}

void SomfyRts::sendCommand(unsigned char *frame, unsigned char sync) {
    if(sync == 2) { // Only with the first frame.
        // Wake-up pulse & Silence
        digitalWrite(REMOTE_TX_PIN, HIGH);
        delayMicroseconds(9415);
        digitalWrite(REMOTE_TX_PIN, LOW);
        delayMicroseconds(89565);
    }

    // Hardware sync: two sync for the first frame, seven for the following ones.
    for (int i = 0; i < sync; i++) {
        digitalWrite(REMOTE_TX_PIN, HIGH);
        delayMicroseconds(4*SYMBOL);
        digitalWrite(REMOTE_TX_PIN, LOW);
        delayMicroseconds(4*SYMBOL);
    }

    // Software sync
    digitalWrite(REMOTE_TX_PIN, HIGH);
    delayMicroseconds(4550);
    digitalWrite(REMOTE_TX_PIN, LOW);
    delayMicroseconds(SYMBOL);


    //Data: bits are sent one by one, starting with the MSB.
    for(byte i = 0; i < 56; i++) {
        if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
            digitalWrite(REMOTE_TX_PIN, LOW);
            delayMicroseconds(SYMBOL);
            // PORTD ^= 1<<5;
            digitalWrite(REMOTE_TX_PIN, HIGH);
            delayMicroseconds(SYMBOL);
        }
        else {
            digitalWrite(REMOTE_TX_PIN, HIGH);
            delayMicroseconds(SYMBOL);
            // PORTD ^= 1<<5;
            digitalWrite(REMOTE_TX_PIN, LOW);
            delayMicroseconds(SYMBOL);
        }
    }

    digitalWrite(REMOTE_TX_PIN, LOW);
    delayMicroseconds(30415); // Inter-frame silence
}

void SomfyRts::sendCommandUp() {
    buildFrame(_frame, HAUT);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandDown() {
    buildFrame(_frame, BAS);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandStop() {
    buildFrame(_frame, STOP);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandProg() {
    buildFrame(_frame, PROG);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

uint16_t SomfyRts::_readRemoteRollingCode() {
  uint16_t code = 0;
  SPIFFS.begin();
  if (SPIFFS.exists(_getConfigFilename())) {
    Serial.println("Reading config");
    File f = SPIFFS.open(_getConfigFilename(), "r");
    if (f) {
      String line = f.readStringUntil('\n');
      code = line.toInt();
      f.close();
    }
    else {
      Serial.println("File open failed");
    }
  }
  // mudar
  // if (_remoteId==1184513) code=0;
  SPIFFS.end();
  return code;
}

void SomfyRts::_writeRemoteRollingCode(uint16_t code) {

  SPIFFS.begin();
  Serial.println("Writing config");
  File f = SPIFFS.open(_getConfigFilename(), "w");
  if (f) {
    f.println(code);
    f.close();
    Serial.print("Writed code:");
    Serial.println(code);
  }
  else {
    Serial.println("File creation failed");
  }
  SPIFFS.end();
}

String SomfyRts::_getConfigFilename() {
  String path = "/data/remote/";
  path += _remoteId;
  path += ".txt";
  Serial.println(path);
  return path;
}