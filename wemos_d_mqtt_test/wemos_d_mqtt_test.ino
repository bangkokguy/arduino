#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 
const char* ssid = "MrWhite";
const char* password =  "Feketebikapata1965!";
const char* mqttServer = "frank"; //"m11.cloudmqtt.com";
const int   mqttPort = 1883; //12948;
const char* mqttUser = ""; //"YourMqttUser";
const char* mqttPassword = ""; //"YourMqttUserPassword";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
 
WiFiClient espClient;
PubSubClient client(espClient);

struct tDomoticzIn {
  int                   Battery;
  int                   RSSI;
  const char*           description;
  const char*           dtype;
  const char*           hwid;
  const char*           id;
  int                   idx;
  const char*           name;
  int                   nvalue;
  const char*           stype;
  const char*           svalue1;
  const char*           switchType;
  int                   unit;
  DeserializationError  error;
} domoticzIn;

tDomoticzIn parseJson (char json[] ) { 
  
  StaticJsonDocument<1024> doc;
  tDomoticzIn din;

  din.error = deserializeJson(doc, json);

  if (din.error) {
    Serial.print("deserializeJson() failed ");
    Serial.println(din.error.f_str());
    return din;
  } else Serial.print("deserializeJson() OK ");

   Serial.println();
   din.Battery = doc["Battery"]; // 255
   din.RSSI = doc["RSSI"]; // 12
   din.description = doc["description"]; // ""
   din.dtype = doc["dtype"]; // "Light/Switch"
   din.hwid = doc["hwid"]; // "3"
   din.id = doc["id"]; // "00014052"
   din.idx = doc["idx"]; // 2
   din.name = doc["name"]; // "switch test"
   din.nvalue = doc["nvalue"]; // 0
   din.stype = doc["stype"]; // "Switch"
   din.svalue1 = doc["svalue1"]; // "0"
   din.switchType = doc["switchType"]; // "On/Off"
   din.unit = doc["unit"]; // 1
   return din;
}

void setup() {
 
  Serial.begin(115200);
  delay(500);
  Serial.println();
 
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  Serial.println("successful");

//Initialize time server
  timeClient.begin();
  timeClient.update();
  Serial.println("Time initialized");
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println(" successful");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.publish("domoticz/in", "Hello from ESP8266");
  client.subscribe("domoticz/out");
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  
  char json[1024];
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json[i] = (char)payload [i];
  }
  
  tDomoticzIn dinn = parseJson(json /*payload*/);
  
  Serial.println();
  Serial.println("-----------------------");

  Serial.println(dinn.Battery);
  Serial.println(dinn.RSSI);
  Serial.println(dinn.description);
  Serial.println(dinn.dtype);
  /*Serial.println(dinn.hwid);
  Serial.println(dinn.id);
  Serial.println(dinn.idx);
  Serial.println(dinn.name);
  Serial.println(dinn.nvalue);
  Serial.println(dinn.stype);
  Serial.println(dinn.svalue1);
  Serial.println(dinn.switchType);
  Serial.println(dinn.unit);*/
  //Serial.println(din.error);
  
 
}
 
void loop() {
  client.loop();
}
