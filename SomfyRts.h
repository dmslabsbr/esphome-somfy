#ifndef somfyrts_h
#define somfyrts_h
#include <Arduino.h>

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8

#ifdef REMOTE_TX_PIN
#else
 #define REMOTE_TX_PIN D0
#endif

class SomfyRts {

  private:
    bool _debug;
    uint32_t _remoteId;
    uint16_t newRollingCode = 1000;       //<-- Change it!
    uint16_t rollingCode = 0;
    unsigned char _frame[7];
    char checksum;
    uint16_t _readRemoteRollingCode();
    void _writeRemoteRollingCode(uint16_t code);
    String _getConfigFilename();

  public:
    SomfyRts(uint32_t remoteID, bool debug);
    SomfyRts(uint32_t remoteID);
    void init();
    void sendCommandUp();
    void sendCommandDown();
    void sendCommandStop();
    void sendCommandProg();
    void buildFrame(unsigned char *frame, unsigned char button);
    void sendCommand(unsigned char *frame, unsigned char sync);
};

#endif