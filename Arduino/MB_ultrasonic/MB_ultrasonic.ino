#include <SimpleTimer.h>
#include <Trends.h>

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
  int direct;  // 0 when not moving, 1 when moving away, -1 when moving towards
  float slope = distance_hist.getSlopeOfAverage();

  if (slope < 0.3 && slope > -0.3) direct = 0;
  else if (slope >= 0.3) direct = 1;
  else direct = -1;
  
  Serial.println(slope);
  
  return direct;
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
        if (getMovingDirection() == -1) {
          Serial.println("Close and moving closer"); 
        }
//        sendDistanceData(2);
      }
      read_cnt = 0;
    }
    else read_cnt++;
  }
}

void loop() {
    timer.run();
}
