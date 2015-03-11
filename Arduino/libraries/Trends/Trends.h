//
//    FILE: Trends.h
//  AUTHOR: Rob Tillaart (RunningAverage) and James Mackenzie
// PURPOSE: Data smoothing and trend library for Arduino
//     URL: Modified from http://arduino.cc/playground/Main/RunningAverage

#ifndef Trends_h
#define Trends_h

#include "Arduino.h"

class Trends
{
public:
    Trends(void);
    Trends(int, int);
    ~Trends();

    void clear();
    void addValue(int);
    void fillValue(int, int);

    int getAverage();
    float getSlopeOfAverage();

    int getWeightedAverage();

    int getElement(uint8_t idx);
    uint8_t getSize() { return _size; }
    uint8_t getCount() { return _cnt; }

protected:
    uint8_t _size;
    uint8_t _cnt;
    uint8_t _idx;
    int   _sum;
    int * _ar;

    uint8_t _size_ma_hist;
    uint8_t _cnt_ma_hist;
    uint8_t _idx_ma_hist;
    int   _sum_ma_hist;
    int * _ar_ma_hist;
};

#endif
