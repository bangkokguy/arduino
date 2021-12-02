//#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <string>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include "timer.h"

/*---------------------------*/
/*---------------------------*/
#define SELF "thermostat1"
/*---------------------------*/
/*---------------------------*/

/**
 * Default values
 * --------------
 */
#define DIGEST "19650604"
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

/**
* Other constants 
* ---------------
*/
#define d0 16
#define d1 5
#define d2 4
#define d3 0
#define d4 17
#define d5 14
#define d6 12
#define d7 13
#define d8 15
#define relayPin d5
#define switchPin d8

//#define ledPin LED_BUILTIN
#define ledPin d6

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 0
#define utcOffsetInSeconds 3600
#define DAYSOFTHEWEEK                                                            \
  {                                                                              \
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" \
  }
#define mqttServer "frank" //"m11.cloudmqtt.com"
#define mqttPort 1883      //12948;
#define mqttUser ""
#define mqttPassword ""
#define MQTTKEEPALIVE 3600
#define dstOffsetInSeconds 3600

/**
 * Persistent global parameters
 * ----------------------------
 */
struct tParam
{
  String sSSID;
  String sWiFiPassword;
  String sDayStart;
  String sNightStart;
  String sDayTemp;
  String sNightTemp;
  String sSwithThreshold;
  DeserializationError error;
} _param;
String _paramJSON;

/**
 * Working variables
 * -----------------
 */
struct t_date
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} lastDate;

struct tTempIn
{
  int idx;
  int nvalue;
  float svalue;
  DeserializationError error;
};

bool bDay;
float fCurrentTemp = 23.45;
int iManualToggleState = 2;
//int                     iManualToggleTimeOut = 40;
//int                     iNoTempDataTimeOut = 4000;
int iPhysicalSwitchState = 0;
int iPrevSwitchState;
int iRelayState = 0;
String sIP;
char daysOfTheWeek[7][12] = DAYSOFTHEWEEK;
DynamicJsonDocument doc(1024);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
ESP8266WebServer WEBServer(8080);
WiFiClient espClient;
PubSubClient MQTTClient(espClient);
Adafruit_SSD1306 t1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



Timer timer_1(15000, AUTORESET);
Timer timer_manualToggle(MANUALTOGGLETIMEOUT, AUTORESET);
Timer timer_noData(NOTEMPDATATIMEOUT, NOAUTORESET);
Timer t100(100, AUTORESET);
Timer t150(150, AUTORESET);
Timer t120(200, AUTORESET);

/**
 * handlePhysicalSwitch
 * ------------------
 */
void handlePhysicalSwitch()
{
  iPhysicalSwitchState = digitalRead(switchPin);
  if (iPhysicalSwitchState != iPrevSwitchState)
  {
    iManualToggleState++;
    if (iManualToggleState > 2)
      iManualToggleState = 0;
    timer_1.flush();
  }
  iPrevSwitchState = iPhysicalSwitchState;
}



/**
 * statusDisplay
 */
void statusDisplay()
{
  Serial.println("statusDisplay");
  timeClient.update();
  t1.setTextSize(1);
  t1.clearDisplay();
  t1.setCursor(36, 20);
  t1.println(timeClient.getFormattedTime());
  t1.setTextSize(2);
  t1.setCursor(36, 34);
  t1.println(String(fCurrentTemp));

  t1.setTextSize(1);
  t1.setCursor(36, 54);

  if (iManualToggleState == 0)
    t1.print("X");
  else if (iManualToggleState == 1)
    t1.print("O");
  else
    t1.print("A");
  //  t1.drawBitmap(46, 54, clock_icon16x16, 16, 16, 1);

  if (bDay)
    t1.print("A " + _param.sDayTemp);
  else
    t1.print("N " + _param.sNightTemp);

  t1.invertDisplay(timer_noData.timeout()); //t1.println(" x"); else t1.println();
  t1.dim(true);

  //t1.startscrollright(34, 52);

  t1.display();

  String m = "";
  Serial.println("****************************");
  m = "* " + timeClient.getFormattedTime();
  Serial.println(m);
  m =
      "* " +
      String(lastDate.year) +
      "-" +
      String(lastDate.month) +
      "-" +
      String(lastDate.day) +
      " " +
      String(lastDate.hour) +
      ":" +
      String(lastDate.minute) +
      ":" +
      String(lastDate.second);
  Serial.println(m);
  Serial.println("* " + String(fCurrentTemp));
  if (iManualToggleState == 0)
    Serial.println("* off ");
  else if (iManualToggleState == 2)
    Serial.println("* auto");
  else
  {
    Serial.print("* on for ");
    Serial.println(timer_manualToggle.remaining());
  }

  if (timer_noData.timeout())
    Serial.println("* no temp data received for a while");
  if (iRelayState == 1)
    Serial.println("* Heating ON");
  else
    Serial.println("* Heating OFF");
  if (iPhysicalSwitchState == 1)
    Serial.println("* Physical sw ON");
  else
    Serial.println("* Physical sw OFF");
  Serial.println("****************************");
}

