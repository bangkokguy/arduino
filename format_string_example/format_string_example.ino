
String cleanString (String s, char delimiter) {
  Serial.println("cleanString");
  int len = (int)s.length();
  if (len > 10) return "len>10";
  if (len == 0) return "len=0";
  char * w = (char *)malloc(5); 
  int i=0;
  for (int j=0; j<=5; j++) {
    while (i<=len && s[i] != delimiter && (s[i] < '0' || s[i] > '9') ) i++;
    w [j] = s [i];
    i++;
    Serial.print (w [j]);
    }
  return "";
}

int getHour (String s, char delimiter) {
  Serial.print("getHour start " + s);
  int i = s.length();
  while (i >= 0 && s[i] != delimiter) {
    s [i] = ' ';
    i--;
  }
  if (i >= 0) s[i] = ' '; //delimiter
  Serial.print(" end " + s + "-->");
  return s.toInt();
}

int getMinute (String s, char delimiter) {
  Serial.print("getMinute start " + s);
  int i = 0;
  while (i <= s.length() && s[i] != delimiter) {
    s [i] = ' ';
    i++;
  }
  if (i <= s.length()) s[i] = ' '; //delimiter
  Serial.print(" end " + s + "-->");
  return s.toInt();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
/*  Serial.println("*** begin ***");
  Serial.println(cleanString(" 7ad:0 0ef", ':'));*/
  Serial.println(getHour (" 7:0 0ef", ':'));
  Serial.println(getHour ("1 7d:0 0ef", ':'));
  Serial.println(getHour (" 17:0 0ef", ':'));
  Serial.println(getHour (" 09:0 0ef", ':'));
  Serial.println(getHour ("090:0 0ef", ':'));
  Serial.println(getHour ("09:30", ':'));

  Serial.println(getMinute (" 7:0 0ef", ':'));
  Serial.println(getMinute ("1 7d:0 0ef", ':'));
  Serial.println(getMinute (" 17:0 0ef", ':'));
  Serial.println(getMinute (" 09:0 0ef", ':'));
  Serial.println(getMinute ("090:0 0ef", ':'));
  Serial.println(getMinute ("09:30", ':'));

  /*Serial.println("*** end ***");*/
  int a, b;
  float c;
  a = 7;
  b = 15;
  c = (float)(a * 100 + b) / 100;  
  Serial.println(c);
}

int incomingByte = 0;
String s = "";
void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    //Serial.print("I received: ");
    //Serial.println(incomingByte, DEC);
    Serial.print((char)incomingByte);
    s += ((char)incomingByte);
    if (incomingByte == 10) {
  //    Serial.print("\\r");
  ;
      incomingByte = Serial.read();
      if (incomingByte == 13) /*Serial.print("\\n")*/;
//      Serial.println (s);
      Serial.println(getHour(s, ':'));
    }
  }
}
