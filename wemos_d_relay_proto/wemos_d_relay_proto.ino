#define SELF "WD1THERMOSTAT"
#define relayPin d5
#define ledPin d6

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define d0 16
#define d1 5
#define d2 4
#define d3 0
#define d4 17
#define d5 14
#define d6 12
#define d7 13
#define d8 15

#define cSSID 0
#define cPWD 36
#define cIP 72
#define cGW 108
#define cFROMHOUR 144
#define cTOHOUR 180
#define cMAXLEN 30
#define NIL "\0"

String digest = "19650604";
#define cDIGESTLEN 8

const char* ssid = "MrWhite";  // Enter SSID here
const char* password = "Feketebikapata1965!";  //Enter Password here
const char* mqttServer = "frank"; //"m11.cloudmqtt.com";
const int   mqttPort = 1883; //12948;
const char* mqttUser = ""; //"YourMqttUser";
const char* mqttPassword = ""; //"YourMqttUserPassword";

const int   fromHour = 7;
const int   toHour = 20;

String wdigest;
String wssid;
String wpassword;
String wip;
String wgw;
String wfromHour;
String wtoHour;

bool switchOn = false;
bool timeToOn = false;
bool remoteEnabled = false;

int wait = 5000;
unsigned long pastMillis = millis();
unsigned long currentMillis;
long pastTime;

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

ESP8266WebServer server(8080);

WiFiClient espClient;
PubSubClient client(espClient);
struct tDomoticzIn {
  int                   Battery;
  int                   Color;
  int                   Level;
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

/**
 * parseJson - parse incoming MQTT json message
 * --------------------------------------------
*/
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
   din.Battery      = doc["Battery"]; 
   din.Color        = doc["Color"];
   din.Level        = doc["Level"];
   din.RSSI         = doc["RSSI"]; 
   din.description  = doc["description"]; 
   din.dtype        = doc["dtype"]; 
   din.hwid         = doc["hwid"]; 
   din.id           = doc["id"];
   din.idx          = doc["idx"];
   din.name         = doc["name"];
   din.nvalue       = doc["nvalue"];
   din.stype        = doc["stype"];
   din.svalue1      = doc["svalue1"];
   din.switchType   = doc["switchType"];
   din.unit         = doc["unit"];
   return din;
}

/**
 * setParam - Write data to eprom
 * Format : 8 bytes of digest, 1 byte length, length bytes parameter
 * -----------------------------------------------------------------
*/
void setParam (String parm, int pos) {
  Serial.println("write_to_Memory" + digest);
  EEPROM.begin(512);

//write digest from pos to pos + digest.len - 1
  Serial.print("write digest: ");
  for(int n = pos; n < pos + digest.length() ; n++){
    EEPROM.write(n, digest[n-pos]);
    Serial.print(digest[n-pos]);
  }
  Serial.println();

//write lenght of parameter
  Serial.print("write param length: --");
  byte l = parm.length(); 
  Serial.print(l); Serial.print("-- to --");
  EEPROM.write(pos + digest.length(), l);
  Serial.print(pos + digest.length()); Serial.println("..");

//write param
  Serial.print("param: ");
  for(int n = pos + digest.length() + 1; n < pos + digest.length() + 1 + parm.length(); n++){
     EEPROM.write(n, parm[n - pos - digest.length() - 1]);
     Serial.print(parm[n - pos - digest.length() - 1]);
    }
  Serial.println("--");
  Serial.println("eeprom commit");
  EEPROM.commit();
  EEPROM.end();
}

/**
 * getParam - Read data from eprom
 * -------------------------------
 */
String getParam (int pos) {
  EEPROM.begin(512);
  Serial.print("getParameter "); Serial.println(pos);

  byte l;
  char test[128];
  String x;
  byte textPos; byte epromPos;

//read digest
  Serial.print("get digest --");
  l = cDIGESTLEN;
  for(int n = pos; n < pos + l; n++){
     test[n - pos] = EEPROM.read(n);
     Serial.print(test[n - pos]);
  }
  test[l] = '\0'; 
  x = test;
  Serial.println("--"+x+"..");

//read param
  Serial.print("read param --");
  if (x == digest) {
    Serial.println("-- digest found");
    l = EEPROM.read(pos + cDIGESTLEN);
    Serial.print("read param len --");
    Serial.print(l);
    Serial.print("-- param --");
    if (l > cMAXLEN) l = cMAXLEN;
    textPos = 0;
    epromPos = pos + cDIGESTLEN + 1;
    for(int n = epromPos; n < epromPos + l; n++){
       test[textPos] = EEPROM.read(n);
       Serial.print(test[textPos]);
       textPos++;
    }
    test[l] = '\0'; 
    x = test;
    Serial.println("--");
  } else {
    x = NIL;
    Serial.println("-- digest not found");
  }
  return x;

  EEPROM.end();
}