/**
 * Alarm
 * -----
 * pattern
 * 1: display init error
 * 2: wifi connect error
 * 3: time server init error
 * 4: web server init error
 * 5: mqtt init error
 * 10: no data time out
 */
void alarm(int pattern)
{
  Serial.println("alarm");
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
  delay(100);
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  delay(50);
}

/**
 * parseJson - parse incoming MQTT json message
 * http://192.168.1.249:8080/temp?temp={idx : 1, nvalue : 0, svalue : 25.0}
 * --------------------------------------------
*/
tTempIn parseTemp(char json[])
{
  Serial.println("parseTemp");

  StaticJsonDocument<1024> doc;
  tTempIn tin;

  tin.error = deserializeJson(doc, json);

  if (tin.error)
  {
    Serial.print("deserializeJson() failed ");
    Serial.println(tin.error.f_str());
    return tin;
  }
  else
    Serial.print("deserializeJson() OK ");

  Serial.println();
  tin.idx = doc["idx"];
  tin.nvalue = doc["nvalue"];
  tin.svalue = doc["svalue"];
  return tin;
}

/**
 * Read persistent param from EEPROM
 * ---------------------------------
 */
void getParam()
{
  Serial.println("getParam ");

  byte l;
  char test[128];
  String x;
  byte textPos;
  byte epromPos;
  String digest = DIGEST;
  //tParam * wparam = malloc(sizeof());

  EEPROM.begin(512);

  //read digest
  Serial.print("get digest --");
  l = digest.length();
  for (int n = 0; n < 0 + l; n++)
  {
    test[n - 0] = EEPROM.read(n);
    Serial.print(test[n - 0]);
  }
  test[l] = '\0';
  x = test;
  Serial.println("--" + x + "..");

  Serial.print("read param --");
  if (x == digest)
  {
    Serial.println("-- digest found");
    l = EEPROM.read(0 + digest.length());
    Serial.print("read param len --");
    Serial.print(l);
    Serial.print("-- param --");
    if (l > 512)
      l = 512;
    textPos = 0;
    epromPos = 0 + digest.length() + 1;
    for (int n = epromPos; n < epromPos + l; n++)
    {
      test[textPos] = EEPROM.read(n);
      Serial.print(test[textPos]);
      textPos++;
    }
    test[l] = '\0';
    x = test;
    Serial.println("--");
  }
  else
  {
    x = "";
  }
  EEPROM.end();

  if (x != "")
  {
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
  }
  else
  {
    Serial.println("-- digest not found");
    _param.sSSID = CSSID;
    _param.sWiFiPassword = WIFIPASSWORD;
    _param.sDayStart = DAYSTART;
    _param.sNightStart = NIGHTSTART;
    _param.sDayTemp = DAYTEMP;
    _param.sNightTemp = NIGHTTEMP;
    _param.sSwithThreshold = SWITCHTHRESHOLD;
  }

  //return wparam;
}

/**
 * Save param to EEPROM
 * --------------------
 */
