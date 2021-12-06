

#include <ArduinoJson.h>
#include <EEPROM.h>

#define DIGEST "19650604"

struct tParam {
  String                sSSID;
  String                sWiFiPassword;
  String                sDayStart;
  String                sNightStart;
  String                sDayTemp;
  String                sNightTemp;
  String                sSwithThreshold;
  DeserializationError  error;
};

DynamicJsonDocument doc(1024);

/**
 * getParam
 * --------
 */
tParam * getParam () {
  Serial.println("getParam ");
  
  byte l;
  char test[128];
  String x;
  byte textPos; byte epromPos;
  String digest = DIGEST;
  tParam wparam;

  EEPROM.begin(512);
  
//read digest
  Serial.print("get digest --");
  l = digest.length();
  for(int n = 0; n < 0 + l; n++){
     test[n - 0] = EEPROM.read(n);
     Serial.print(test[n - 0]);
  }
  test[l] = '\0'; 
  x = test;
  Serial.println("--"+x+"..");

  Serial.print("read param --");
  if (x == digest) {
    Serial.println("-- digest found");
    l = EEPROM.read(0 + digest.length());
    Serial.print("read param len --");
    Serial.print(l);
    Serial.print("-- param --");
    if (l > 512) l = 512;
    textPos = 0;
    epromPos = 0 + digest.length() + 1;
    for(int n = epromPos; n < epromPos + l; n++){
       test[textPos] = EEPROM.read(n);
       Serial.print(test[textPos]);
       textPos++;
    }
    test[l] = '\0'; 
    x = test;
    Serial.println("--");
  } else {
    x = "";
    Serial.println("-- digest not found");
  }
  EEPROM.end();

  Serial.println("deserialise json");
  deserializeJson(doc, x);
  wparam.sSSID           = (const char*)doc["sSSID"];
  wparam.sWiFiPassword   = (const char*)doc["sWiFiPassword"];
  wparam.sDayStart       = (const char*)doc["sDayStart"];
  wparam.sNightStart     = (const char*)doc["sNightStart"];
  wparam.sDayTemp        = (const char*)doc["sDayTemp"];
  wparam.sNightTemp      = (const char*)doc["sNightTemp"];
  wparam.sSwithThreshold = (const char*)doc["sSwithThreshold"];
  
  Serial.println(wparam.sSSID);

  return &wparam;

}
 
/**
 * Save param to EEPROM
 * --------------------
 */
bool saveParam (tParam param) {
  Serial.println("saveParam");
  String digest = DIGEST;
  Serial.println("convert param to json");
  doc["sSSID"]          = param.sSSID;
  doc["sWiFiPassword"]  = param.sWiFiPassword;
  doc["sDayStart"]      = param.sDayStart;
  doc["sNightStart"]    = param.sNightStart;
  doc["sDayTemp"]       = param.sDayTemp;
  doc["sNightTemp"]     = param.sNightTemp;
  doc["sSwithThreshold"]= param.sSwithThreshold;
  String buff;
  int paramLength = serializeJson(doc, buff);
  
  EEPROM.begin(512); //should be: size-of param + size-of digest?

  Serial.print("write digest: ");
  for(int n = 0; n < digest.length() ; n++){
    EEPROM.write(n, digest[n]);
    Serial.print(digest[n]);
  }
  Serial.println();

  Serial.print("write param length: "); Serial.print(paramLength);  Serial.print(" at "); Serial.println(digest.length());
  EEPROM.write(digest.length(), (byte)paramLength);

  Serial.print("write param: "); 
  for(int n = digest.length() + 1; n < digest.length() + 1 + buff.length(); n++){
     EEPROM.write(n, buff[n - digest.length() - 1]);
     Serial.print(buff[n - digest.length() - 1]);
    }
  Serial.println();
  
  Serial.println("eeprom commit");
  EEPROM.commit();
  EEPROM.end();
  return true;
}

/**
 * setup
 * -----
 */
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    /*-------------*/
    tParam a;  
      a.sSSID           = "Faszom";
      a.sSSID           = "MrWhite";
      a.sWiFiPassword   = "Feketebikapata1965!";
      a.sDayStart       = "0700";
      a.sNightStart     = "2300";
      a.sDayTemp        = "23.00";
      a.sNightTemp      = "19.00";
      a.sSwithThreshold = "0.20";
    saveParam(a);
    /*-------------*/
    tParam * b;
    b = getParam();
  
    Serial.println(b->sSSID);
}

void loop() {
}
