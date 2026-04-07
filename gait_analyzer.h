#ifndef GAIT_ANALYZER_H
#define GAIT_ANALYZER_H

#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>

class GaitAnalyzer {
public:
    GaitAnalyzer(size_t window_size = 128) : m_size(window_size) {
        if ((m_size & (m_size - 1)) != 0) {
            m_size = 128; 
        }
    }

    void fft(std::vector<std::complex<double>>& a) {
        size_t n = a.size();
        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(a[i], a[j]);
        }

        for (int len = 2; len <= n; len <<= 1) {
            double ang = 2 * M_PI / len;
            std::complex<double> wlen(std::cos(ang), std::sin(ang));
            for (int i = 0; i < n; i += len) {
                std::complex<double> w(1);
                for (int j = 0; j < len / 2; j++) {
                    std::complex<double> u = a[i + j], v = a[i + j + len / 2] * w;
                    a[i + j] = u + v;
                    a[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }
    }

    double analyzeGait(const std::vector<double>& accel_mags, double sample_rate) {
        if (accel_mags.size() < m_size) return 0.0;

        double sum = 0;
        double sq_sum = 0;
        for (double d : accel_mags) {
            sum += d;
            sq_sum += d * d;
        }
        double mean = sum / accel_mags.size();
        double variance = (sq_sum / accel_mags.size()) - (mean * mean);
        double std_dev = std::sqrt(std::max(0.0, variance));

        if (std_dev < 0.05) {
            return 0.0; 
        }

        std::vector<std::complex<double>> complex_data(m_size);
        for (size_t i = 0; i < m_size; ++i) {
            complex_data[i] = std::complex<double>(accel_mags[i] - mean, 0);
        }

        fft(complex_data);

        double freq_step = sample_rate / m_size;
        double gait_energy = 0;
        double tremor_energy = 0;
        double peak_energy = 0;
        
        const double MIN_ENERGY_THRESHOLD = 0.5; 

        for (size_t i = 1; i < m_size / 2; ++i) {
            double freq = i * freq_step;
            double magnitude = std::abs(complex_data[i]);

            if (freq >= 0.5 && freq <= 3.5) { 
                gait_energy += magnitude;
                if (magnitude > peak_energy) {
                    peak_energy = magnitude;
                }
            } else if (freq > 4.0 && freq <= 15.0) {
                tremor_energy += magnitude;
            }
        }

        if (gait_energy < MIN_ENERGY_THRESHOLD) {
            return 0.0;
        }

        double gait_stability = peak_energy / (gait_energy + 1e-6);
        
        double normalized_tremor = tremor_energy / (gait_energy + 1e-6);
        
        double risk = 0.0;
        
        if (gait_stability < 0.35) {
            risk += 0.5; 
        }
        
        if (normalized_tremor > 0.4) {
            risk += 0.4; 
        }
        
        return std::clamp(risk, 0.0, 1.0);
    }

private:
    size_t m_size;
};

#endif
