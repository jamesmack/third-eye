//
//    FILE: Trends.h
//  AUTHOR: Rob Tillaart (RunningAverage) and James Mackenzie
// PURPOSE: Data smoothing and trend library for Arduino
//     URL: Modified from http://arduino.cc/playground/Main/RunningAverage

#include "Trends.h"
#include <stdlib.h>

Trends::Trends(int n, int m)
{
    _size = n;
    _ar = (int*) malloc(_size * sizeof(int));
    if (_ar == NULL) _size = 0;

    _size_ma_hist = m;
    _ar_ma_hist = (int*) malloc(_size_ma_hist * sizeof(int));
    if (_ar_ma_hist == NULL) _size = 0;
    clear();
}

Trends::~Trends()
{
    if (_ar != NULL) free(_ar);
}

// resets all counters
void Trends::clear()
{
    _cnt = 0;
    _idx = 0;
    _sum = 0;
    for (int i = 0; i< _size; i++) _ar[i] = 0;

    _cnt_ma_hist = 0;
    _idx_ma_hist = 0;
    _sum_ma_hist = 0;
    for (int i = 0; i< _size_ma_hist; i++) _ar_ma_hist[i] = 0;
}

// adds a new value to the data-set
void Trends::addValue(int f)
{
    if (_ar == NULL) return;
    _sum -= _ar[_idx];
    _ar[_idx] = f;
    _sum += _ar[_idx];
    _idx++;
    if (_idx == _size) _idx = 0;  // faster than %
    if (_cnt < _size) _cnt++;
}

// returns the average of the data-set added sofar
int Trends::getAverage()
{
    if (_cnt == 0) return NAN;

    // Compute result
    int result = _sum / _cnt;

    // Add to MA history
    _sum_ma_hist -= _ar_ma_hist[_idx_ma_hist];
    _ar_ma_hist[_idx_ma_hist] = result;
    _sum_ma_hist += result;
    _idx_ma_hist++;
    if (_idx_ma_hist == _size_ma_hist) _idx_ma_hist = 0;
    if (_cnt_ma_hist < _size_ma_hist) _cnt_ma_hist++;

    return result;
}

// returns the value of an element if exist, 0 otherwise
int Trends::getElement(uint8_t idx)
{
    if (idx >=_cnt) return NAN;
    return _ar[idx];
}

// returns weighted average of the data set
int Trends::getWeightedAverage()
{
    if (_cnt == 0) return NAN;
    long numerator = 0;
    long denominator = (_cnt*(_cnt + 1)) >> 1; // Compute n+n-1+...+1
    int i = _idx;
    int c = _cnt;

    do {
        i--;
        if (i == -1) i = _cnt - 1;
        numerator += c*_ar[i];
        c--;
    } while (i != _idx);

    return numerator / denominator;
}

// returns slope of data
float Trends::getSlopeOfAverage()
{
    if (_cnt_ma_hist < _size_ma_hist) return NAN;

    long wsum = 0;
    int i = _idx_ma_hist;
    int c = _cnt_ma_hist;

    do {
        i--;
        if (i == -1) i = _cnt_ma_hist - 1;
        // Add to weighted sum
        wsum += c*_ar_ma_hist[i];
        c--;
    } while (i != _idx_ma_hist);

    // Algorithm to calculate slope (from "Random Data", Bendat, Ch. 11.1.2)
    return (12.0*(wsum)-6*(_cnt_ma_hist+1)*(_sum_ma_hist))/((_cnt_ma_hist)*(_cnt_ma_hist-1)*(_cnt_ma_hist+1));
}

// fill the average with a value
// the param number determines how often value is added (weight)
// number should preferably be between 1 and _size
void Trends::fillValue(int value, int number)
{
    clear();
    for (int i = 0; i < number; i++)
    {
        addValue(value);
    }
}