/**
 * setup
 * -----
 */
void setup() {
//Prepare environment
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ledPin, HIGH);
  
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(ledPin, LOW);



//Read persistent setup data
  wssid = getParam (cSSID); 
  if (wssid == NIL) wssid = ssid;
  wpassword = getParam(cPWD); 
  if (wpassword == NIL) wpassword = password;
  wfromHour = getParam(cFROMHOUR); 
  if (wfromHour == NIL) wfromHour = fromHour;
  wtoHour = getParam(cTOHOUR); //== NIL ? "hihi" : "haha" ;;
  if (wtoHour == NIL) wtoHour = toHour;
  Serial.print(wssid+" "+wpassword+" "+wfromHour+" "+wtoHour+"... ");
  Serial.println("Parameters loaded");

//Connect to WiFi; If no wifi connection could be established, then act as an AP;
  Serial.print("Connecting to ");
  Serial.print(wssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wssid, wpassword);
  int trials = 15;
  while (WiFi.status() != WL_CONNECTED && trials > 0) {
    delay(500);
    Serial.print(".");
    trials--;
    }
  if (trials == 0) {
    Serial.println("WiFi timeout reached");
      WiFi.mode(WIFI_AP);
    WiFi.softAP("WD1_RELAY_1");
  } else {
    Serial.println("WiFi connected");  
  }
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);

//Initialize time server
  timeClient.begin();
  timeClient.update();
  Serial.println("Time initialized");

//Initialize local web server
  server.on("/", handleRoot);
  server.on("/setup/", handleSetup);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

//Initialize MQTT connection
  client.setServer(mqttServer, mqttPort);
  client.setBufferSize(1024);
  client.setCallback(callback);
  
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
 
    if (   client.connect(SELF, nullptr, nullptr, "domoticz", 0, true, "", false)
          /*client.connect("relay proto", mqttUser, mqttPassword )*/) {
 
      Serial.println(" successful"); 
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.publish("domoticz/in", "{ \"idx\" : 1, \"nvalue\" : 0, \"svalue\" : \"25.0\" }");
  client.subscribe("domoticz/out");
  Serial.println("MQTT connection established");

}

/**
 * mqttCallback
 * ------------
 */
void callback(char* topic, byte* payload, unsigned int length) {
  
  char json[1024];
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json[i] = (char)payload [i];
  }
  Serial.println();
  
  tDomoticzIn dinn = parseJson(json);

  if (dinn.error)
    Serial.println("Couldn't parse message");
  else {
    Serial.print("evaluate dinn.id ");
    String id = dinn.id;
    String svalue1 = dinn.svalue1;
    if (id == "00014053") {
      Serial.println("Switch toggled");
      switchOn = svalue1 == "20";
      remoteEnabled = svalue1 > "0";
      Serial.println(switchOn);
      //alter delay registers in order to make the changes taking effect immediately
      if (pastMillis >= wait) pastMillis = pastMillis - wait;
      //client.publish("domoticz/in", "{ \"idx\" : 1, \"nvalue\" : 0, \"svalue\" : \"25.0\" }");
    }
    
    
    Serial.println();
    Serial.println("-----------------------");
  
    /*Serial.println(dinn.Battery);
    Serial.println(dinn.Color);
    Serial.println(dinn.Level);
    Serial.println(dinn.RSSI);
    Serial.println(dinn.description);
    Serial.println(dinn.dtype);
    Serial.println(dinn.hwid);
    Serial.println(dinn.id);
    Serial.println(dinn.idx);
    Serial.println(dinn.name);
    Serial.println(dinn.nvalue);
    Serial.println(dinn.stype);
    Serial.println(dinn.svalue1);
    Serial.println(dinn.switchType);
    Serial.println(dinn.unit);*/
  }
}

/**
 * loop
 * ----
 */
void loop() {

  server.handleClient();

  currentMillis = millis(); 

  if (currentMillis > wait) pastTime = currentMillis - wait; else pastTime = 0;
  if (pastTime > pastMillis) {
    //Serial.print("currentMillis:"); Serial.print(currentMillis);
    //Serial.print(" pastMillis:"); Serial.print(pastMillis);
    //Serial.print(" wait:"); Serial.print(wait);
    //Serial.print(" currentMillis-wait:"); Serial.println(pastTime);

    pastMillis = millis();
    timeClient.update();
    //Serial.println(timeClient.getFormattedTime());
    //Serial.println(timeClient.getHours());
    //Serial.println(wfromHour);

    timeToOn = ((timeClient.getHours() >= wfromHour.toInt()) && (timeClient.getHours() < wtoHour.toInt()));
    
    if ((remoteEnabled && switchOn) || (!remoteEnabled && timeToOn)) {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(ledPin, HIGH);
      digitalWrite(relayPin, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(ledPin, LOW);
      digitalWrite(relayPin, LOW);
    }
  }
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT...");
    Serial.println(
      client.connect(
        SELF,
        nullptr,
        nullptr,
        "domoticz",
        0,
        true,
        "",
        false));//client.connect("relay proto", mqttUser, mqttPassword ));
    delay(100);
    client.setBufferSize(1024);
    client.setCallback(callback);
    client.publish("domoticz/in", "{ \"idx\" : 1, \"nvalue\" : 0, \"svalue\" : \"25.0\" }");
    client.subscribe("domoticz/out");
  }
  client.loop();
}

