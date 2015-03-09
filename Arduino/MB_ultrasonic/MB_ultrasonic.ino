#include <SimpleTimer.h>
#include <Trends.h>

enum directions_t {TOWARDS = 1, AWAY = 2, NOT_MOVING = 3, UNDETERMINED = 4};

const int DIST_VCLOSE = 60; // ceil(1.5 m * 39.37 in)
const int DIST_CLOSE = 98; // ceil(2.5 m * 39.37 in)
// warn between 80-200 if something closing
// reading can fluct between 20-60 when right next to car
// use moving avg to build collection of last 10-20 readings for best fit
const unsigned int SENSOR_PIN = 9;

SimpleTimer timer;
int sensor_distance, sensor_raw;
Trends distance_hist(5, 5);
int read_cnt = 0;

int getMovingDirection() {
  float slope = distance_hist.getSlopeOfAverage();

  if (slope < 0.3 && slope > -0.3) return NOT_MOVING;
  else if (slope >= 0.3) return TOWARDS;
  else if (slope <= -0.3) return AWAY;
  else {
    Serial.print('Error in getMovingDirection - slope was equal to '); 
    Serial.println(slope);
    return UNDETERMINED;
  }
}

void readSensor() {
//  if (!ble_connected()) return;
  sensor_raw = pulseIn(SENSOR_PIN, HIGH);
  sensor_distance = sensor_raw/147;
  if (sensor_distance != 246) {
    distance_hist.addValue(sensor_distance);
    
    if (read_cnt == 5) {
      int avg = distance_hist.getAverage();
      
      if (avg < DIST_VCLOSE) {
          Serial.println("Very close (1)");
//          sendDistanceData(1);
      }
      else if (avg < DIST_CLOSE && avg > DIST_VCLOSE) {
        Serial.println("Close (2)"); 
        if (getMovingDirection() == TOWARDS) {
          Serial.println("Close and moving closer"); 
        }
//        sendDistanceData(2);
      }
      else {
        Serial.println(getMovingDirection()); 
      }
      read_cnt = 0;
    }
    else read_cnt++;
  }
}

void loop() {
    timer.run();
}

void setup() {
 Serial.begin(9600); 
 pinMode(SENSOR_PIN, INPUT);
 timer.setInterval(75, readSensor);
}