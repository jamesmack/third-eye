#include <Servo.h>
#include <SPI.h>
#include <SimpleTimer.h>
#include <QueueList.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include "Boards.h"

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

// Ultrasonic sensor consts
const int DIST_VCLOSE = 39; // ceil(1 m * 39.37 in)
const int DIST_CLOSE = 79; // ceil(2 m * 39.37 in)
const int SENSOR_PIN = 9;

// Averaging defines
const int AVG_POWER = 2;     // 1 for 2 readings, 2 for 4 readings, 3 for 8, etc.
const int AVG_COUNT = pow(2, AVG_POWER);

// Global variables
SimpleTimer timer;
long sensor_distance, sensor_raw;
QueueList <int> sensor_readings;

// Pin arrays
byte pin_mode[TOTAL_PINS];
byte pin_state[TOTAL_PINS];
byte pin_pwm[TOTAL_PINS];
byte pin_servo[TOTAL_PINS];
Servo servos[MAX_SERVOS];

void setup()
{
  Serial.begin(57600);
  Serial.println("BLE Arduino Slave");
  
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

int average(QueueList<int> & q) {
    long sum = 0;
    for (int i = 0; i < AVG_COUNT; i++) {
        sum += q.pop();
    }
    return (sum >> AVG_POWER);
}

void sendDistanceData(int level) {
  const int str_len = 1;
  uint8_t str[str_len+1];  // String plus null  
  
  if (level == 1) {
    str[0] = 1;
  }
  else if (level == 2) {
    str[0] = 2;
  }
  else {
    return;
  }
  
  sendCustomData(str, str_len); 
}

void readSensor() {
  if (!ble_connected()) return;
  sensor_raw = pulseIn(SENSOR_PIN, HIGH);
  sensor_distance = sensor_raw/147;
  sensor_readings.push(sensor_distance);
  if(sensor_readings.count() > AVG_COUNT) {
        int avg = average(sensor_readings);
        if (avg < DIST_VCLOSE) {
          Serial.println("Very close (1)");
          sendDistanceData(1);
        }
        else if (avg < DIST_CLOSE && avg > DIST_VCLOSE) {
          Serial.println("Close (2)"); 
          sendDistanceData(2);
        }
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

byte reportDigitalInput()
{
  if (!ble_connected())
    return 0;

  static byte pin = 0;
  byte report = 0;
  
  if (!IS_PIN_DIGITAL(pin))
  {
    pin++;
    if (pin >= TOTAL_PINS)
      pin = 0;
    return 0;
  }
  
  if (pin_mode[pin] == INPUT)
  {
      byte current_state = digitalRead(pin);
            
      if (pin_state[pin] != current_state)
      {
        pin_state[pin] = current_state;
        byte buf[] = {'G', pin, INPUT, current_state};
        ble_write_string(buf, 4);
        
        report = 1;
      }
  }
  
  pin++;
  if (pin >= TOTAL_PINS)
    pin = 0;
    
  return report;
}

void reportPinCapability(byte pin)
{
  byte buf[] = {'P', pin, 0x00};
  byte pin_cap = 0;
                    
  if (IS_PIN_DIGITAL(pin))
    pin_cap |= PIN_CAPABILITY_DIGITAL;
            
  if (IS_PIN_ANALOG(pin))
    pin_cap |= PIN_CAPABILITY_ANALOG;

  if (IS_PIN_PWM(pin))
    pin_cap |= PIN_CAPABILITY_PWM;

  if (IS_PIN_SERVO(pin))
    pin_cap |= PIN_CAPABILITY_SERVO;

  buf[2] = pin_cap;
  ble_write_string(buf, 3);
}

void reportPinServoData(byte pin)
{
//  if (IS_PIN_SERVO(pin))
//    servos[PIN_TO_SERVO(pin)].write(value);
//  pin_servo[pin] = value;
  
  byte value = pin_servo[pin];
  byte mode = pin_mode[pin];
  byte buf[] = {'G', pin, mode, value};         
  ble_write_string(buf, 4);
}

byte reportPinAnalogData()
{
  if (!ble_connected())
    return 0;
    
  static byte pin = 0;
  byte report = 0;
  
  if (!IS_PIN_DIGITAL(pin))
  {
    pin++;
    if (pin >= TOTAL_PINS)
      pin = 0;
    return 0;
  }
  
  if (pin_mode[pin] == ANALOG)
  {
    uint16_t value = analogRead(pin);
    byte value_lo = value;
    byte value_hi = value>>8;
    
    byte mode = pin_mode[pin];
    mode = (value_hi << 4) | mode;
    
    byte buf[] = {'G', pin, mode, value_lo};         
    ble_write_string(buf, 4);
  }
  
  pin++;
  if (pin >= TOTAL_PINS)
    pin = 0;
    
  return report;
}

void reportPinDigitalData(byte pin)
{
  byte state = digitalRead(pin);
  byte mode = pin_mode[pin];
  byte buf[] = {'G', pin, mode, state};
  ble_write_string(buf, 4);
}

void reportPinPWMData(byte pin)
{
  byte value = pin_pwm[pin];
  byte mode = pin_mode[pin];
  byte buf[] = {'G', pin, mode, value};         
  ble_write_string(buf, 4);
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
    Serial.write(cmd);
    
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
      
      case 'C': // query board total pin count
        {
          byte buf[2];
          buf[0] = 'C';
          buf[1] = TOTAL_PINS; 
          ble_write_string(buf, 2);
        }        
        break;
      
      case 'M': // query pin mode
        {  
          byte pin = ble_read();
          byte buf[] = {'M', pin, pin_mode[pin]}; // report pin mode
          ble_write_string(buf, 3);
        }  
        break;
      
      case 'S': // set pin mode
        {
          byte pin = ble_read();
          byte mode = ble_read();
          
          if (IS_PIN_SERVO(pin) && mode != SERVO && servos[PIN_TO_SERVO(pin)].attached())
            servos[PIN_TO_SERVO(pin)].detach();
  
          /* ToDo: check the mode is in its capability or not */
          /* assume always ok */
          if (mode != pin_mode[pin])
          {              
            pinMode(pin, mode);
            pin_mode[pin] = mode;
          
            if (mode == OUTPUT)
            {
              digitalWrite(pin, LOW);
              pin_state[pin] = LOW;
            }
            else if (mode == INPUT)
            {
              digitalWrite(pin, HIGH);
              pin_state[pin] = HIGH;
            }
            else if (mode == ANALOG)
            {
              if (IS_PIN_ANALOG(pin)) {
                if (IS_PIN_DIGITAL(pin)) {
                  pinMode(PIN_TO_DIGITAL(pin), LOW);
                }
              }
            }
            else if (mode == PWM)
            {
              if (IS_PIN_PWM(pin))
              {
                pinMode(PIN_TO_PWM(pin), OUTPUT);
                analogWrite(PIN_TO_PWM(pin), 0);
                pin_pwm[pin] = 0;
                pin_mode[pin] = PWM;
              }
            }
            else if (mode == SERVO)
            {
              if (IS_PIN_SERVO(pin))
              {
                pin_servo[pin] = 0;
                pin_mode[pin] = SERVO;
                if (!servos[PIN_TO_SERVO(pin)].attached())
                  servos[PIN_TO_SERVO(pin)].attach(PIN_TO_DIGITAL(pin));
              }
            }
          }
            
  //        if (mode == ANALOG)
  //          reportPinAnalogData(pin);
          if ( (mode == INPUT) || (mode == OUTPUT) )
            reportPinDigitalData(pin);
          else if (mode == PWM)
            reportPinPWMData(pin);
          else if (mode == SERVO)
            reportPinServoData(pin);
        }
        break;

      case 'G': // query pin data
        {
          byte pin = ble_read();
          reportPinDigitalData(pin);
        }
        break;
        
      case 'T': // set pin digital state
        {
          byte pin = ble_read();
          byte state = ble_read();
          
          digitalWrite(pin, state);
          reportPinDigitalData(pin);
        }
        break;
      
      case 'N': // set PWM
        {
          byte pin = ble_read();
          byte value = ble_read();
          
          analogWrite(PIN_TO_PWM(pin), value);
          pin_pwm[pin] = value;
          reportPinPWMData(pin);
        }
        break;
      
      case 'O': // set Servo
        {
          byte pin = ble_read();
          byte value = ble_read();

          if (IS_PIN_SERVO(pin))
            servos[PIN_TO_SERVO(pin)].write(value);
          pin_servo[pin] = value;
          reportPinServoData(pin);
        }
        break;
      
      case 'A': // query all pin status
        {
          for (int pin = 0; pin < TOTAL_PINS; pin++)
          {
            reportPinCapability(pin);
            if ( (pin_mode[pin] == INPUT) || (pin_mode[pin] == OUTPUT) )
              reportPinDigitalData(pin);
            else if (pin_mode[pin] == PWM)
              reportPinPWMData(pin);
            else if (pin_mode[pin] == SERVO)
              reportPinServoData(pin);  
          }
          queryDone = true; 
        }
        break;

      case 'P': // query pin capability
        {
          byte pin = ble_read();
          reportPinCapability(pin);
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

  // No input data, no commands, process analog data
  if (!ble_connected())
    queryDone = false; // reset query state
    
//  if (queryDone) // only report data after the query state
//  { 
//    byte input_data_pending = reportDigitalInput();  
//    if (input_data_pending)
//    {
//      ble_do_events();
//      buf_len = 0;
//      
//      return; // only do this task in this loop
//    }
//  
//    reportPinAnalogData();
//    
//    ble_do_events();
//    buf_len = 0;
//    
//    return;  
//  }
  
  // Do a sensor check
  timer.run();
  
  ble_do_events();
  buf_len = 0;
}

