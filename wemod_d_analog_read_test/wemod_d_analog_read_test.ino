#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define CSSID               "MrWhite"
#define WIFIPASSWORD        "Feketebikapata1965!"
#define utcOffsetInSeconds 3600
#define dstOffsetInSeconds 3600

struct t_date {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;  
};

WiFiUDP                 ntpUDP;
NTPClient               timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
WiFiClient              espClient;


/**
 * connectWiFi
 * -----------
 */
void connectWiFi () {
  Serial.print("Connecting to ");
  Serial.print(CSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(CSSID, WIFIPASSWORD);
  int trials = 15;
  while (WiFi.status() != WL_CONNECTED && trials > 0) {
    delay(500);
    Serial.print(".");
    trials--;
    }
  if (trials == 0) {
    Serial.println("WiFi timeout reached");
      WiFi.mode(WIFI_AP);
    WiFi.softAP("WD1_RELAY_2");
  } else {
    Serial.println("WiFi connected");  
  }
  Serial.print(WiFi.softAPIP());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
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
void initTimeServer () {
  Serial.println("initTimeServer");
  timeClient.begin();
  delay(100);
  timeClient.update();
  
  t_date d = getDate(timeClient.getEpochTime());
//  int[] DST_start = {28, 27, 26, 31};
//  int[] DST_end   = {31, 30, 29, 27};
  if (d.month > 3 && d.month < 11) 
    timeClient.setTimeOffset(utcOffsetInSeconds + dstOffsetInSeconds);
  
  Serial.println("Time initialized");
}

t_date getDate (double dSeconds) {
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
  int  iMonthDays [12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  liElapsedHours = (long int)trunc(dSeconds / 3600);
  liElapsedDays = (long int)trunc(dSeconds / (3600*24));
  liElapsedYears = (long int)trunc(dSeconds / (3600*24*365));
  liElapsedLeapYears = liElapsedYears / 4 + liElapsedYears / 400 - liElapsedYears / 100;
  liRemainingDays = liElapsedDays - liElapsedYears * 365 - liElapsedLeapYears;

  liCurrentHours = ((long int)dSeconds - liElapsedDays * 3600 * 24) / 3600;
  liCurrentMinutes = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 / 60;
  liCurrentSeconds = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 % 60;
  
  liCurrentYear = 1970 + liElapsedYears;
  bCurrentYearIsLeapYear = (liCurrentYear % 4 == 0 && (liCurrentYear % 100 != 0 || liCurrentYear % 400 == 0));
  if (bCurrentYearIsLeapYear) iMonthDays[1]++;
  
  int i = 0; int j = 0;
  while (i < 12 && j < liRemainingDays) {
    j+= iMonthDays [i];
    i++;
  }
  liCurrentMonth = i;

  j = 0;
  liCurrentDay = liRemainingDays;
  while (j < i - 1) {
    liCurrentDay = liCurrentDay - iMonthDays [j];
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

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  pinMode (A0, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
    
  digitalWrite(LED_BUILTIN, LOW);
  
  connectWiFi ();
  initTimeServer ();
  digitalWrite(LED_BUILTIN, HIGH);
}

bool sw = false;
void loop() {
  timeClient.update();
  
  t_date s = getDate(timeClient.getEpochTime());
  
  Serial.print(s.year);     Serial.print("-");
  Serial.print(s.month);    Serial.print("-");
  Serial.print(s.day);      Serial.print(" "); 
  Serial.print(s.hour);     Serial.print(":");
  Serial.print(s.minute);   Serial.print(":");
  Serial.println(s.second);

  delay(1000);

 
 /*int i = analogRead(A0);
  if (i>40) {
      Serial.print(i); Serial.println(sw?" On":" Off");
      sw = !sw;
      delay(500);
   }
   
   delay(20);*/
}
