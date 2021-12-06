
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//Creating the input form
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

const char *ssid     = "MrWhite";
const char *password = "Feketebikapata1965!";
WiFiUDP ntpUDP;

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String day;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
void handleSentVar();
ESP8266WebServer server(80);

int ii;
int currentTime;
int d[9]={16, 5, 4, 0, 17, 14, 12, 13, 15};
int t;

/**
 * Setup
 */
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(D0, 'x ');
  Serial.print(D1, 'x ');
  Serial.print(D2, 'x ');
  Serial.print(D3, 'x ');
  Serial.print(D4, 'x ');
  Serial.print(D5, 'x ');
  Serial.print(D6, 'x ');
  Serial.print(D7, 'x ');
  Serial.println(LED_BUILTIN);
  for (int j=0;j<=8;j++) {
    Serial.print(j, 'y ');
    pinMode(d[j], OUTPUT);
    }
  Serial.println();
  ii = 0;          //0=D3 4=D2 5=D1 12=D6 13=D7 14=D5 15=D8 16=D0 17=D4
                  // 0 1 2 3  4  5  6  7  8
                  //16 5 4 0 17 14 12 13 15
                  // 4 5 10 12 13 14 15 16 19 20 21 22
  
//  WiFi.mode(STA);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.print("connected at ");
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  
  timeClient.begin();
  timeClient.update();
  currentTime = timeClient.getMinutes();
  t = timeClient.getSeconds();

  server.on("/", handleRoot);
  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.onNotFound(handleNotFound);
  server.begin();

  read_from_Memory (" 1", " 2", " 3", " 4");

}

/**
 * loop
 */
void loop() {
  timeClient.update();
  t = timeClient.getSeconds();
  if (abs (t - currentTime) >=5) {
    currentTime = t;
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(" ");
    Serial.println(timeClient.getFormattedTime());
    }


  if (timeClient.getHours() >=7 && timeClient.getHours() < 22) {
    digitalWrite(d[ii], HIGH);  delay(70);
    digitalWrite(d[ii], LOW);   delay(70);        
  }
  
  if (ii==8) ii = 0; else ii++;
  
  server.handleClient();
  //delay(1000);
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
void write_to_Memory(String ssid,String pwd,String ip, String gw){
  EEPROM.begin(512);
  ssid+="\0";
  write_EEPROM(ssid,0);
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
String read_EEPROM(int pos){
  byte l = EEPROM.read(pos);
  char test[128];
  String x;
  for(int n = pos + 1; n < l + pos + 1; n++){
     test[n-pos-1] = EEPROM.read(n);
  }
  test[l] = '\0'; 
  x = test;
  return x;
}

void read_from_Memory (String ssid,String pwd,String ip, String gw) {
  EEPROM.begin(512);
  Serial.println(read_EEPROM(0));
  Serial.println(read_EEPROM(100));
  Serial.println(read_EEPROM(200));
  Serial.println(read_EEPROM(220));
  EEPROM.end();
}

void handleSubmit(){//dispaly values and write to memmory
  String response="<p>The ssid is ";
  response += server.arg("ssid");
  response +="<br>";
  response +="And the password is ";
  response +=server.arg("Password");
  response +="<br>";
  response +="And the IP Address is ";
  response +=server.arg("IP");
  response +="</P><BR>";
  response +="<H2><a href=\"/\">go home</a></H2><br>";

  server.send(200, "text/html", response);
  //calling function that writes data to memory 
  write_to_Memory(String(server.arg("ssid")),String(server.arg("Password")),String(server.arg("IP")),String(server.arg("GW")));
}

void handleRoot() {
  /*String h = HTTPHEAD;
  String f = HTTPFOOT;
  String s ="<CENTER><B>" + day + "</B></CENTER>";

  server.send(200, "text/html", s);   // Send HTTP status 200 (Ok) and send some text to the browser/client*/
  
  if (server.hasArg("ssid")&& server.hasArg("Password")&& server.hasArg("IP")&&server.hasArg("GW") ) {//If all form fields contain data call handelSubmit()
    handleSubmit();
  } else {
    //Redisplay the form
    server.send(200, "text/html", INDEX_HTML);
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleSentVar() {
  Serial.println("handleSentVar");
  if (server.hasArg("sensor_reading")) { // this is the variable sent from the client
//    int readingInt = server.arg("sensor_reading").toInt();
//    char readingToPrint[5];
//    itoa(readingInt, readingToPrint, 10); //integer to string conversion for OLED library
//    u8g2.firstPage();
//    u8g2.drawUTF8(0, 64, readingToPrint);
//    u8g2.nextPage();
    server.send(200, "text/html", "<h1>Data received</>");
  } else
    server.send(200, "text/html", "<h1>Data could not interpreted</>");
}
