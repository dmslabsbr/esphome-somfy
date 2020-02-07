#include "esphome.h"
#include <SomfyRts.h>
#include <Ticker.h>

// cmd 11 - program mode
// cmd 21 - delete rolling code file

class RFsomfy : public Component, public Cover {

 private:
  int index;
  Ticker ticker;
  
 public:
  #define STATUS_LED_PIN D1
  #define REMOTE_TX_PIN D0
  #define REMOTE_FIRST_ADDR 0x121300   // <- Change remote name and remote code here!
  #define REMOTE_COUNT 3
  int remoteId = -1;    
  unsigned char frame[7];
 
  /*
  void tickLed(void) {
      //toggle state
      int state = digitalRead(STATUS_LED_PIN);
      digitalWrite(STATUS_LED_PIN, !state);
  }
  usar ticker.attach_ms(50, RFsomfy::tickLed);
  */
  
  SomfyRts rtsDevices[REMOTE_COUNT] = {
    SomfyRts(REMOTE_FIRST_ADDR),
    SomfyRts(REMOTE_FIRST_ADDR + 1),
    SomfyRts(REMOTE_FIRST_ADDR + 2)
  };

  RFsomfy(int rmx) : Cover() { //register
    index = rmx;
    remoteId = index;
    ESP_LOGD("somfy", "Cover %d", index);
  }

  void setup() override {
    // This will be called by App.setup()
    ESP_LOGD("RFsomfy", "Starting Device");
    Serial.begin(115200);
    Serial.println("Initialize remote devices");
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
    ESP_LOGD("RFsomfy","Somfy ESPHome Cover v0.12");
    ESP_LOGD("RFsomfy","Initialize remote devices");
    for (int i=0; i<REMOTE_COUNT; i++) {
      rtsDevices[i].init();
      Serial.println("i: " + String(i));
      ESP_LOGD("somfy", "Initialize remote %d", REMOTE_FIRST_ADDR + i);
      ESP_LOGD("somfy","file path:");
      String fp = file_path(i);
      Serial.print("Init remote: ");
      Serial.println(fp);
    }
    //rtsDevices[index].init();
    //ESP_LOGD("somfy", "Initialize remote %d", REMOTE_FIRST_ADDR + index);
    digitalWrite(STATUS_LED_PIN, LOW);
  }

  // delete rolling code . 0....n
  void delete_code(int remoteId) {
      SPIFFS.begin();
      //String path = "/data/remote/";
      //path += REMOTE_FIRST_ADDR + remoteId;
      //path += ".txt";
      String path = file_path(remoteId);
      Serial.println(path);
      SPIFFS.remove(path);
      Serial.println("Deleted");
      ESP_LOGD("RFsomfy","Deleted remote %i", remoteId);
  }
  
  String file_path(int remoteId) {
    String path = "/data/remote/";
      path += REMOTE_FIRST_ADDR + remoteId;
      path += ".txt";
    return path;
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(false);
    traits.set_supports_position(true);
    traits.set_supports_tilt(true); // to send other commands
    return traits;
  }
  
  char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
   }
  
  void control(const CoverCall &call) override {
    // This will be called every time the user requests a state change.
    
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(50);
    
    ESP_LOGD("RFsomfy", "Using remote %d", REMOTE_FIRST_ADDR + index);
    ESP_LOGW("RFsomfy", "Remoteid %d", remoteId);
    ESP_LOGW("RFsomfy", "index %d", index);
    
    Serial.print("remoteId: ");
    Serial.println(remoteId);
    
    
    if (call.get_position().has_value()) {
      float pos = *call.get_position();
      // Write pos (range 0-1) to cover
      // ...
      int ppos = pos * 100;
      ESP_LOGD("RFsomfy", "get_position is: %d", ppos);

      if (ppos == 0) {
        ESP_LOGD("RFsomfy","POS 0");
        Serial.println("* Command Down");
        rtsDevices[remoteId].sendCommandDown();
        pos = 0.01;
      }

      if (ppos == 100) {
        ESP_LOGD("RFsomfy","POS 100");
        Serial.println("* Command UP");
        rtsDevices[remoteId].sendCommandUp();
        pos = 0.99;
      }

      // Publish new state
      this->position = pos;
      this->publish_state();
    }
    if (call.get_stop()) {
      // User requested cover stop
      ESP_LOGD("RFsomfy","get_stop");
      Serial.println("* Command STOP - ");
      Serial.println(remoteId);
      rtsDevices[remoteId].sendCommandStop();
    }
    
    if (call.get_tilt().has_value()) {
      auto tpos = *call.get_tilt();
      int xpos = tpos * 100;
      ESP_LOGI("tilt", "Command tilt xpos: %d", xpos);

      
      if (xpos == 11) {
        ESP_LOGD("tilt","program mode");
        digitalWrite(STATUS_LED_PIN, HIGH);
        rtsDevices[remoteId].sendCommandProg();
        delay(1000);
      }
      if (xpos == 21) {
        ESP_LOGD("tilt","delete file");
        digitalWrite(STATUS_LED_PIN, HIGH);
        delete_code(remoteId);
        delay(1000);
      }
      this->tilt = tpos;
      this->publish_state();
      
    }
    
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(50);
    
  }
};
