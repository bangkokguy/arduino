/**********************************************
 * Catalin Batrinu bcatalin@gmail.com 
 * Read temperature and pressure from BMP280
 * and send it to thingspeaks.com
**********************************************/

#include <Arduino.h>
//#include <USBAPI.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
//#include <ESPAsyncWebServer.h> //randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/
#include <EEPROM.h>

#define BMP_SCK  13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS   10

const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta content=\"text/html; charset=ISO-8859-1\""
" http-equiv=\"content-type\">"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 Web Form Demo</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"<label>ssid:&nbsp;</label>"
"<input maxlength=\"30\" name=\"ssid\"><br>"
"<label>Password:&nbsp;</label><input maxlength=\"30\" name=\"Password\"><br>"
"<label>IP:&nbsp;</label><input maxlength=\"15\" name=\"IP\"><br>"
"<label>Gateway:&nbsp;</label><input maxlength=\"15\" name=\"GW\"><br>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";

Adafruit_BMP280 bme; // I2C
// replace with your channel’s thingspeak API key,
String apiKey = "R9A17PBGUIYZZY3Y";
const char* ssid = "MrWhite";
const char* password = "Feketebikapata1965!";
const char* cserver = "api.thingspeak.com";
const char* digest = "19650604";
String wssid;
String wpassword;
WiFiClient client;

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String day;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
void handleSentVar();
void read_from_Memory (String ssid,String pwd,String ip, String gw);
ESP8266WebServer webserver(80);

#define d0 d[0]
#define d1 d[1]
#define d2 d[2]
#define d3 d[3]
#define d4 d[4]
#define d5 d[5]
#define d6 d[6]
#define d7 d[7]
#define d8 d[8]
#define d9 d[9]

int ii;
int currentTime;
int d9={16, 5, 4, 0, 17, 14, 12, 13, 15};
int t;

void ledON(int led) {digitalWrite(led,(led == LED_BUILTIN)&!HIGH);}
void ledOFF(int led) {digitalWrite(led,(led == LED_BUILTIN)&HIGH);}
void ledPulse (int led, int interval, int blur) {
  int i = interval;
  while (i > 0) {
    ledON(led);
    delay(blur);
    i = i - blur;
    ledOFF(led);
    delay(blur);
    i = i - blur;
    }
  }

/**************************  
 *   S E T U P
 **************************/
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  ledON(LED_BUILTIN);
  delay(2000);
  Serial.println();
  //Serial.println(F("BMP280 test"));
  Serial.println("BMP280 test");
  
  while (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring! ");
    delay (2000);
  }
  Serial.println("BMP280 checked, functioning");

  wssid = readSSID();
  wpassword = readPassword();
  Serial.print(wssid+wpassword);
  Serial.println("Parameters loaded");

  Serial.print("Connecting to ");
  Serial.println(wssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wssid, wpassword);
  
  int trials = 15;
  while (WiFi.status() != WL_CONNECTED && trials > 0) {
    delay(500);
    Serial.print(".");
    trials--;
    }
  Serial.println("");
  if (trials == 0) {
    Serial.println("WiFi timeout reached");
    WiFi.softAP("ESP_19650604");
  } else {
    Serial.println("WiFi connected");  
  }

  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  ledOFF(LED_BUILTIN);

  timeClient.begin();
  timeClient.update();
  currentTime = timeClient.getMinutes();
  t = timeClient.getSeconds();
  Serial.println("Time initialized");

  webserver.on("/", handleRoot);
  webserver.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  webserver.onNotFound(handleNotFound);
  webserver.begin();
}

void sendTempData () {
  WiFiClient client;
  Serial.print("Connecting to 192.168.1.228:8080...");
  if (client.connect("192.168.1.228",8080))  {// "184.106.153.149" or api.thingspeak.com 
    Serial.println("successful");
    String postStr = "";
    postStr +="?temp={idx:1,nvalue:";
    postStr += "1";
    postStr +=",svalue:";
    postStr += String(bme.readTemperature()); //String(bme.readPressure());
    postStr += "}";
    
    //\r\n\r\n";
    
    Serial.print("Sending data to server...");
    String headerStr = "POST /temp" + postStr + " HTTP/1.1\n";
    client.print(headerStr);
    //client.print("POST /temp?temp={idx:1,nvalue:0,svalue:17.3} HTTP/1.1\n");
    client.print("Host: 192.168.1.228\n");
    client.print("Connection: close\n");
    //client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);   
    Serial.println("finished"); 
  } else {
    Serial.println("failed");
  } 
  client.stop(); 
}
  
/**************************  
 *  L O O P
 **************************/
