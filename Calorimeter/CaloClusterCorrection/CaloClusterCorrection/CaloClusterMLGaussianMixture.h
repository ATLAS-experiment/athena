/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBGAUSSIANMIXTURE_H
#define CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBGAUSSIANMIXTURE_H

#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include <limits>

namespace CaloClusterMLCalib
{
    constexpr float epsilon = 1e-9; // Introduced to avoid FPE DIVBYZERO warning in std::log()

    // Helper function for logsumexp
    float log_sum_exp(const std::vector<float> &vec)
    {
        if (vec.empty())
        {
            return -std::numeric_limits<float>::infinity();
        }
        float max_val = *std::max_element(vec.begin(), vec.end());
        float sum = 0.0;
        for (float val : vec)
        {
            sum += std::exp(val - max_val);
        }
        return max_val + std::log(sum);
    }

    float log_likelihood(float x, const std::vector<float> &mus, const std::vector<float> &log_sigma2s, const std::vector<float> &alphas)
    {
        // Ensure vectors have the correct size
        assert(mus.size() == 3 && log_sigma2s.size() == 3 && alphas.size() == 3);

        std::vector<float> log_likelihood_components(3);

        for (int i = 0; i < 3; ++i)
        {
            // Calculate one-dimensional negative log-Gaussian
            float neg_log_gauss = std::pow(mus[i] - x, 2.0) / (2.0 * std::exp(log_sigma2s[i])) + 0.5 * log_sigma2s[i];
            neg_log_gauss += 0.5 * std::log(2.0 * M_PI); // M_PI is a common constant for pi

            // log_prob_components = -neg_log_gauss_i + log(alpha_i)
            log_likelihood_components[i] = -neg_log_gauss + std::log(alphas[i] + epsilon); // Added epsilon to avoid FPE DIVBYZERO warning
        }

        // log_prob = log(sum_i exp(log_prob_components_i))
        return log_sum_exp(log_likelihood_components);
    }

    float modes(const std::vector<float> &mus, const std::vector<float> &log_sigma2s, const std::vector<float> &alphas)
    {
        // Select range to check for maxima
        float x_min = 0.9 * (*std::min_element(mus.begin(), mus.end()));
        float x_max = 1.1 * (*std::max_element(mus.begin(), mus.end()));

        // Create array of ranges to check for maxima
        const int num_points = 1000;
        std::vector<float> x_test(num_points);
        float step = (x_max - x_min) / (num_points - 1);
        for (int i = 0; i < num_points; ++i)
        {
            x_test[i] = x_min + i * step;
        }

        float max_log_likelihood = -std::numeric_limits<float>::infinity();
        float mode = x_test[0]; // Initialize to first test point instead of 0

        // Find the x-point with the largest likelihood value
        for (float x : x_test)
        {
            float ll = log_likelihood(x, mus, log_sigma2s, alphas);
            if (ll > max_log_likelihood)
            {
                max_log_likelihood = ll;
                mode = x;
            }
        }

        return mode;
    }

    float sigma_stoch(const std::vector<float> &mus,
                      const std::vector<float> &log_sigma2s,
                      const std::vector<float> &alphas)
    {
        float sigma_stoch2 = 0.0;
        float sum_alphas_mus = 0.0;

        // This loop combines the first three lines of the Python function.
        for (size_t i = 0; i < mus.size(); ++i)
        {
            // Corresponds to: np.sum(alphas * sigma2s, axis=-1)
            sigma_stoch2 += alphas[i] * std::exp(log_sigma2s[i]);
            // Corresponds to: np.sum(alphas * np.power(mus, 2), axis=-1)
            sigma_stoch2 += alphas[i] * std::pow(mus[i], 2);
            // Accumulates sum for the final term: np.sum(alphas * mus, axis=-1)
            sum_alphas_mus += alphas[i] * mus[i];
        }

        // Corresponds to: sigma_stoch2 -= np.power(np.sum(alphas * mus, axis=-1), 2)
        sigma_stoch2 -= std::pow(sum_alphas_mus, 2);

        // Corresponds to: return np.sqrt(sigma_stoch2)
        return std::sqrt(sigma_stoch2);
    }
}

#endif // CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBGAUSSIANMIXTURE_H
