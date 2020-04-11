#include "esphome.h"
using namespace esphome;
#include <SomfyRts.h>
#include <Ticker.h>
#include "FS.h"

// cmd 11 - program mode
// cmd 21 - delete rolling code file
// cmd 41 - List files
// cmd 51 - Test filesystem.
// cmd 61 - Format filesystem and test.
// cmd 71 - Show actual rolling code
// cmd 81 - Get all rolling code
// cmd 85 - Write new rolling codes

#define STATUS_LED_PIN D1
#define REMOTE_TX_PIN D0
#define REMOTE_FIRST_ADDR 0x121311   // <- Change remote name and remote code here!
#define REMOTE_COUNT 3   // <- Number of somfy blinds.


int xcode[REMOTE_COUNT];
uint16_t iCode[REMOTE_COUNT];

char const * string2char(String command) {
  if(command.length()!=0){
      char *p = const_cast<char*>(command.c_str());
      return p;
  } 
  return "";
}

String file_path(int remoteId) {
  String path = "/data/remote/";
    path += REMOTE_FIRST_ADDR + remoteId;
    path += ".txt";
  return path;
}

uint16_t getCodeFromFile(int remoteId) {
  uint16_t code = 0;
  SPIFFS.begin();
  String arq = file_path(remoteId);
  if (SPIFFS.exists(arq)) {
    Serial.println("Reading config");
    //ESP_LOGI("file", "Reading config");
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
      code = -1;
    }
  }
  SPIFFS.end();
  return code;
}

// get code from all files
void getCodeFromAllFiles() {
  uint16_t code = 0;
  ESP_LOGW("somfy", "* Get all rolling codes from files");
  for (int i=0; i<REMOTE_COUNT; i++) {
    code = getCodeFromFile(i);
    ESP_LOGI("somfy", "Remoteid %d", REMOTE_FIRST_ADDR + i);
    ESP_LOGI("file", "Code: %d", code);
    xcode[i] = code;
  }
}

void writeCode2file(int remoteId, uint16_t code) {
  SPIFFS.begin();
  Serial.println("Writing config");
  String arq = file_path(remoteId);
  File f = SPIFFS.open(arq, "w");
  if (f) {
    f.println(code);
    f.close();
    ESP_LOGI("somfy", "Writed code: %d", code);
  }
  else {
    ESP_LOGW("somfy","File creation failed");
  }
  SPIFFS.end();
}


class RFsomfy : public Component, public Cover {

 private:
  int index;
  Ticker ticker;

 //protected:
 //  uint8_t width{8};
  
 public:
  int remoteId = -1;    
  unsigned char frame[7];

  void set_code(const uint16_t code) { 
    // this->width = width;
    iCode[remoteId] = code;
    writeCode2file(remoteId, code);
    xcode[remoteId] = code;
    }

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
      xcode[i] = i;
    }

    digitalWrite(STATUS_LED_PIN, LOW);

    //  testFs();
    getCodeFromAllFiles();
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


  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(false);
    traits.set_supports_position(true);
    traits.set_supports_tilt(true); // to send other commands
    return traits;
  }
  
  
  void control(const CoverCall &call) override {
    // This will be called every time the user requests a state change.
    
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(50);
    
    ESP_LOGW("RFsomfy", "Using remote %d", REMOTE_FIRST_ADDR + index);
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
       // get roling code from file
       uint16_t code = 0;
       code = getCodeFromFile(remoteId);
       ESP_LOGI("file", "Code: %d", code);
       xcode[remoteId] = code;
     }

     if (xpos == 81) {
       // get all roling code from file
      getCodeFromAllFiles();
     }
     if (xpos == 85) {
       // Write new roling codes
        for (int i=0; i<REMOTE_COUNT; i++) {
          writeCode2file(REMOTE_FIRST_ADDR + i, iCode[i]);
        }
     }

    /* Don't publish
    this->tilt = tpos;
    this->publish_state();
    */ 
  }
    
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(50);
    
  }
};

class RFsomfyInfo : public PollingComponent, public TextSensor {
 public:
  // constructor
  RFsomfyInfo() : PollingComponent(15000) {}

  void setup() override {
    // This will be called by App.setup()
  }
  void update() override {
    // This will be called every "update_interval" milliseconds.
    // Publish state
    char tmp[REMOTE_COUNT * 100];
    strcpy (tmp,"");
    char str[5];
    char str2[3];
    String rem;
    boolean bl_code {false};
    for (int i=0; i<REMOTE_COUNT; i++) {
      String rem = String(REMOTE_FIRST_ADDR + i, HEX);
      /*
      itoa(xcode[i],str,10);
      itoa(i, str2, 10);
      strcat(tmp, "(");
      strcat(tmp, str2);
      */
      char linha[100];
      sprintf(linha, "\n ( %u - #%s) - code: %d / ", i, string2char(rem), xcode[i]);
      strcat(tmp, linha);
      if (iCode[i] != 0) {
        bl_code = true;
        ESP_LOGI("icode", "icode: %d", iCode[i]);
      }
    }
    publish_state(tmp);
    if (bl_code) {
      ESP_LOGW("set_code", "Atention! After set code, and it works, remove it from your YAML");
    }
  }
};