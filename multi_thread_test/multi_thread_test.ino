#include "Timer.h"

void func ( void (*f)(int, int) ) {;}
void print ( int x, int y) {
  printf("%d\n", x);
}

Timer t1(1000, AUTORESET, print);

void setup() {
  // put your setup code here, to run once:

}




void loop() {
  func(print); delay(1000);
  // put your main code here, to run repeatedly:

}
