#define d0 16
#define d1 5
#define d2 4
#define d3 0
#define d4 17
#define d5 14
#define d6 12
#define d7 13
#define d8 15

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

class mDelay {      
  public:

    unsigned long mDelayMs;
    unsigned long mLastMillis;

    mDelay(unsigned long ms) {
      #ifdef DEBUG 
        Serial.println("ini"); 
      #endif
      mDelayMs = ms;
      mLastMillis = millis();
    }
    bool isTimeOut () {
      #ifdef DEBUG
        Serial.print("istimeout ");
        Serial.print(mLastMillis); Serial.print(" "); Serial.print(mDelayMs); Serial.print("istimeout "); Serial.println(millis());
      #endif
      if (mLastMillis + mDelayMs < millis()) {
        #ifdef DEBUG
          Serial.println("time out");
        #endif
        mLastMillis = millis();
        return true;
      } else {
        return false;
      }
    }
    int getDelayMs() {return mDelayMs;}
};
/**---------------------------------------------------------------------------
     * Depending on the Battery percentage delivers the calculated LED color.
     * @param percent the battery percent (int)
     * @return the argb LED color (int)
*/
int * argbLedColor (int percent) {
      
  static int  colors[3];
      
  int j = (percent/(100/6));
  int r, g, b;
  int grade=((percent%(100/6))*255/(100/6));

  switch (j) { // @formatter:off
            case 0: r = 255;        g = 0;              b = 255-grade;  break;//0-16 pink_to_red         255,0:255,0
            case 1: r = 255;        g = (grade/2);      b = 0;          break;//17-33 red_to_orange      255:0,255,0
            case 2: r = 255;        g = 128+(grade/2);  b = 0;          break;//34-50 orange_to_yellow   0,255,0:255
            case 3: r = 255-grade;  g = 255;            b = 0;          break;//51-66 yellow_to_green    0,255:0,255
            case 4: r = 0;          g = 255;            b = grade;      break;//67-83 green_to_cyan      0:255,0,255
            case 5: r = 0;          g = 255-grade;      b = 255;        break;//84-100 cyan_to_blue
            default:r = 200;        g = 200;            b = 200;        break;//gray if full
        } //@formatter:on

  colors[0] = r;
  colors[1] = g;
  colors[2] = b;
  return colors; //Color.argb(OPAQUE, r, g, b);
}

mDelay ms500(500);
mDelay ms2000(2000);
mDelay ms5000(5000);
mDelay ms5000_1(5000);
mDelay ms5000_2(5000);

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  pinMode(A0, INPUT);
  
  pinMode(d5, OUTPUT);
  pinMode(d6, OUTPUT);
  pinMode(d7, OUTPUT);
  
  pinMode(d3, OUTPUT);
  pinMode(d8, OUTPUT);
  
  digitalWrite(d5, LOW);
  digitalWrite(d6, LOW);
  digitalWrite(d7, LOW);
  
  digitalWrite(d3, LOW);
  digitalWrite(d8, HIGH);
  delay(100);

  Serial.println(ms500.getDelayMs());
  Serial.println(ms2000.getDelayMs());

  WiFi.begin("MrWhite", "Feketebikapata1965!"); //Connect to WiFi network

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the connection to the WiFi network 
  
  delay(500);
  Serial.print(".");
  
  }
  
  if (MDNS.begin("myesp")) {  //Start mDNS
  
  Serial.println("MDNS started");
  
  }
  
  server.on("/", handleRoot);  //Associate handler function to path
  
  server.begin();                           //Start server
  Serial.println("HTTP server started");

}

void handleRoot() {

  server.send(200, "text/plain", "Hello resolved by mDNS!");

}

void loop() {
  server.handleClient(); 
  
  if (ms2000.isTimeOut()) {
    Serial.println(analogRead(A0));
  }
  delay (1000);

  digitalWrite(d3, LOW);
  digitalWrite(d8, HIGH);
  /*digitalWrite(d7, LOW);
  digitalWrite(d3, HIGH);*/

  if (ms5000.isTimeOut()) {
    digitalWrite(d8, LOW);
    digitalWrite(d3, HIGH);
  
    
    digitalWrite(d5, HIGH);
    digitalWrite(d6, LOW);
    digitalWrite(d7, LOW);
    digitalWrite(d3, LOW);
  
      if (ms5000_1.isTimeOut()) {
        digitalWrite(d5, LOW);
        digitalWrite(d6, LOW);
        digitalWrite(d7, HIGH);
        digitalWrite(d3, HIGH);
    
        if (ms5000_2.isTimeOut()) {
          digitalWrite(d3, LOW);
      
          #ifdef DEBUG
            Serial.println("loop");
          #endif
      
          for (int x=1; x<=95; x++) {
            if (ms2000.isTimeOut()) {
              Serial.println(analogRead(A0));
            }
            int *p;
            p = argbLedColor(x);
            #ifdef DEBUG
              for ( int f = 0; f < 3; f++ ) {
                Serial.println( *(p + f));
              }
            #endif
            analogWrite(d6, *(p + 0)); //red
            analogWrite(d7, *(p + 1)); //green
            analogWrite(d5, *(p + 2)); //blue
      
            delay(100);
          }
      }
    }
  }
}