void handleRoot() {
  Serial.println("handleRoot");
  timeClient.update();

  if (server.hasArg("from")&& server.hasArg("to") ) {
    Serial.println("handle root submit");
    handleRootSubmit();
  } else {
    //Redisplay the form
    Serial.println("redisplay root");
    server.send(200, "text/html", SendHTML(fromHour, toHour)); 
  }
}

void handleNotFound(){
  Serial.println("handleNotFound");
  server.send(404, "text/plain", "Not found");
}


void handleRootSubmit(){//dispaly values and write to memmory
  Serial.println("handleRootSubmit");
  String response="<p>\n";
  response +="from hour is "; response +=server.arg("from");      response +="<br>\n";
  response +="to hour is ";   response +=server.arg("to");

  response +="</P><BR>\n";
  response +="<H2><a href=\"/\">go home</a></H2><br>\n";

  server.send(200, "text/html", response);
  
  //refresh data with the new values and persist them to eprom
  wfromHour = server.arg("from");
  wtoHour = server.arg("to");
  setParam(wfromHour, cFROMHOUR);
  setParam(wtoHour, cTOHOUR);

  //alter delay registers in order to make the changes taking effect immediately
  if (pastMillis >= wait) pastMillis = pastMillis - wait;

  //write_to_Memory(String(server.arg("ssid")),String(server.arg("Password")),String(server.arg("IP")),String(server.arg("GW")));
}

void handleSubmit(){//dispaly values and write to memmory
  Serial.println("handleSubmit");
  String response="<p>\n";
  response +="ssid is ";      response +=server.arg("ssid");      response +="<br>\n";
  response +="password is ";  response +=server.arg("Password");  response +="<br>\n";
  response +="IP is ";        response +=server.arg("IP");        response +="<br>\n";
  response +="GW is ";        response +=server.arg("GW");        response +="<br>\n";
  response +="from hour is "; response +=wfromHour;               response +="<br>\n";
  response +="to hour is ";   response +=wtoHour;

  response +="</P><BR>\n";
  response +="<H2><a href=\"/\">go home</a></H2><br>\n";

  server.send(200, "text/html", response);
  
  //calling function that writes data to memory 
  setParam(String(server.arg("ssid")), cSSID);
  setParam(String(server.arg("Password")), cPWD);
  setParam(String(server.arg("IP")), cIP);
  setParam(String(server.arg("GW")), cGW);
  //
  ESP.restart();
  //write_to_Memory(String(server.arg("ssid")),String(server.arg("Password")),String(server.arg("IP")),String(server.arg("GW")));
}

const char INDEX_HTML[] =
"<!DOCTYPE HTML>\n"
"<html>\n"
"<head>\n"
"<meta content=\"text/html; charset=ISO-8859-1\"\n"
" http-equiv=\"content-type\">\n"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">\n"
"<title>ESP8266 Web Form Demo</title>\n"
"<style>\n"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h1>ESP8266 Web Form Demo</h1>\n"
"<FORM action=\"/setup/\" method=\"post\">\n"
"<P>\n"
"<label>ssid:&nbsp;</label>\n"
"<input maxlength=\"30\" name=\"ssid\"><br>\n"
"<label>Password:&nbsp;</label><input maxlength=\"30\" name=\"Password\"><br>\n"
"<label>IP:&nbsp;</label><input maxlength=\"15\" name=\"IP\"><br>\n"
"<label>Gateway:&nbsp;</label><input maxlength=\"15\" name=\"GW\"><br>\n"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">\n"
"</P>\n"
"</FORM>\n"
"</body>\n"
"</html>\n";
void handleSetup() {
  Serial.println("handleSetUp");
  /*String h = HTTPHEAD;
  String f = HTTPFOOT;
  String s ="<CENTER><B>" + day + "</B></CENTER>";

  server.send(200, "text/html", s);   // Send HTTP status 200 (Ok) and send some text to the browser/client*/
  
  Serial.println(server.hasArg("ssid"));
  Serial.println(server.hasArg("Password"));
  Serial.println(server.hasArg("IP"));
  Serial.println(server.hasArg("GW"));
  if (server.hasArg("ssid")&& server.hasArg("Password")&& server.hasArg("IP")&& server.hasArg("GW") ) {//If all form fields contain data call handelSubmit()
    Serial.println("handleSubmit call");
    handleSubmit();
  } else {
    //Redisplay the form
    Serial.println("INDEX_HTML");
    server.send(200, "text/html", INDEX_HTML);
  }
}

