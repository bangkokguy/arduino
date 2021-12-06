/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266OTA.h>

const char* host = "es";
const char* ssid = "-MrWhite";
const char* password = "Feketebikapata1965!";

ESP8266WebServer httpServer(80);
ESP8266OTA otaUpdater;

void setup(void){

  Serial.begin(115200);
  delay(500);
  
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int trials = 15;
  while (WiFi.status() != WL_CONNECTED && trials > 0) {
    delay(500);
    Serial.print(".");
    trials--;
    }
  if (trials == 0) {
    Serial.println("WiFi timeout reached");
      WiFi.mode(WIFI_AP);
    WiFi.softAP("WD1_MDNS_1");
  } else {
    Serial.println("WiFi connected");  
  }
   
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);


  MDNS.begin(host);
  
  //set web UI
  otaUpdater.setUpdaterUi("Title","Banner","Build : 0.01","Branch : master","Device info : ukn","footer");

  //setup web UI , with chip ID auto generated
  //otaUpdater.setUpdaterUi("Title","Banner","Build : 0.01","Branch : master","footer");

  otaUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void loop(void){
//  MDNS.loop();
  httpServer.handleClient();
}
