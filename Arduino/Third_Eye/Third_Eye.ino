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
const int DIST_VCLOSE = 59; // ceil(1.5 m * 39.37 in)
const int DIST_CLOSE = 98; // ceil(2.5 m * 39.37 in)
const int SENSOR_PIN = 9;

// Global variables
SimpleTimer timer;
int sensor_distance, sensor_raw;
int read_cnt = 0;
Trends distance_hist(5, 5);

// Pin arrays
byte pin_mode[TOTAL_PINS];
byte pin_state[TOTAL_PINS];
byte pin_pwm[TOTAL_PINS];
byte pin_servo[TOTAL_PINS];
Servo servos[MAX_SERVOS];

void setup()
{
  Serial.begin(57600);
  
  /* Default all to digital input */
  for (int pin = 0; pin < TOTAL_PINS; pin++)
  {
    // Set pin to input with internal pull up
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);

    // Save pin mode and state
    pin_mode[pin] = INPUT;
    pin_state[pin] = LOW;
  }
  
  // Set name here (max 10 chars)
  ble_set_name("Third Eye");
  
  // Set up sensor
  pinMode(SENSOR_PIN, INPUT);
  timer.setInterval(75, readSensor);
  
  // Init. and start BLE library.
  ble_begin();
}

//// Ultrasonic code

void sendDistanceData(int level, directions_t dir) {
  const int str_len = 2;
  uint8_t str[str_len+1];  // Data plus null  
  
  str[0] = level;
  str[1] = dir;
  
  sendCustomData(str, str_len); 
}

int getMovingDirection() {
  float slope = distance_hist.getSlopeOfAverage();

  Serial.print("Slope: ");  // DEBUG ONLY?
  Serial.println(slope);

  if (slope < 0.3 && slope > -0.3) return NOT_MOVING;
  else if (slope >= 0.3) return TOWARDS;
  else if (slope <= -0.3) return AWAY;
  else {
    Serial.print("getMovingDirection() - slope was equal to ");
    Serial.println(slope);
    return UNDETERMINED;
  }
}

void readSensor() {
  if (!ble_connected()) return;
  sensor_raw = pulseIn(SENSOR_PIN, HIGH);
  sensor_distance = sensor_raw/147;
  if (sensor_distance != 246) {
    distance_hist.addValue(sensor_distance);
    
    if (read_cnt == 5) {
      int avg = distance_hist.getAverage();
      
      if (avg < DIST_VCLOSE) {
          Serial.println("Very close (1)");
          sendDistanceData(1, (directions_t)getMovingDirection());
      }
      else if (avg < DIST_CLOSE && avg > DIST_VCLOSE) {
        Serial.println("Close (2)"); 
        sendDistanceData(2, (directions_t)getMovingDirection());
      }
      else {
        getMovingDirection(); // DEBUG ONLY?
      }
      read_cnt = 0;
    }
    else read_cnt++;
  }
}

//// End ultrasonic code

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
  ble_write_string(data, len+1);
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
