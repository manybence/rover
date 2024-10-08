#pragma once
#include <vector>
#include <complex>

#define FILTERTAPS 50
#define FREQ 10.0
#define PTC 1.15 // 1.25//1.15 //1.15

double median(std::vector<double> &v);
std::vector<double> averageTimeSeries(const std::vector<std::vector<double>> &series);
std::vector<int16_t> complexToI16(const std::vector<std::complex<double>> &data);
std::vector<int32_t> complexToI32(const std::vector<std::complex<double>> &data);
std::vector<std::complex<double>> hilbert(const std::vector<double> &data);
