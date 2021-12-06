/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/

int brightness = 0;
int fadeAmount = 5;

void setup() {
  Serial.begin(115200);
  Serial.println("hahó");
  pinMode(LED_BUILTIN, OUTPUT);
  
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("hahó");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);             
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);             
/*  digitalWrite(D5, LOW);  
  digitalWrite(D6, HIGH); 
  delay(2000);             
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);  
  digitalWrite(D6, LOW); 
  delay(2000);             */
//--
  /*analogWrite (D6, brightness);

  brightness = brightness + fadeAmount;
  if (brightness == 0 || brightness == 255) {
    fadeAmount = -fadeAmount;
    }

  delay(30);*/
  /*digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  delay(2000);          
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  delay(2000);          */

}
