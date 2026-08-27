#pragma once
#include <string>

class Candlestick
{
public:
    std::string time;   // time bucket label (e.g., "2020-03-17 17:01")
    double open;          // opening price for the bucket
    double high;          // highest price for the bucket
    double low;          // lowest price for the bucket
    double close;         // closing price for the bucket


    // Constructor simply stores the OHLC values.
    Candlestick(std::string t,
                double o,
                double h,
                double l,
                double c)
        : time(t), open(o), high(h), low(l), close(c) {}
};