bool saveParam(tParam param)
{
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
  if (buff != _paramJSON)
  {
    _paramJSON = buff;
  }
  else
  {
    return false;
  }

  EEPROM.begin(512); //should be: size-of param + size-of digest?

  Serial.print("write digest: ");
  for (int n = 0; n < digest.length(); n++)
  {
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
  for (int n = digest.length() + 1; n < digest.length() + 1 + buff.length(); n++)
  {
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
 * initDisplay()
 * -------------
 */
void initDisplay()
{
  Serial.println("initDisplay");
  Wire.begin();
  t1.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  t1.display();
  delay(2000);
  t1.clearDisplay();
  t1.setTextSize(1);
  t1.setTextColor(WHITE);
  /*for (int i=20;i<80;i++) {
    t1.setCursor(i,i);
    t1.println("Hello, world!");
    t1.display();
    delay(100);
    t1.clearDisplay();
  } */

  // Clear the buffer.
  t1.clearDisplay();
  Serial.println("display init done");
}

/**
 * connectWiFi
 * -----------
 */
void connectWiFi()
{
  Serial.print("Connecting to ");
  Serial.print(_param.sSSID);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(SELF);
  WiFi.begin(_param.sSSID, _param.sWiFiPassword);
  int trials = 15;
  while (WiFi.status() != WL_CONNECTED && trials > 0)
  {
    delay(500);
    Serial.print(".");
    trials--;
  }
  if (trials == 0)
  {
    Serial.println("WiFi timeout reached");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("WD1_RELAY_1");
  }
  else
  {
    Serial.println("WiFi connected");
  }
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  sIP = WiFi.localIP().toString();
  WiFi.printDiag(Serial);
}

/**
 * getHour
 * -------
 * clean the input field coming from the web form
 */
int getHour(String s, char delimiter)
{
  int i = s.length();
  while (i >= 0 && s[i] != delimiter)
  {
    s[i] = ' ';
    i--;
  }
  if (i >= 0)
    s[i] = ' '; //delimiter
  return s.toInt();
}

int getMinute(String s, char delimiter)
{
  int i = 0;
  while (i <= s.length() && s[i] != delimiter)
  {
    s[i] = ' ';
    i++;
  }
  if (i <= s.length())
    s[i] = ' '; //delimiter
  return s.toInt();
}

/**
 * getDate
 * -------
 * calculate date and time from epoche time
 */
t_date getDate(double dSeconds)
{
  //double dSeconds = timeClient.getEpochTime(); //time(NULL);
  long int
      liElapsedHours,
      liElapsedDays,
      liElapsedYears,
      liElapsedLeapYears,
      liRemainingDays,
      liCurrentYear,
      liCurrentMonth,
      liCurrentDay,
      liCurrentHours,
      liCurrentMinutes,
      liCurrentSeconds;

  char bCurrentYearIsLeapYear;
  int iMonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  liElapsedHours = (long int)trunc(dSeconds / 3600);
  liElapsedDays = (long int)trunc(dSeconds / (3600 * 24));
  liElapsedYears = (long int)trunc(dSeconds / (3600 * 24 * 365));
  liElapsedLeapYears = liElapsedYears / 4 + liElapsedYears / 400 - liElapsedYears / 100;
  liRemainingDays = liElapsedDays - liElapsedYears * 365 - liElapsedLeapYears;

  liCurrentHours = ((long int)dSeconds - liElapsedDays * 3600 * 24) / 3600;
  liCurrentMinutes = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 / 60;
  liCurrentSeconds = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 % 60;

  liCurrentYear = 1970 + liElapsedYears;
  bCurrentYearIsLeapYear = (liCurrentYear % 4 == 0 && (liCurrentYear % 100 != 0 || liCurrentYear % 400 == 0));
  if (bCurrentYearIsLeapYear)
    iMonthDays[1]++;

  int i = 0;
  int j = 0;
  while (i < 12 && j < liRemainingDays)
  {
    j += iMonthDays[i];
    i++;
  }
  liCurrentMonth = i;

  j = 0;
  liCurrentDay = liRemainingDays;
  while (j < i - 1)
  {
    liCurrentDay = liCurrentDay - iMonthDays[j];
    j++;
  }

  t_date st; // = (t_date *)malloc( sizeof(t_date ));
  st.year = liCurrentYear;
  st.month = liCurrentMonth;
  st.day = liCurrentDay;
  st.hour = liCurrentHours;
  st.minute = liCurrentMinutes;
  st.second = liCurrentSeconds;
  return (st);
}

/**
 * initTimeServer
 * --------------
 * String getFormattedTime() const;
 * int getDay() const;
 * int getHours() const;
 * int getMinutes() const;
 * int getSeconds() const;
 */
void initTimeServer()
{
  Serial.println("initTimeServer");
  timeClient.begin();
  delay(100);
  timeClient.update();
  lastDate = getDate(timeClient.getEpochTime());
  lastDate.day = 0;
  Serial.println("Time initialized");
}

/**
 * updateTimeServer
 * --------------
 */
void updateTimeServer()
{
  timeClient.update();
  t_date d = getDate(timeClient.getEpochTime());
  if (d.day != lastDate.day)
  {
    lastDate = d;
    //  int[] DST_start = {28, 27, 26, 31};
    //  int[] DST_end   = {31, 30, 29, 27};
    if (d.month > 3 && d.month < 11)
      timeClient.setTimeOffset(utcOffsetInSeconds + dstOffsetInSeconds);
    else
      timeClient.setTimeOffset(utcOffsetInSeconds);
  }
}

void handleTemp();
void handleNotFound();

/**
 * Init WEB Server
 * ---------------
 */
void initWebServer()
{
  Serial.println("initWEBServer");
  WEBServer.on("/", handleRoot);
  WEBServer.on("/admin", handleRoot);
  WEBServer.on("/admin/", handleRoot);
  //  WEBServer.on("/setup/", handleSetup);
  WEBServer.on("/temp", handleTemp);
  WEBServer.onNotFound(handleNotFound);
  WEBServer.begin();
  Serial.println("HTTP server started");
}

/**
 * mqttCallback
 * ------------
 */
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("MQTTCallBack");
}

/**
 * MQTT init
 * ---------
 */
void initMQTT()
{
  Serial.println("initMQTT");
  MQTTClient.setServer(mqttServer, mqttPort);
  MQTTClient.setBufferSize(1024);
  MQTTClient.setCallback(callback);

  int trials = 3;
  Serial.print("Connecting to MQTT");
  MQTTClient.setKeepAlive(MQTTKEEPALIVE);
  while (!MQTTClient.connected() && trials > 0)
  {
    if (MQTTClient.connect(SELF, nullptr, nullptr, "domoticz", 0, true, "", false))
    {
      /*client.connect("relay proto", mqttUser, mqttPassword )*/
    }
    else
    {
      Serial.print(".");
    }
    trials--;
  }
  if (!MQTTClient.connected())
  {
    Serial.print("failed with state ");
    Serial.println(MQTTClient.state());
  }
  else
  {
    //client.publish("domoticz/in", "{ \"idx\" : 1, \"nvalue\" : 0, \"svalue\" : \"25.0\" }");
    MQTTClient.subscribe("domoticz/out");
    Serial.println("MQTT connection established");
  }
}

/**
 * handleRoot ()
 * -------------
 * if get -> return html
 * if post -> process arguments and reply with 303
 */
void handleRoot()
{
  Serial.println("handleRoot" + WEBServer.uri());
  timeClient.update();

  Serial.print(WEBServer.hostHeader());

  if (WEBServer.args())
  {
    //Serial.println("handle root submit");
    //    handleRootSubmit();
    if (WEBServer.arg("auto") == "on")
    {
      iManualToggleState = 1;
    }
    else if (WEBServer.arg("auto") == "off")
    {
      iManualToggleState = 0;
    }
    else if (WEBServer.arg("auto") == "auto")
    {
      iManualToggleState = 2;
    }

    /*tParam backup = _param;*/
    _param.sDayTemp = WEBServer.arg("dayTemp");
    _param.sNightTemp = WEBServer.arg("nightTemp");
    _param.sDayStart = WEBServer.arg("startDay");
    _param.sNightStart = WEBServer.arg("nightStart");
    _param.sSwithThreshold = WEBServer.arg("switchThreshold");
    /*if (backup.sDayTemp != _param.sDayTemp ||
      backup.sDayTemp != _param.sDayTemp ||
      backup.sDayTemp != _param.sDayTemp ||
      backup.sDayTemp != _param.sDayTemp ||
      backup.sDayTemp != _param.sDayTemp ||
      backup.sDayTemp != _param.sDayTemp) {
      Serial.println("parameters have changed");*/
    saveParam(_param);
    timer_manualToggle.reset();
    timer_1.flush();
    /*}*/

    WEBServer.sendHeader("Location", "/");
    WEBServer.send /*Header*/ (303, "text/html", "");
    //"HTTP/1.1 400 Bad Request"
    //"Connection: Closed"
    //"Location: \r\n");
  }
  else
  {
    //Redisplay the form
    Serial.println("redisplay root");
    WEBServer.send(200, "text/html", rootHTML());
  }
}

/**
 * handleTemp
 * ----------
 */
void handleTemp()
{
  Serial.println("handleTemp");

  char json[1024];
  Serial.print("Temp arrived ");

  Serial.print("Message:");
  /*for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json[i] = (char)payload [i];
  }*/
  Serial.println(WEBServer.args());

  for (int i = 0; i <= WEBServer.args(); i++)
  {
    Serial.print(i);
    Serial.println(WEBServer.arg(i));
    Serial.println(WEBServer.header(i));
  }
  Serial.println();

  String s = WEBServer.arg("temp");
  int n = s.length();
  strcpy(json, s.c_str());

  tTempIn tinn = parseTemp(json);

  if (tinn.error)
    Serial.println("Couldn't parse message");
  else
  {
    Serial.print("evaluate tinn.idx ");
    int idx = tinn.idx;
    //global_nvalue = tinn.nvalue;
    float svalue = tinn.svalue;
    if (idx == idx)
    {
      Serial.println("Temp changed");
      //reset delay counter
      //f_currentTemp = tinn.svalue;
      fCurrentTemp = tinn.svalue;
      timer_1.flush();
      //data present, start counting for data time-out
      timer_noData.reset();
    }
  }
  WEBServer.send(200, "text/plain", "Temp OK");
}

/**
 * handleNotFound ()
 * -----------------
 */
void handleNotFound()
{
  Serial.print("handleNotFound ");
  Serial.println(WEBServer.uri());
  WEBServer.send(404, "text/plain", "Not found");
}

/**
 * Setup environment
 * -----------------
 */
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ledPin, HIGH);

  Serial.begin(115200);
  delay(2000);
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(ledPin, LOW);

  initDisplay();
  getParam();
  Serial.println("***" + _param.sSSID + "***");
  connectWiFi();
  initTimeServer();
  initWebServer();
  iPrevSwitchState = digitalRead(switchPin);
  initMQTT();
}