String SendHTML(int fromHour,int toHour){
  Serial.println("sendHTML");
  String ptr = "<!DOCTYPE HTML>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "<meta content=\"text/html; charset=ISO-8859-1\"\n";
  ptr += " http-equiv=\"content-type\">\n";
  ptr += "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">\n";
  ptr += "<title>relay 1 switch times</title>\n";
  ptr += "<style>\n";
  ptr += "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Enter switch times and press submit</h1>\n";
  ptr += "<FORM action=\"/\" method=\"post\">\n";
  ptr += "<P>\n";
  ptr += "<label>from:&nbsp;</label>\n";
  ptr += "  <input maxlength=\"30\" name=\"from\"><br>\n";
  ptr += "<label>to:&nbsp;</label>";
  ptr += "  <input maxlength=\"30\" name=\"to\"><br>\n";
  ptr += "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">\n";
  ptr += "</P>\n";
  ptr += "</FORM>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
//------------
  /*String ptr = "<!DOCTYPE html>";
  ptr +="<html>\n";
  ptr +="<head>\n";
  ptr +="<title>ESP8266 Weather Station</title>\n";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>\n";
  ptr +="<style>\n";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}\n";
  ptr +="body{margin: 0px;} \n";
  ptr +="h1 {margin: 50px auto 30px;} \n";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}\n";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}\n";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}\n";
  ptr +=".temperature .reading{color: #F29C1F;}\n";
  ptr +=".humidity .reading{color: #3B97D3;}\n";
  ptr +=".pressure .reading{color: #26B99A;}\n";
  ptr +=".altitude .reading{color: #955BA5;}\n";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}\n";
  ptr +=".data{padding: 10px;}\n";
  ptr +=".container{display: table;margin: 0 auto;}\n";
  ptr +=".icon{width:65px}\n";
  ptr +="</style>\n";
  ptr +="<script type = 'text/JavaScript'>\n";
  ptr +="function AutoRefresh( t ) {\n";
  ptr +="setTimeout(function(){window.location = window.location}, t);\n";
  //setTimeout("location.reload(true);", t);
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="</head>\n";
  //ptr +="<body   onload = 'JavaScript:AutoRefresh(50000);'>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Weather Station</h1>\n";
  ptr +="<h2>"+mTime+"</h2>\n";
  ptr +="<div class='container'>\n";
  ptr +="<div class='data temperature'>\n";
  ptr +="<div class='side-by-side icon'>\n";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>\n";
  ptr +="</div>\n";
  ptr +="<div class='side-by-side text'>Temperature</div>\n";
  ptr +="<div class='side-by-side reading'>\n";
  ptr +=(int)temperature;
  ptr +="<span class='superscript'>&deg;C</span></div>\n";
  ptr +="</div>\n";
  ptr +="<div class='data humidity'>\n";
  ptr +="<div class='side-by-side icon'>\n";
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>\n";
  ptr +="</div>\n";
  ptr +="<div class='side-by-side text'>Humidity</div>\n";
  ptr +="<div class='side-by-side reading'>\n";
  ptr +=(int)humidity;
  ptr +="<span class='superscript'>%</span></div>\n";
  ptr +="</div>\n";
  ptr +="<div class='data pressure'>\n";
  ptr +="<div class='side-by-side icon'>\n";
  ptr +="<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424";
  ptr +="c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25";
  ptr +="c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804";
  ptr +="c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178";
  ptr +="C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814";
  ptr +="c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05";
  ptr +="C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>\n";
  ptr +="</div>\n";
  ptr +="<div class='side-by-side text'>Pressure</div>\n";
  ptr +="<div class='side-by-side reading'>\n";
  ptr +=(int)pressure;
  ptr +="<span class='superscript'>hPa</span></div>\n";
  ptr +="</div>\n";
  ptr +="<div class='data altitude'>\n";
  ptr +="<div class='side-by-side icon'>\n";
  ptr +="<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902";
  ptr +="c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004";
  ptr +="c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994";
  ptr +="C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0";
  ptr +="c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004";
  ptr +="C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813";
  ptr +="C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>\n";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Altitude</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)altitude;
  ptr +="<span class='superscript'>m</span></div>";
  ptr +="</div>";
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";*/
  return ptr;
}
