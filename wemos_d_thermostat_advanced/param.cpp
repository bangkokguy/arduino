#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "param.h"

#define DEBUG if (1==2)
#define DIGEST "19650604"

DynamicJsonDocument doc(1024);
String _paramJSON;

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
    DEBUG Serial.println("getParam ");

    byte l;
    char test[128];
    String x;
    byte textPos;
    byte epromPos;
    String digest = DIGEST;
    // tParam * wparam = malloc(sizeof());

    EEPROM.begin(512);

    // read digest
    DEBUG Serial.print("get digest --");
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
        DEBUG Serial.println("-- digest found");
        l = EEPROM.read(0 + digest.length());
        DEBUG Serial.print("read param len --");
        DEBUG Serial.print(l);
        DEBUG Serial.print("-- param --");
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
        DEBUG Serial.println("save json to check later if parameter changed");
        _paramJSON = x;
        DEBUG Serial.println("deserialise json");
        deserializeJson(doc, x); 
        _param.sSSID = (const char *)doc["sSSID"];
        _param.sWiFiPassword = (const char *)doc["sWiFiPassword"];
        _param.sDayStart = (const char *)doc["sDayStart"];
        _param.sNightStart = (const char *)doc["sNightStart"];
        _param.sDayTemp = (const char *)doc["sDayTemp"];
        _param.sNightTemp = (const char *)doc["sNightTemp"];
        _param.sSwithThreshold = (const char *)doc["sSwithThreshold"];
        DEBUG Serial.println(_param.sSSID);
    } else {
        DEBUG Serial.println("-- digest not found");
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
bool Param::saveParam(Param::tParam param) {
    DEBUG Serial.println("saveParam");
    String digest = DIGEST;
    DEBUG Serial.println("convert param to json");
    doc["sSSID"] = param.sSSID;
    doc["sWiFiPassword"] = param.sWiFiPassword;
    doc["sDayStart"] = param.sDayStart;
    doc["sNightStart"] = param.sNightStart;
    doc["sDayTemp"] = param.sDayTemp;
    doc["sNightTemp"] = param.sNightTemp;
    doc["sSwithThreshold"] = param.sSwithThreshold;
    String buff;
    int paramLength = serializeJson(doc, buff);
    DEBUG Serial.println("check if json has changed; if not, return");
    if (buff != _paramJSON) {
        _paramJSON = buff;
    } else {
        return false;
    }

    EEPROM.begin(512); // should be: size-of param + size-of digest?

    DEBUG Serial.print("write digest: ");
    for (int n = 0; n < digest.length(); n++) {
        EEPROM.write(n, digest[n]);
        Serial.print(digest[n]);
    }
    DEBUG Serial.println();

    DEBUG Serial.print("write param length: ");
    DEBUG Serial.print(paramLength);
    DEBUG Serial.print(" at ");
    DEBUG Serial.println(digest.length());
    EEPROM.write(digest.length(), (byte)paramLength);

    DEBUG Serial.print("write param: ");
    for (int n = digest.length() + 1; n < digest.length() + 1 + buff.length(); n++) {
        EEPROM.write(n, buff[n - digest.length() - 1]);
        Serial.print(buff[n - digest.length() - 1]);
    }
    DEBUG Serial.println();

    DEBUG Serial.println("eeprom commit");
    EEPROM.commit();
    EEPROM.end();
    return true;
}