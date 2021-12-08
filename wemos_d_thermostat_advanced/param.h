#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

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
    #define NOTEMPDATA 10
    #define MANUALTOGGLETIMEOUT 900000
    #define TIMER_1TIMEOUT 15000
    unsigned long startTime;
    unsigned long waitTime;
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
    tParam getParam(tParam param);
    bool saveParam (tParam param);
};

