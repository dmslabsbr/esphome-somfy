#include "esphome.h"
#include <SomfyRts.h>
#include <Ticker.h>
#include "FS.h"

// cmd 11 - program mode
// cmd 21 - delete rolling code file
// cmd 41 - List files
// cmd 51 - Test filesystem.
// cmd 61 - Format filesystem and test.
// cmd 71 - Show actual roling code

class RFsomfy : public Component, public Cover {

 private:
  int index;
  Ticker ticker;

  
 public:
  #define STATUS_LED_PIN D1
  #define REMOTE_TX_PIN D0
  #define REMOTE_FIRST_ADDR 0x121311   // <- Change remote name and remote code here!
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
  
  void readFile()
    {
        ESP_LOGW("info","Readfile");
        Serial.println("reading");
        File f = SPIFFS.open("/myFile.txt", "r");
        if (!f) {
            Serial.println("file not available");
            ESP_LOGW("info","Readfile-File not avaliable");
        } else if (f.available()<=0) {
           ESP_LOGW("info","Readfile-File exist but not avaliable");
           Serial.println("file exists but available <0");
           }
        else
        {
            ESP_LOGI("info","Readfile OK");
            String ssidString = f.readStringUntil('#');
            Serial.print("read from file: ");
            Serial.println(ssidString);
        }
        f.close();
    }
  
  void writeFile()
    {
    ESP_LOGW("info","writefile");
    Serial.println("writing");
    File f = SPIFFS.open("/myFile.txt", "w");
    if (!f) {
        Serial.println("File creation failed");
        ESP_LOGW("info","writefile-failed");
     }
    else
    {
        ESP_LOGW("info","Write_OK");
        f.print("networkConfig");
        f.print("#");
        f.flush();
        f.close();
    }
    }

    void testFs() {
        ESP_LOGW("tilt","Testing filesystem!");
        Serial.begin(115200);
        if (!SPIFFS.begin()) {
            ESP_LOGW("tilt","error while mounting filesystem!");
            Serial.println("error while mounting filesystem!");
        } else {
            readFile();
            writeFile();
            readFile();
            Serial.println("done");
        }
        SPIFFS.end();
    }

  
  
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

    testFs();
  }

  // delete rolling code . 0....n
  void delete_code(int remoteId) {
      SPIFFS.begin();
      String path = file_path(remoteId);
      Serial.println(path);
      // SPIFFS.remove(path);
      Serial.println("Deleted");
      ESP_LOGD("RFsomfy","Deleted remote %i", remoteId);
      SPIFFS.end();
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
    return "";
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
      
      if (xpos == 41) {
        ESP_LOGD("tilt","List Files");
        String str = "";
        SPIFFS.begin();
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            str += dir.fileName();
            str += " / ";
            str += dir.fileSize();
            str += "\r\n";
        }
        Serial.print(str);
        ESP_LOGD("files", string2char(str));
        SPIFFS.end();
      }
      
     if (xpos == 51) {
        ESP_LOGD("tilt","51 mode");
        testFs();
      }
      
     if (xpos == 61) {
        ESP_LOGD("tilt","61 mode");
        SPIFFS.begin();

        if (!SPIFFS.exists("/formatComplete.txt")) {
            Serial.println("Please wait 30 secs for SPIFFS to be formatted");
            ESP_LOGW("file", "Please wait 30 s");
            SPIFFS.format();
            delay(30000);
            Serial.println("Spiffs formatted");
            ESP_LOGW("file", "Spiffs formatted");
        
        File f = SPIFFS.open("/formatComplete.txt", "w");
        if (!f) {
            Serial.println("file open failed");
            ESP_LOGW("file", "file open failed");
        } else {
            f.println("Format Complete");
            ESP_LOGW("file", "Format Complete");
        } 
        } else {
            Serial.println("SPIFFS is formatted. Moving along...");
            ESP_LOGW("file", "SPIFFS is formatted. Moving along...");
        }
        SPIFFS.end();
     }
     
     if (xpos == 71) {
      uint16_t code = 0;
      SPIFFS.begin();
      String arq = file_path(remoteId);
      if (SPIFFS.exists(arq)) {
        Serial.println("Reading config");
        ESP_LOGI("file", "Reading config");
        ESP_LOGI("Arq", string2char(arq));
        File f = SPIFFS.open(arq, "r");
        if (f) {
          String line = f.readStringUntil('\n');
          code = line.toInt();
          f.close();
        }
        else {
          Serial.println("File open failed");
          ESP_LOGI("file", "File open failed");
        }
      }
      ESP_LOGI("file", "Code: %d", code);
      SPIFFS.end();
     }
      
      
      
      this->tilt = tpos;
      this->publish_state();
      
      
      
    }
    
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(50);
    
  }
};
