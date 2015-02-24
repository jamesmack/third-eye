#include <SimpleTimer.h>
#include <QueueList.h>

const int dist_vclose = 39; // ceil(1 m * 39.37 in)
const int dist_close = 79; // ceil(2 m * 39.37 in)
const int sensor_pin = 9;
const int avg_power = 2;     // 1 for 2 readings, 2 for 4 readings, 3 for 8, etc.
const int avg_count = pow(2, avg_power);

SimpleTimer timer;
long distance, reading;
QueueList <int> sensor_readings;

int average(QueueList<int> & q, int avg_power) {
    long sum = 0;
    for (int i = 0; i < avg_count; i++) {
        sum += q.pop();
    }
    return (sum >> avg_power);
}

void readSensor() {
  reading = pulseIn(sensor_pin, HIGH);
  distance = reading/147;
  sensor_readings.push(distance);
  if(sensor_readings.count() > avg_count) {
        int avg = average(sensor_readings, avg_power);
        if (avg < dist_vclose) {
          Serial.println("Very close");
        }
        else if (avg < dist_close && avg > dist_vclose) {
         Serial.println("Close"); 
        }
  }
}

void setup () {
  Serial.begin(9600);
  pinMode(sensor_pin, INPUT);
  timer.setInterval(75, readSensor);
}

void loop() {
    timer.run();
}
