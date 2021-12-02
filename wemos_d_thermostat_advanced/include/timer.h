#include <ESP8266WiFi.h>

class Timer {
  private:    
    unsigned long startTime;
    unsigned long waitTime;
  public:  
    Timer(int ms, bool autoreset);
    void reset (int ms = 0);
    bool timeout ();
    void flush ();
    int remaining ();
    int get ();
    bool autoReset;
};

Timer::Timer(int ms, bool autoreset/* = true*/) {
      startTime = millis();
      waitTime = (unsigned long)ms;
      autoReset = autoreset;
      Serial.print("Timer serialized "); Serial.print(ms); Serial.print(" - "); Serial.println(startTime);
    }
    
void Timer::reset (int ms) {
      Serial.print("Timer reset "); Serial.print(ms);  Serial.print(" - "); Serial.println(startTime); 
      startTime = millis();
      if (ms != 0) waitTime = (unsigned long)ms;
    }

void Timer::flush () {
      Serial.print("Timer flush ");
      startTime = 0;
    }

int Timer::remaining () {return millis() - startTime;}

int Timer::get () {return waitTime;}
  
bool Timer::timeout () {
      //Serial.print("timeout - "); Serial.print(startTime);  Serial.print(" - "); Serial.println(millis());
      if (millis() - startTime > waitTime) {
        Serial.print("Timeout reached "); Serial.print(waitTime); Serial.println(autoReset);
        if (autoReset) reset(waitTime);
        return true;
      } else { 
        return false;
      }
    }