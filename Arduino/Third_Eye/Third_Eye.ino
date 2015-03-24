#include <Servo.h>
#include <SPI.h>
#include <SimpleTimer.h>
#include <Trends.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include "Boards.h"
#include "Third_Eye.h"

#define PROTOCOL_MAJOR_VERSION   0
#define PROTOCOL_MINOR_VERSION   0
#define PROTOCOL_BUGFIX_VERSION  2

#define PIN_CAPABILITY_NONE      0x00
#define PIN_CAPABILITY_DIGITAL   0x01
#define PIN_CAPABILITY_ANALOG    0x02
#define PIN_CAPABILITY_PWM       0x04
#define PIN_CAPABILITY_SERVO     0x08
#define PIN_CAPABILITY_I2C       0x10

// Pin modes
//#define INPUT                 0x00 // defined in wiring.h
//#define OUTPUT                0x01 // defined in wiring.h
#define ANALOG                  0x02 // analog pin in analogInput mode
#define PWM                     0x03 // digital pin in PWM output mode
#define SERVO                   0x04 // digital pin in Servo output mode

// Ultrasonic sensor
const int DIST_VVCLOSE = 60; // ceil(1.5 m * 39.37 in/m)
const int DIST_VCLOSE = 98; // ceil(2.5 m * 39.37 in/m)
const int DIST_CLOSE = 138; // ceil(3.5 m * 39.37 in/m)
const int SENSOR_PIN = 9;
const int READ_INTERVAL = 75;  // in ms, can't be less than 50 ms due to sensor limitation

// Global variables
SimpleTimer timer;
int sensor_distance, sensor_raw;
int read_cnt = 0;
Trends distance_hist(5, 5);

void setup()
{
  Serial.begin(57600);
  
  // Set name here (max 10 chars)
  ble_set_name("Third Eye");
  
  // Set up sensor
  pinMode(SENSOR_PIN, INPUT);
  timer.setInterval(READ_INTERVAL, readSensor);
  
  // Init. and start BLE library.
  ble_begin();
}

//// Ultrasonic code
// --------------------------------------------------------------------

int getMovingDirection() {
  float slope = distance_hist.getSlopeOfAverage();

  Serial.print("Slope: ");  // DEBUG ONLY?
  Serial.println(slope);

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
  float slope = distance_hist.getSlopeOfAverage();
  // (1 inch/(375 milliseconds) = 0.24384 kilometers/hour) * -1
  // Multiplied by -1 so that positive is moving towards and negative is moving away
  float speed_diff_kph = slope * -0.24384;
  alert_t level = ALERT_UNKNOWN;
  const int str_len = 1;
  uint8_t str[str_len + 1];  // Data plus null

  // Check distance first, then speed
  if (distance <= DIST_CLOSE && distance > DIST_VCLOSE)
  {
    if (speed_diff_kph <= 5.4 && speed_diff_kph > 3.6)
    {
      str[0] = ALERT_3;
    }
    else if (speed_diff_kph <= 10.8 && speed_diff_kph > 5.4)
    {
      str[0] = ALERT_2;
    }
    else if (speed_diff_kph > 10.8)
    {
      str[0] = ALERT_1;
    }
    else str[0] = ALERT_4; 
  }
  else if (distance <= DIST_VCLOSE && distance > DIST_VVCLOSE)
  {
    if (speed_diff_kph <= 3.6 && speed_diff_kph > 2.4)
    {
      str[0] = ALERT_3;
    }
    else if (speed_diff_kph <= 7.2 && speed_diff_kph > 3.6)
    {
      str[0] = ALERT_2;
    }
    else if (speed_diff_kph > 7.2)
    {
      str[0] = ALERT_1;
    }
  }
  else if (distance <= DIST_VVCLOSE)
  {
    if (speed_diff_kph <= 1.8 && speed_diff_kph > 1.2)
    {
      str[0] = ALERT_3;
    }
    else if (speed_diff_kph <= 3.6 && speed_diff_kph > 1.8)
    {
      str[0] = ALERT_2;
    }
    else if (speed_diff_kph > 3.6)
    {
      str[0] = ALERT_1;
    }
  }
  else
  {
    str[0] = ALERT_5;
  }
  sendCustomData(str, str_len); 
}

void readSensor() {
  if (!ble_connected()) return;
  sensor_raw = pulseIn(SENSOR_PIN, HIGH);
  sensor_distance = sensor_raw/147;
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

static byte buf_len = 0;

void ble_write_string(byte *bytes, uint8_t len)
{
  if (buf_len + len > 20)
  {
    for (int j = 0; j < 15000; j++)
      ble_do_events();
    
    buf_len = 0;
  }
  
  for (int j = 0; j < len; j++)
  {
    ble_write(bytes[j]);
    buf_len++;
  }
    
  if (buf_len == 20)
  {
    for (int j = 0; j < 15000; j++)
      ble_do_events();
    
    buf_len = 0;
  }  
}

void sendCustomData(uint8_t *buf, uint8_t len)
{
  uint8_t data[20] = "Z";
  memcpy(&data[1], buf, len);
  ble_write_string(data, len + 1);
}

byte queryDone = false;

void loop()
{
  while(ble_available())
  {
    byte cmd;
    cmd = ble_read();
//    Serial.write(cmd);
    
    // Do a sensor check
    timer.run();
    
    // Parse data here
    switch (cmd)
    {
      case 'V': // query protocol version
        {
          byte buf[] = {'V', 0x00, 0x00, 0x01};
          ble_write_string(buf, 4);
        }
        break;
        
      case 'Z': // custom data
        {
          byte len = ble_read();
          byte buf[len];
          for (int i=0;i<len;i++)
            buf[i] = ble_read();
          Serial.println("->");
          Serial.print("Received: ");
          Serial.print(len);
          Serial.println(" byte(s)");
          Serial.print(" Hex: ");
          for (int i=0;i<len;i++)
            Serial.print(buf[i], HEX);
          Serial.println();
        }
    }

    // send out any outstanding data
    ble_do_events();
    buf_len = 0;
    
    return; // only do this task in this loop
  }

  // process text data
  if (Serial.available())
  {
    byte d = 'Z';
    ble_write(d);

    delay(5);
    while(Serial.available())
    {
      d = Serial.read();
      ble_write(d);
    }
    
    ble_do_events();
    buf_len = 0;
    
    return;    
  }

  // No input data, no commands, process other data
  if (!ble_connected())
    queryDone = false; // reset query state
  
  // Do a sensor check
  timer.run();
  
  ble_do_events();
  buf_len = 0;
}
