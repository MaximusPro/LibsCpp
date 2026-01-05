#pragma once
#include "Defines.h"
#include <iomanip> // для std::setprecision
#include <cmath>   // для std::nan
#include"ta_libc.h"

struct OHLC {
    double high;
    double low;
    double close;
};
std::vector<double> calculateEMA(const std::vector<double>& data, int period);



std::vector<double> calculateNATR(const std::vector<OHLC>& data, int period);

vector<double> TA_SMA(const std::vector<double>& data, int period);
