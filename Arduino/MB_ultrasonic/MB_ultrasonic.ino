#include <SimpleTimer.h>

SimpleTimer timer;
const int pwPin1 = 9;
long pulse1, sensor1;

void setup () {
  Serial.begin(9600);
  pinMode(pwPin1, INPUT);
  timer.setInterval(100, read_sensor);
}

void read_sensor(){
  pulse1 = pulseIn(pwPin1, HIGH);
  sensor1 = pulse1/147;
  Serial.print(sensor1);
  Serial.println(" ");
}

void loop() {
    timer.run();
}
