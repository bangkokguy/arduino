#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include </home/bangkokguy/Arduino/libraries/Adafruit_SSD1306_Wemos_Mini_OLED/Adafruit_SSD1306.h>

#define d0 16
#define d1 5
#define d2 4
#define d3 0
#define d4 17
#define d5 14
#define d6 12
#define d7 13
#define d8 15
// SCL GPIO5
// SDA GPIO4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET d7

//Adafruit_SSD1306 t1(OLED_RESET);
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Adafruit_SSD1306 t1(48, 48, &Wire, OLED_RESET);
//Adafruit_SSD1306 t1(48, 64, &Wire, OLED_RESET);
//Adafruit_SSD1306 t1(48, 96, &Wire, OLED_RESET);
//?Adafruit_SSD1306 t1(48, 104, &Wire, OLED_RESET);
//Adafruit_SSD1306 t1(64, 48, &Wire, OLED_RESET);
//?Adafruit_SSD1306 t1(96, 48, &Wire, OLED_RESET);
//!!Adafruit_SSD1306 t1(128, 64, &Wire, OLED_RESET);
Adafruit_SSD1306 t1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
 
 
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
 
 
void setup()   {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("program started");
  
  Wire.begin();
   
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  t1.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done

  Serial.println("display init done");
  
  t1.display();
  delay(2000);
 
  // Clear the buffer.
  t1.clearDisplay();

  Serial.println("display cleared");
  delay(5000);

}
 
 
void loop() {
  // text display tests
  t1.setTextSize(1);
  t1.setTextColor(WHITE);
  for (int i=20;i<80;i++) {
    t1.setCursor(i,i);
    t1.println("Hello, world!");
    t1.display();
    delay(100);
    t1.clearDisplay();
  } 
}