void loop()
{
  //check for web input
  WEBServer.handleClient();
  handlePhysicalSwitch();

  if (timer_1.timeout())
  {

    if (iManualToggleState != 2)
    {
      if (timer_manualToggle.timeout())
      {
        iManualToggleState = 2;
      }
    }

    if (iManualToggleState == 2)
    {
      updateTimeServer();
      float fTime, fDayStart, fNightStart;
      fTime = (float)(timeClient.getHours() * 100 + timeClient.getMinutes()) / 100;
      fDayStart = (float)(getHour(_param.sDayStart, ':') * 100 + getMinute(_param.sDayStart, ':')) / 100;
      fNightStart = (float)(getHour(_param.sNightStart, ':') * 100 + getMinute(_param.sNightStart, ':')) / 100;
      Serial.println(fTime);
      Serial.println(fDayStart);
      bDay = (fTime >= fDayStart && fTime < fNightStart);
      if (bDay)
        if (fCurrentTemp > _param.sDayTemp.toFloat())
          iRelayState = 0;
        else
          iRelayState = 1;
      else if (fCurrentTemp > _param.sNightTemp.toFloat())
        iRelayState = 0;
      else
        iRelayState = 1;
    }
    else
    {
      iRelayState = iManualToggleState;
    }

    if (timer_noData.timeout())
    {
      alarm(NOTEMPDATA);
      iRelayState = 0;
    }

    if (iRelayState == 1)
    {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(ledPin, HIGH);
      digitalWrite(relayPin, HIGH);
    }
    else
    {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(ledPin, LOW);
      digitalWrite(relayPin, LOW);
    }

    // consider calling it in every loop to be able to show seconds correctly; timer must be set than in the function itself
    statusDisplay();

    //WEBServer.sendHeader("Location", "");
    //WEBServer.send/*Header*/(303, "text/html", "");
  }

  /*
  float                   fCurrentTemp = 22.01;
  int                     iManualToggleState = 1;
  int                     iManualToggleTimeOut = 40;
  int                     iNoTempDataTimeOut = 4000;
  int                     iPhysicalSwitchState = 1;
  int                     iPrevSwitchState;
  int                     iRelayState = 1;
  */
  //delay(5000);

  //check for delays
  //check for data timeouts
  //refresh time
  //evaluate current parameters
  //  switch state -> auto, on, off
  //     manual switch time-out
  //  time -> maxDay, MaxNight
  //  currTemp
  //  and decide whether relay should be turned on or off
  //show current state on display
  //refresh delay parameters
}

