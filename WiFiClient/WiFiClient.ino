/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "MrWhite"
#define STAPSK  "Feketebikapata1965!"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

/*const char* host = "djxmmx.net";
const uint16_t port = 17;*/
const char* host = "frank.lan";
const uint16_t port = 8080;
bool connectionOK = false;

WiFiClient client;

void initMonitor () {  Serial.begin(115200); Serial.println("Monitor initalized"); }

void initLED () {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D4, OUTPUT); 
  //pinMode(D5, OUTPUT);
  //digitalWrite(D5, HIGH);
  Serial.println("LED initialized");
} 

void ledOn(unsigned char pin) { digitalWrite(pin, LOW);  }
void ledOff(unsigned char pin) { digitalWrite(pin, HIGH); }  // Turn the LED on (Note that LOW is the voltage level

/**
 * ledBlink
 */
void ledBlink(int ms, int rate) {
  while (ms / rate > 0) {
    ledOn(D4); ledOn(LED_BUILTIN);
    //Serial.println("blink");
    delay(rate / 2);
    ledOff(D4); ledOff (LED_BUILTIN);
    delay(rate / 2);
    ms = ms - rate;
  }
}

/**
 * connectToWiFi
 */
bool connectToWiFi (char const* ssid, char const* password) {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

/**
 * connectToServer
 */
bool connectToServer (char const* host, uint16_t port) {
  
  bool isConnectionEstablished = false;
  
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  
  // Use WiFiClient class to create TCP connections
  // WiFiClient client;

  isConnectionEstablished = client.connect(host, port);
  std::string str(host);
  if (isConnectionEstablished) {
     Serial.print("connection to ");
     Serial.print(host);
     Serial.println(" succesful");
  } else {
     Serial.print("connection to ");
     Serial.print(host);
     Serial.println(" failed");
  }
  return isConnectionEstablished;
}

/**
 * closeConnection
 */
bool closeConnection() {
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void ledTest(int dly) {
  for (int i=1;i<5;i++) {
    Serial.print("inside ledtest "); Serial.println(i);

    digitalWrite(D4, HIGH);
    delay(dly);
    digitalWrite(D4, LOW);
    delay(dly);
  }
}
/**
 * setup
 */
void setup() {
  initMonitor();
  initLED();

  ledOn(D4);
  connectToWiFi (ssid, password);
  ledOff(D4);

}

/**
 * loop
 */
void loop() {
 
  if (!client.connected()) {
    Serial.println ("*** host not connected");
    if (connectToServer(host, port)) {
      connectionOK = true;
    } else {
      connectionOK = false;
      ledBlink (500,60); //delay(60000); 
      ledOff(D4);
      delay(6000);
      return;
    }
  }

  // Send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from ESP8266");
  }

  // waiting for data to be available
  Serial.println("waiting data to be available");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 50000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      ledBlink (200,60); //delay(60000);
      ledOff(D4);
      delay(150);
      ledBlink (200,60); //delay(60000);
      ledOff(D4);
      delay(3000);
      return;
    }
  }

  char buff[80];
  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  int i = 0;
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    buff [i++] = ch;
    Serial.print(ch);
  }
  Serial.println("----end of received bytes");
  ledOff(LED_BUILTIN);
  if ((i>2) && buff[0]=='l' && buff[1]=='e' && buff[2]=='d') {
    if (strcmp( buff, "led")) ;
    Serial.println("ledtest started");
    ledTest(300);
    Serial.println("ledtest finished");
    }

  ledOn(D4);  
  Serial.println("wait before next loop");
  //closeConnection ();
  ledBlink (6000,1000); //delay(60000);delay(3000); // execute once every 3000ms minutes, don't flood remote service
}
