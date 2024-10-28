// signal_processing.h
#include <vector>
#include <complex>
#include "signal_processing.h"
#include "filt.h"
#include <algorithm>
#include <complex>
#include <fftw3.h>

double median(std::vector<double> &v)
{
    size_t n = v.size() / 2;
    nth_element(v.begin(), v.begin() + n, v.end());
    if (v.size() % 2 == 1)
    {
        return v[n];
    }
    else
    {
        return 0.5 * (v[n] + v[n - 1]);
    }
}

// Compute the average of time series, excluding the most deviant series
std::vector<double> averageTimeSeries(const std::vector<std::vector<double>> &series)
{
    int numSeries = series.size();
    int seriesLength = series[0].size();

    std::vector<double> result(seriesLength, 0);
    std::vector<double> diffs(numSeries, 0);

    // Compute the sum of absolute differences from the median for each series
    for (int i = 0; i < seriesLength; ++i)
    {
        std::vector<double> currentPointValues;
        for (int j = 0; j < numSeries; ++j)
        {
            currentPointValues.push_back(series[j][i]);
        }
        double medianValue = median(currentPointValues);

        for (int j = 0; j < numSeries; ++j)
        {
            diffs[j] += std::abs(series[j][i] - medianValue);
        }
    }

    // Find the series index with the largest difference
    int maxDiffIndex = std::distance(diffs.begin(), std::max_element(diffs.begin(), diffs.end()));

    // Compute the average, excluding the series with max difference
    for (int i = 0; i < seriesLength; ++i)
    {
        for (int j = 0; j < numSeries; ++j)
        {
            if (j != maxDiffIndex)
            {
                result[i] += series[j][i];
            }
        }
        result[i] /= (numSeries - 1); // Since we're excluding one series
    }

    return result;
}

std::vector<int16_t> complexToI16(const std::vector<std::complex<double>> &data)
{
    std::vector<int16_t> out(data.size());

    for (size_t i = 0; i < data.size(); i++)
    {
        double magnitude = std::abs(data[i]);

        // Clip or scale the magnitude if necessary. This example just clips it.
        magnitude = std::min(magnitude, static_cast<double>(std::numeric_limits<int16_t>::max()));

        out[i] = static_cast<int16_t>(magnitude);
    }

    return out;
}
std::vector<int32_t> complexToI32(const std::vector<std::complex<double>> &data)
{
    std::vector<int32_t> out(data.size());

    for (size_t i = 0; i < data.size(); i++)
    {
        double magnitude = std::abs(data[i]);

        // Clip or scale the magnitude if necessary. This example just clips it.
        magnitude = std::min(magnitude, static_cast<double>(std::numeric_limits<int32_t>::max()));

        out[i] = static_cast<int32_t>(magnitude);
    }

    return out;
}

// Function to compute the analytic signal using the Hilbert transform
std::vector<std::complex<double>> hilbert(const std::vector<double> &data)
{
    int N = data.size();

    std::vector<std::complex<double>> out(N);

    fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *fft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

    // Populate input with data
    for (int i = 0; i < N; i++)
    {
        in[i][0] = data[i];
        in[i][1] = 0;
    }

    // Perform FFT
    fftw_plan forward = fftw_plan_dft_1d(N, in, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(forward);

    // Zero out negative frequencies (except the Nyquist if N is even)
    fft_result[0][1] = 0;
    for (int i = 1; i < (N + 1) / 2; ++i)
    {
        fft_result[i][0] *= 2;
        fft_result[i][1] *= 2;
    }
    for (int i = (N + 1) / 2; i < N; ++i)
    {
        fft_result[i][0] = 0;
        fft_result[i][1] = 0;
    }

    // Perform inverse FFT
    fftw_plan backward = fftw_plan_dft_1d(N, fft_result, in, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(backward);

    // Store the result
    for (int i = 0; i < N; i++)
    {
        out[i] = std::complex<double>(in[i][0] / N, in[i][1] / N);
    }

    fftw_destroy_plan(forward);
    fftw_destroy_plan(backward);
    fftw_free(in);
    fftw_free(fft_result);

    return out;
}
