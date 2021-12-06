//#include <SPI.h>
//#include "nRF_SSD1306Wire.h"
#include "nRF_SH1106Wire.h"
#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include "Timer.h"

#include <Ticker.h>

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

#define SCREEN_WIDTH  132
#define SCREEN_HEIGHT 64
#define OLED_RESET 0

SH1106Wire/*SSD1306Wire*/ OLEDDisplay(0x3c, SDA, SCL);
//OLEDDisplay /*Adafruit_SSD1306*/        tx(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define OLED_RESET 5

/**
 * initDisplay()
 * -------------
 */
void initDisplay () {
  Serial.println("initDisplay");
  Wire.begin();
  
  OLEDDisplay.init();
  OLEDDisplay.resetDisplay();
  OLEDDisplay.clear();

  OLEDDisplay.flipScreenVertically();
  OLEDDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
  OLEDDisplay.setFont(ArialMT_Plain_24);
  OLEDDisplay.drawString(0, 0, "Hello world");
  OLEDDisplay.display();
  delay(2000);
  OLEDDisplay.displayOff();
  
  Serial.println(OLEDDisplay.getWidth());
  Serial.println(OLEDDisplay.getHeight());
  OLEDDisplay.displayOn();
  
  Serial.println("display init done");
}

int buttonState = 0;         
int lastButtonState = 0;
int touchCounter = 0;
int gap = 0;
int buttonPress = 0;
int sensorState = 0;
int prevSensorState = 0;
 
Ticker blinker;

/**---------------------------------
 * pirISR()
 -----------------------------------*/
void pirISR () {
  //sensorState = digitalRead(d3);
  //if (sensorState != prevSensorState) {
  Serial.println("Sensor state changed");
  //  prevSensorState = sensorState;
}

/**---------------------------------
 * changeState() 
 -----------------------------------*/
void changeState() {
  int c = analogRead(A0);

  //Serial.println(digitalRead(d3));
  sensorState = digitalRead(d3);
  if (sensorState != prevSensorState) {
    Serial.println("Sensor state changed");
    prevSensorState = sensorState;
  }
  
  //Serial.print(c); Serial.print(" -- "); Serial.println(buttonState);
  if(c > 20) {
    if (gap <3)             {buttonPress = 1; Serial.print("bounce    ");}
    if (gap >=3 && gap <10) {buttonPress = 2; Serial.print("double tap");}
    if (gap >=10)           {buttonPress = 3; Serial.print("simple tap");}
    Serial.print(" gap=");  {Serial.print(gap); Serial.print(" -- "); Serial.print(c); Serial.print(" -- "); Serial.println(buttonState);}
    gap = 0;
    digitalWrite(d6, !(digitalRead(d6)));  //Invert Current State of LED  
    lastButtonState = buttonState;
    buttonState = !(buttonState); 
  } else {
    if (lastButtonState != buttonState) {}
    gap++;
  }
}

/**---------------------------------
 * setup()
 -----------------------------------*/
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Start");
  pinMode(d5, OUTPUT);
  pinMode(d6, OUTPUT);
  pinMode(d7, OUTPUT);
  pinMode (switchPin, INPUT);
  pinMode (A0, INPUT);
  pinMode(d3, INPUT);
  blinker.attach(0.1, changeState);
  //attachInterrupt(digitalPinToInterrupt(02), pirISR, CHANGE);
  initDisplay();
}

void turnOffAll() {
      digitalWrite(d5, HIGH);
      digitalWrite(d6, HIGH);
      digitalWrite(d7, HIGH);
}

void vibrate(int led, int cycle, int dly) {
  if (buttonState == 0)
    for (int i = 0; i <= cycle-1; i++) {
      digitalWrite(led, LOW);
      delay(dly);
      digitalWrite(led, HIGH);
      delay(dly);
    }
}

void vibrate1(int led1, int led2, int cycle, int dly) {
  if (buttonState == 0)
    for (int i = 0; i <= cycle-1; i++) {
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      delay(dly);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      delay(dly);
    }
}

Timer t1(500, AUTORESET);
Timer t2(500, AUTORESET);
Timer t3(500, AUTORESET);
Timer t4(500, AUTORESET);
Timer t5(500, AUTORESET);

void loop() {
  //Serial.println(readCapacitivePin(d6));
  
  OLEDDisplay.clear();
  OLEDDisplay.drawString(0, 0, "kitaláltam");
  OLEDDisplay.display();
  turnOffAll();
  vibrate(d5, 6, 35);
  delay(500);

  OLEDDisplay.clear();
  OLEDDisplay.drawString(0, 0, "a");
  OLEDDisplay.display();
  turnOffAll();
  vibrate(d6, 6, 35);
  delay(500);

  OLEDDisplay.clear();
  OLEDDisplay.drawString(0, 0, "játékomat");
  OLEDDisplay.display();
  turnOffAll();
  vibrate(d7, 6, 35);
  delay(500);

  OLEDDisplay.clear();
  OLEDDisplay.drawString(0, 0, "is:");
  OLEDDisplay.display();
  turnOffAll();
  vibrate1(d5, d7, 6, 30);
  delay(500);

  OLEDDisplay.clear();
  OLEDDisplay.drawString(0, 0, "foci játék!!");
  OLEDDisplay.display();
  turnOffAll();
  vibrate1(d5, d6, 6, 30);
  delay(500);

  turnOffAll();
  for(int i=1;i<=100;i++) {
    OLEDDisplay.clear();
    OLEDDisplay.drawString(0, 0, String(i));
    OLEDDisplay.display();
    delay(100);
    }
  //delay(50000);

}
