#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define DIGEST "19650604"

class Param {
  private:    
    #define CSSID "MrWhite"
    #define WIFIPASSWORD "Feketebikapata1965!"
    #define DAYTEMP 25.0
    #define NIGHTTEMP 18.0
    #define DAYSTART 0700
    #define NIGHTSTART 2300
    #define NOTEMPDATATIMEOUT 400000
    #define SWITCHTHRESHOLD 0.20
    #define AUTORESET true
    #define NOAUTORESET false
    #define NOTEMPDATA 10
    #define MANUALTOGGLETIMEOUT 900000
    #define TIMER_1TIMEOUT 15000
    unsigned long startTime;
    unsigned long waitTime;
    String _paramJSON;
  public:  
    struct tParam {
        String sSSID;
        String sWiFiPassword;
        String sDayStart;
        String sNightStart;
        String sDayTemp;
        String sNightTemp;
        String sSwithThreshold;
        DeserializationError error;
    };    
    Param();
    Param::tParam getParam(Param::tParam param);
    bool saveParam (tParam param);
  /*private:
    tParam _param;*/
};

DynamicJsonDocument doc(1024);
struct Param::tParam {
    String sSSID;
    String sWiFiPassword;
    String sDayStart;
    String sNightStart;
    String sDayTemp;
    String sNightTemp;
    String sSwithThreshold;
    DeserializationError error;
};
Param::Param() {
      //startTime = millis();
      //waitTime = (unsigned long)ms;
      //autoReset = autoreset;
      //Serial.print("Timer serialized "); Serial.print(ms); Serial.print(" - "); Serial.println(startTime);
}
    
/**
 * Read persistent param from EEPROM
 * ---------------------------------
 */
Param::tParam Param::getParam(Param::tParam _param) {
    Serial.println("getParam ");

    byte l;
    char test[128];
    String x;
    byte textPos;
    byte epromPos;
    String digest = DIGEST;
    // tParam * wparam = malloc(sizeof());

    EEPROM.begin(512);

    // read digest
    Serial.print("get digest --");
    l = digest.length();
    for (int n = 0; n < 0 + l; n++) {
        test[n - 0] = EEPROM.read(n);
        Serial.print(test[n - 0]);
    }
    test[l] = '\0';
    x = test;
    //Serial.println("--" + x + "..");

    //Serial.print("read param --");
    if (x == digest) {
        Serial.println("-- digest found");
        l = EEPROM.read(0 + digest.length());
        Serial.print("read param len --");
        Serial.print(l);
        Serial.print("-- param --");
        if (l > 512)
            l = 512;
        textPos = 0;
        epromPos = 0 + digest.length() + 1;
        for (int n = epromPos; n < epromPos + l; n++) {
            test[textPos] = EEPROM.read(n);
            Serial.print(test[textPos]);
            textPos++;
        }
        test[l] = '\0';
        x = test;
        Serial.println("--");
    } else {
        x = "";
    }
    EEPROM.end();

    if (x != "") {
        Serial.println("save json to check later if parameter changed");
        _paramJSON = x;
        Serial.println("deserialise json");
        deserializeJson(doc, x); 
        _param.sSSID = (const char *)doc["sSSID"];
        _param.sWiFiPassword = (const char *)doc["sWiFiPassword"];
        _param.sDayStart = (const char *)doc["sDayStart"];
        _param.sNightStart = (const char *)doc["sNightStart"];
        _param.sDayTemp = (const char *)doc["sDayTemp"];
        _param.sNightTemp = (const char *)doc["sNightTemp"];
        _param.sSwithThreshold = (const char *)doc["sSwithThreshold"];
        Serial.println(_param.sSSID);
    } else {
        Serial.println("-- digest not found");
        _param.sSSID = CSSID;
        _param.sWiFiPassword = WIFIPASSWORD;
        _param.sDayStart = DAYSTART;
        _param.sNightStart = NIGHTSTART;
        _param.sDayTemp = DAYTEMP;
        _param.sNightTemp = NIGHTTEMP;
        _param.sSwithThreshold = SWITCHTHRESHOLD;
    }

    return _param;
}

/**
 * Save param to EEPROM
 * --------------------
 */
bool Param::saveParam(tParam param) {
    Serial.println("saveParam");
    String digest = DIGEST;
    Serial.println("convert param to json");
    doc["sSSID"] = param.sSSID;
    doc["sWiFiPassword"] = param.sWiFiPassword;
    doc["sDayStart"] = param.sDayStart;
    doc["sNightStart"] = param.sNightStart;
    doc["sDayTemp"] = param.sDayTemp;
    doc["sNightTemp"] = param.sNightTemp;
    doc["sSwithThreshold"] = param.sSwithThreshold;
    String buff;
    int paramLength = serializeJson(doc, buff);
    Serial.println("check if json has changed; if not, return");
    if (buff != _paramJSON) {
        _paramJSON = buff;
    } else {
        return false;
    }

    EEPROM.begin(512); // should be: size-of param + size-of digest?

    Serial.print("write digest: ");
    for (int n = 0; n < digest.length(); n++) {
        EEPROM.write(n, digest[n]);
        Serial.print(digest[n]);
    }
    Serial.println();

    Serial.print("write param length: ");
    Serial.print(paramLength);
    Serial.print(" at ");
    Serial.println(digest.length());
    EEPROM.write(digest.length(), (byte)paramLength);

    Serial.print("write param: ");
    for (int n = digest.length() + 1; n < digest.length() + 1 + buff.length(); n++) {
        EEPROM.write(n, buff[n - digest.length() - 1]);
        Serial.print(buff[n - digest.length() - 1]);
    }
    Serial.println();

    Serial.println("eeprom commit");
    EEPROM.commit();
    EEPROM.end();
    return true;
}