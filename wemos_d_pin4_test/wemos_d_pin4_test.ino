

int ii;
int d[9]={16, 5, 4, 0, 17, 14, 12, 13, 15};
int t;

/**
 * Setup
 */
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  //digitalWrite(LED_BUILTIN, HIGH);
  //digitalWrite(LED_BUILTIN, LOW);
  for (int i=0;i < 9; i++) {
    pinMode(d[i], OUTPUT);
    digitalWrite(d[i], LOW);
  }
  Serial.println();
}

/**
 * loop
 */
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  delay(70);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);   delay(70);        
  delay(1000);
}