void loop() {
  ledON(LED_BUILTIN);
  Serial.print("T=");
  Serial.print(bme.readTemperature());
  Serial.print(" *C");
  
  Serial.print(" P=");
  Serial.print(bme.readPressure());
  Serial.print(" Pa");

  Serial.print(" A= ");
  Serial.print(bme.readAltitude(1013.25)); // this should be adjusted to your local forcase
  Serial.println(" m");

  Serial.print("Connecting to server...");
  if (client.connect(cserver,80))  {// "184.106.153.149" or api.thingspeak.com 
    Serial.println("successful");
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(bme.readTemperature());
    postStr +="&field2=";
    postStr += String(bme.readPressure());
    postStr += "\r\n\r\n";
    
    Serial.print("Sending data to server...");
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);   
    Serial.println("finished"); 
  } else {
    Serial.println("failed");
  } 
  client.stop(); 

  delay (50);
  sendTempData ();
  
  ledOFF(LED_BUILTIN);
  //every 20 sec
  int i = 20;
  while (i>0) {
    //Serial.printf("Stations connected to soft-AP = %d\n", WiFi.softAPgetStationNum());
    webserver.handleClient();
    delay(5000);
    timeClient.update();

    if (timeClient.getHours() >=7 && timeClient.getHours() < 22)
      ledPulse(LED_BUILTIN, 200, 40);
    else
      delay(10000);
      
    i--;
  }
}
  
/**
 * Write data to eprom
*/
void write_EEPROM(String x,int pos){
  Serial.println(x.length()+1);
  byte l = x.length();
  EEPROM.write(pos, l);
  for(int n = pos + 1; n < x.length() + 1 + pos + 1; n++){
     EEPROM.write(n, x[n-pos - 1]);
     Serial.print(x[n-pos - 1]);
    }
  Serial.println("--");
  }

/**
 * Write data to memory
 * 
 * We prepping the data strings by adding the end of line symbol I decided to use ";". 
 * Then we pass it off to the write_EEPROM function to actually write it to memmory
 */
void write_to_Memory(String ssid,String pwd,String ip, String gw) {
  EEPROM.begin(512);
  write_EEPROM(digest,0);
  ssid+="\0";
  write_EEPROM(ssid,10);
  pwd+="\0";
  write_EEPROM(pwd,100);
  ip+="\0";
  write_EEPROM(ip,200); 
  gw+="\0";
  write_EEPROM(gw,220); 
  Serial.println("eeprom commit");
  EEPROM.commit();
  EEPROM.end();
  }

/**
 * Read data from eprom
*/
String read_EEPROM(int pos) {
  byte l;
  if (pos == 0) l = 8; else l = EEPROM.read(pos);
  char test[128];
  String x;
  for(int n = pos + 1; n < l + pos + 1; n++){
     test[n-pos-1] = EEPROM.read(n);
  }
  test[l] = '\0'; 
  x = test;
  return x;
}

String readSSID () {
  EEPROM.begin(512);
  String dig = read_EEPROM(0);
  Serial.print("digest=");
  Serial.println(dig);
  if (dig == digest) {
    return read_EEPROM(10);
  } else {
    Serial.println("No default SSID stored");  
    return ssid;
  }
  EEPROM.end();
}

String readPassword () {
  EEPROM.begin(512);
  String dig = read_EEPROM(0);
  Serial.print("digest=");
  Serial.println(dig);
  if (dig == digest) {
    return read_EEPROM(100);
  } else {
    Serial.println("No default Password stored");  
    return password;
  }
  EEPROM.end();
}

void read_from_Memory (String ssid,String pwd,String ip, String gw) {
  Serial.println("in memory read");
  EEPROM.begin(512);
  String dig = read_EEPROM(0);
  Serial.print("digest=");
  Serial.println(dig);
  if (dig == digest) {
    Serial.println(read_EEPROM(10));
    Serial.println(read_EEPROM(100));
    Serial.println(read_EEPROM(200));
    Serial.println(read_EEPROM(220));
  } else {
    Serial.println("No default parameters stored");  
  }
  EEPROM.end();
}

void handleSubmit(){//dispaly values and write to memmory
  String response="<p>The ssid is ";
  response +=webserver.arg("ssid");
  response +="<br>";
  response +="And the password is ";
  response +=webserver.arg("Password");
  response +="<br>";
  response +="And the IP Address is ";
  response +=webserver.arg("IP");
  response +="</P><BR>";
  response +="<H2><a href=\"/\">go home</a></H2><br>";

  webserver.send(200, "text/html", response);
  //calling function that writes data to memory 
  write_to_Memory(String(webserver.arg("ssid")),String(webserver.arg("Password")),String(webserver.arg("IP")),String(webserver.arg("GW")));
}

void handleRoot() {
  /*String h = HTTPHEAD;
  String f = HTTPFOOT;
  String s ="<CENTER><B>" + day + "</B></CENTER>";

  server.send(200, "text/html", s);   // Send HTTP status 200 (Ok) and send some text to the browser/client*/
  
  if (webserver.hasArg("ssid")&& webserver.hasArg("Password")&& webserver.hasArg("IP")&& webserver.hasArg("GW") ) {//If all form fields contain data call handelSubmit()
    handleSubmit();
  } else {
    //Redisplay the form
    webserver.send(200, "text/html", INDEX_HTML);
  }
}

void handleNotFound(){
  webserver.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleSentVar() {
  Serial.println("handleSentVar");
  if (webserver.hasArg("sensor_reading")) { // this is the variable sent from the client
//    int readingInt = server.arg("sensor_reading").toInt();
//    char readingToPrint[5];
//    itoa(readingInt, readingToPrint, 10); //integer to string conversion for OLED library
//    u8g2.firstPage();
//    u8g2.drawUTF8(0, 64, readingToPrint);
//    u8g2.nextPage();
    webserver.send(200, "text/html", "<h1>Data received</>");
  } else
    webserver.send(200, "text/html", "<h1>Data could not interpreted</>");
}
