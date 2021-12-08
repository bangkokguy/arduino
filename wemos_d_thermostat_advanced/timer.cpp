#include <ESP8266WiFi.h>
#include "timer.h"

#define DEBUG if (1==2)

Timer::Timer(int ms, bool autoreset/* = true*/) {
      startTime = millis();
      waitTime = (unsigned long)ms;
      autoReset = autoreset;
      DEBUG Serial.print("Timer serialized ");
      DEBUG Serial.print(ms);
      DEBUG Serial.print(" - ");
      DEBUG Serial.println(startTime);
    }
    
void Timer::reset (int ms) {
      DEBUG Serial.print("Timer reset "); 
      DEBUG Serial.print(ms);
      DEBUG Serial.print(" - ");
      DEBUG Serial.println(startTime); 
      startTime = millis();
      if (ms != 0) waitTime = (unsigned long)ms;
    }

void Timer::flush () {
      DEBUG Serial.print("Timer flush ");
      startTime = 0;
    }

int Timer::remaining () {return millis() - startTime;}

int Timer::get () {return waitTime;}
  
bool Timer::timeout () {
      //Serial.print("timeout - "); Serial.print(startTime);  Serial.print(" - "); Serial.println(millis());
      if (millis() - startTime > waitTime) {
        DEBUG Serial.print("Timeout reached "); 
        DEBUG Serial.print(waitTime);
        DEBUG Serial.println(autoReset);
        if (autoReset) reset(waitTime);
        return true;
      } else { 
        return false;
      }
    }