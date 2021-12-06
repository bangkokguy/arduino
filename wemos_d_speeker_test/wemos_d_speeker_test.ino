//These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
#define TIMER_INTERRUPT_DEBUG      1

#include "ESP8266TimerInterrupt.h"

bool flag=false;
//----------------
volatile uint32_t lastMillis = 0;

void ICACHE_RAM_ATTR TimerHandler(void)
{
  static bool toggle = false;
  static bool started = false;
  flag = ~flag;
  Serial.println("interrupt");

  if (!started)
  {
    started = true;
    pinMode(LED_BUILTIN, OUTPUT);
  }

  #if (TIMER_INTERRUPT_DEBUG > 0)
  if (lastMillis != 0)
    Serial.println("Delta ms = " + String(millis() - lastMillis));
  lastMillis = millis();
  #endif
  
  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(LED_BUILTIN, toggle);
  toggle = !toggle;
}

#define TIMER_INTERVAL_MS        1000

// Init ESP8266 timer 0
ESP8266Timer ITimer;

//----------------

int stepUp = 100;

//0=D3 4=D2 5=D1 12=D6 13=D7 14=D5 15=D8 16=D0 17=D4
#define d0 16
#define d1 5
#define d2 4
#define d3 0
#define d4 17
#define d5 14
#define d6 12
#define d7 13
#define d8 15

class Flasher
{
    int ledPin;      // the number of the LED pin
    long OnTime;     // milliseconds of on-time
    long OffTime;    // milliseconds of off-time

    int ledState;                 // ledState used to set the LED
    unsigned long previousMillis;   // will store last time LED was updated

    public: Flasher(int pin, long on, long off) {
      ledPin = pin;
      pinMode(ledPin, OUTPUT);     
      OnTime = on;
      OffTime = off;
      ledState = LOW; 
      previousMillis = 0;
    }

    void Update() {
      // check to see if it's time to change the state of the LED
      unsigned long currentMillis = millis();
       
      if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))  {
        ledState = LOW;  // Turn it off
        previousMillis = currentMillis;  // Remember the time
        digitalWrite(ledPin, ledState);  // Update the actual LED
      }
      else
        if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
          ledState = HIGH;  // turn it on
          previousMillis = currentMillis;   // Remember the time
          digitalWrite(ledPin, ledState);   // Update the actual LED
        }
      }
};    

Flasher led1(d1, 100, 400);
Flasher led2(d2, 350, 350);
/**
   Setup
*/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH);

    // Interval in microsecs
  if (ITimer.attachInterruptInterval(400, TimerHandler))
    Serial.println("Starting  ITimer OK, millis() = " + String(millis()));
  else
    Serial.println("Can't set ITimer correctly. Select another freq. or interval");
}

/**
   loop
*/
void loop() {
  //led1.Update();
  noTone(d6);
  tone(d6, 600 + stepUp);
  if (stepUp > 4000) stepUp = -500;
  stepUp = stepUp + 50;
  delay(10);
  Serial.println(flag);
}
