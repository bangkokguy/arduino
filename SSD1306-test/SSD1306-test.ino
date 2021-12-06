#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*Put your SSID & Password*/
const char* ssid = "MrWhite";  // Enter SSID here
const char* password = "Feketebikapata1965!";  //Enter Password here

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("setup display");
  Wire.begin(4, 5);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  Serial.println("display");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(27,30);
  display.print("Hello, world!");
  display.display();
  
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

}
void loop() {
}
