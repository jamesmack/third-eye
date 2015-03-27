#include <SimpleTimer.h>
#include <Trends.h>

enum directions_t {TOWARDS = 1, AWAY = 2, NOT_MOVING = 3, UNDETERMINED = 4};
enum alert_t {ALERT_UNKNOWN = 0, ALERT_1 = 1, ALERT_2 = 2, ALERT_3 = 3, ALERT_4 = 4, ALERT_5 = 5};

const int DIST_VVCLOSE = 60; // ceil(1.5 m * 39.37 in/m)
const int DIST_VCLOSE = 98; // ceil(2.5 m * 39.37 in/m)
const int DIST_CLOSE = 138; // ceil(3.5 m * 39.37 in/m)
const unsigned int SENSOR_PIN = 9;
const unsigned int LOOP_FOR_ALERT = 8;  // Number of times to send level 1, 2, 3 alert, corresponds to 3 seconds

SimpleTimer timer;
int sensor_distance, sensor_raw;
Trends distance_hist(5, 5);
int read_cnt = 0;
alert_t last_alert = ALERT_UNKNOWN;
unsigned int current_alert_loop = LOOP_FOR_ALERT;

// Fake functions
// --------------------------------------------------------------------
boolean ble_connected() {
  return true;
}

void sendCustomData(uint8_t *string, int length) {
  Serial.println(string[0]);
}
// --------------------------------------------------------------------


// Test program
// --------------------------------------------------------------------
int getMovingDirection() {
  float slope = distance_hist.getSlopeOfAverage();

  if (slope < 0.3 && slope > -0.3) return NOT_MOVING;
  else if (slope >= 0.3) return AWAY;
  else if (slope <= -0.3) return TOWARDS;
  else {
    Serial.print("getMovingDirection() - slope was equal to ");
    Serial.println(slope);
    return UNDETERMINED;
  }
}

void sendAlertLevel(int distance) {
  float speed_diff_kph;
  alert_t current_alert = ALERT_UNKNOWN;
  uint8_t str[2];  // One byte for data, one for null

  if (distance <= DIST_CLOSE)
  {
    // (1 inch/(375 milliseconds) = 0.24384 kilometers/hour) * -1
    // Multiplied by -1 so that positive is moving towards and negative is moving away
    speed_diff_kph = distance_hist.getSlopeOfAverage() * -0.24384;
  }
  else speed_diff_kph = 0;

  // Check distance first, then speed
  if (distance <= DIST_CLOSE && distance > DIST_VCLOSE)
  {
    if (speed_diff_kph <= 5.4 && speed_diff_kph > 3.6)
    {
      current_alert = ALERT_3;
    }
    else if (speed_diff_kph <= 10.8 && speed_diff_kph > 5.4)
    {
      current_alert = ALERT_2;
    }
    else if (speed_diff_kph > 10.8)
    {
      current_alert = ALERT_1;
    }
    else current_alert = ALERT_4;
  }
  else if (distance <= DIST_VCLOSE && distance > DIST_VVCLOSE)
  {
    if (speed_diff_kph <= 3.6 && speed_diff_kph > 2.4)
    {
      current_alert = ALERT_3;
    }
    else if (speed_diff_kph <= 7.2 && speed_diff_kph > 3.6)
    {
      current_alert = ALERT_2;
    }
    else if (speed_diff_kph > 7.2)
    {
      current_alert = ALERT_1;
    }
    else current_alert = ALERT_4;
  }
  else if (distance <= DIST_VVCLOSE)
  {
    if (speed_diff_kph <= 1.8 && speed_diff_kph > 1.2)
    {
      current_alert = ALERT_3;
    }
    else if (speed_diff_kph <= 3.6 && speed_diff_kph > 1.8)
    {
      current_alert = ALERT_2;
    }
    else if (speed_diff_kph > 3.6)
    {
      current_alert = ALERT_1;
    }
    else current_alert = ALERT_4;
  }
  else
  {
    current_alert = ALERT_5;
  }

  // If last alert unknown
  if (last_alert == ALERT_UNKNOWN) last_alert = current_alert;

  // If last alert less important or same importance, restart loop with new alert
  if ((last_alert >= current_alert) && (current_alert < ALERT_4))
  {
    current_alert_loop = LOOP_FOR_ALERT;
    last_alert = current_alert;
  }
  // If last alert more important than current alert, continue loop
  else if ((last_alert < current_alert) && (current_alert_loop > 0) && (last_alert < ALERT_4))
  {
    current_alert_loop--;
    current_alert = last_alert;
  }
  else last_alert = current_alert;

  str[0] = current_alert;
  sendCustomData(str, 2);
}

void readSensor() {
  if (!ble_connected()) return;
  sensor_raw = pulseIn(SENSOR_PIN, HIGH);
  sensor_distance = sensor_raw/147;
  sensor_distance = abs(sensor_distance);
  if (sensor_distance != 246) {
    distance_hist.addValue(sensor_distance);
    
    if (read_cnt == 5) {
      int avg = distance_hist.getAverage();
      sendAlertLevel(avg);
      read_cnt = 0;
    }
    else read_cnt++;
  }
}

// --------------------------------------------------------------------

void loop() {
    timer.run();
}

void setup() {
 Serial.begin(9600); 
 pinMode(SENSOR_PIN, INPUT);
 timer.setInterval(75, readSensor);
}