/**
 * rootHTML
 * --------
 * prepare the hrml string for the root path
 */
String rootHTML()
{
  Serial.println("rootHTML");
  String ptr = "";
  ptr += "<!DOCTYPE HTML><html><head><title>Thermostat</title><script src='http:/";
  ptr += "/ajax.googleapis.com/ajax/libs/jquery/2.0.2/jquery.min.js'></script><me";
  ptr += "ta name='viewport' content='width=device-width, initial-scale=1.0' char";
  ptr += "set='UTF-8'><style>html {font-family: 'Open Sans', sans-serif; display:";
  ptr += " block; margin: 0px auto; text-align: center;color: #444444;}body {marg";
  ptr += "in: 0px;} h1 {margin: 50px auto 30px;} #customers {font: 16px/30px, Ari";
  ptr += "al, sans-serif;border-collapse: collapse;align: center;width: 50%;margi";
  ptr += "n: 0 auto;}#customers td { border: 1px solid #ddd; padding: 8px;}#custo";
  ptr += "mers th { font: bold 18px/30px Arial, sans-serif; border: 1px solid #dd";
  ptr += "d; padding: 8px; padding-top: 12px; padding-bottom: 12px; text-align: l";
  ptr += "eft; background-color: #4CAF50; font-variant: small-caps; color: white;";
  ptr += "}#customers tr:nth-child(even){background-color: #f2f2f2;}#customers tr";
  ptr += ":hover {background-color: #ddd;} #ok {background-color: #4CAF50;}#notok";
  ptr += " {background-color: #FFFC0B;}#input-w {float: none; width: auto; height";
  ptr += ": auto;overflow: hidden;}h2 {margin: 5;display: inline-block;padding: 8";
  ptr += "px;}p {background-color: #FFFC0B;display: inline;}h1::selection { backg";
  ptr += "round: green; color: yellow; }h2::selection { background: red; color: y";
  ptr += "ellow; }.switch-toggle { float: left; background: #242729;}.switch-togg";
  ptr += "le input { position: absolute; opacity: 0;}.switch-toggle input + label";
  ptr += " { padding: 7px; float:left; color: #fff; cursor: pointer;}.switch-togg";
  ptr += "le input:checked + label { background: red;}</style>";
  ptr += "</head><body><h1>Enter your preferred values and press Send</h1><div cl";
  ptr += "ass='container'><FORM action='/' method='post' target='_self'><table id";
  ptr += "='customers'><th colspan='2'>Temperature</th><th colspan='2'>Time</th><";
  ptr += "tr><td><label>Day</label></td><td><input id='input-w' size='2' maxlengt";
  ptr += "h='5'name='dayTemp' placeholder='celsius'value = '@dayTemp' ><br></td><";
  ptr += "td><label>Day</label></td><td><input id='input-w' maxlength='5' name='s";
  ptr += "tartDay' placeholder='hour' size = '2'value='@startDay'><br></td></tr><";
  ptr += "tr><td><label>Night</label></td><td><input id='input-w' size='2' maxlen";
  ptr += "gth='5'name='nightTemp' placeholder='celsius'value = '@nightTemp' ><br>";
  ptr += "</td><td><label>Night</label></td><td><input id='input-w' maxlength='5'";
  ptr += "name='nightStart' placeholder='hour'size='2'value='@nightStart'><br></t";
  ptr += "d></tr><tr><td><label>Switching thereshold</label></td><td><input id='i";
  ptr += "nput-w' size='2' maxlength='5'name='switchThreshold' placeholder='0 < n";
  ptr += " < 1'value = '@switchThreshold'></td><td><label>Manual/Auto</label></td";
  ptr += "><td><div class='switch-toggle'> <input id='on' onChange='this.form.sub";
  ptr += "mit();' name='auto' type='radio' value='on' @on /> <label for='on' >On<";
  ptr += "/label> <input id='auto' onChange='this.form.submit();' name='auto' typ";
  ptr += "e='radio' value='auto' @auto /> <label for='auto' >Auto</label> <input ";
  ptr += "id='off' onChange='this.form.submit();' name='auto' type='radio' value=";
  ptr += "'off' @off/> <label for='off' >Off</label></div></td></tr></TABLE><br><";
  ptr += "p><INPUT id='ok' type='submit' value='Send'> <INPUT id='notok' type='re";
  ptr += "set'></p></FORM></div><div><h2>Heating @relayState</h2><h2>Temperature ";
  ptr += "@tempCurr</h2><h2>Mode @physicalSwitchState</h2><h2>Time @timeCurr</h2>";
  ptr += "</div>";
  ptr += "<div class='container'><table id='customers'><th>Parameter</th><th>Valu";
  ptr += "e</th><th>Parameter</th><th>Value</th><tr><td>daytime temperature</td><";
  ptr += "td>@dayTemp</td><td>nighttime temperature</td><td>@nightTemp</td></tr><";
  ptr += "tr><td>current temperature</td><td>@tempCurr</td><td>physical switch st";
  ptr += "ate</td><td><mark>@physicalSwitchState</mark></td></tr><tr><td>daytimeb";
  ptr += "egin</td><td>@startDay</td><td>nighttime begin </td><td>@nightStart</td";
  ptr += "></tr><tr><td>MQTT status</td><td>@mqttStatus</td><td>timeout</td><td>@";
  ptr += "noTempDataTimeOut</td></tr><tr><td>IP address</td><td>@ip</td><td>WiFi ";
  ptr += "SSID</td><td>@SSID</td></tr><tr><td>Elapsed millis since boot</td><td>@";
  ptr += "millis</td><td>Current time</td><td>@timeCurr</td></tr><tr><td>Current ";
  ptr += "day of week</td><td>@dayName</td><td>Temperature threshold</td><td>@swi";
  ptr += "tchThreshold</td></tr></td><td>manual toggle state</td><td><mark>@manua";
  ptr += "lToggleState</mark></td></tr></table></div> </body></html>";

  //
  /*String                sSSID;
  String                sWiFiPassword;
  String                sDayStart;
  String                sNightStart;
  String                sDayTemp;
  String                sNightTemp;
  String                sSwithThreshold;
  DeserializationError  error;

  float                 fCurrentTemp;
  int                   iManualToggeState;
  int                   iManualToggleTimeOut;
  int                   iNoTempDataTimeOut;
  int                   iPhysicalSwitchState;
  int                   iRelayState;*/
  //
  /*daymax
startday
nighttemp
nightstart
swithThreshold
tempCurr
mqttstatus
ip

on off auto
status
physicalSwitchState;
switchmode
timecurr
noTempDataTimeOut


relayState*/
  timeClient.update();
  ptr.replace("@dayTemp", _param.sDayTemp);
  ptr.replace("@nightTemp", _param.sNightTemp);
  ptr.replace("@startDay", _param.sDayStart);
  ptr.replace("@nightStart", _param.sNightStart);

  ptr.replace("@switchThreshold", _param.sSwithThreshold);
  ptr.replace("@tempCurr", String(fCurrentTemp));
  ptr.replace("@mqttStatus", MQTTClient.connected() ? "connected" : "not connected");
  ptr.replace("@ip", sIP);
  ptr.replace("@millis", String(millis()));
  ptr.replace("@dayName", daysOfTheWeek[timeClient.getDay()]);

  ptr.replace("@physicalSwitchState", (iPhysicalSwitchState == 1) ? "ON" : "OFF");
  ptr.replace("@relayState", (iRelayState == 1) ? "ON" : "OFF");

  switch (iManualToggleState)
  {
  case 1:
  {
    ptr.replace("@manualToggleState", "on");
    ptr.replace("@on", "checked=''");
    break;
  }
  case 0:
  {
    ptr.replace("@manualToggleState", "off");
    ptr.replace("@off", "checked=''");
    break;
  }
  case 2:
  {
    ptr.replace("@manualToggleState", "auto");
    ptr.replace("@auto", "checked=''");
    break;
  }
  default:
    Serial.println("default");
    ;
  }

  ptr.replace("@manualToggleTimeOut", String(timer_manualToggle.get()));

  ptr.replace("@nightStart", _param.sNightStart);
  ptr.replace("@noTempDataTimeOut", (timer_noData.timeout()) ? "yes" : "no");
  ptr.replace("@SSID", _param.sSSID);
  ptr.replace("@timeCurr", timeClient.getFormattedTime());
  ptr.replace("@switchThreshold", _param.sSwithThreshold);

  return ptr;
}
