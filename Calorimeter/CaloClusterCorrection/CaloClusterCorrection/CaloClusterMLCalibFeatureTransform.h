/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBFEATURETRANSFORMS_H
#define CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBFEATURETRANSFORMS_H

#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>
#include <map>
#include <functional>

namespace CaloClusterMLCalib
{
    float standard(float x, const std::vector<float> &params)
    {
        if (params.size() != 2)
            throw std::invalid_argument("Standard expects 2 parameters [mean, std_dev]");
        float mean = params[0], std_dev = params[1];
        if (std_dev == 0)
            return x - mean;
        return (x - mean) / std_dev;
    }

    float log(float x, const std::vector<float> & /*params*/)
    {
        if (x <= 0)
            return -INFINITY; // mimic numpy's log(0) behavior
        return std::log(x);
    }

    float logTen(float x, const std::vector<float> & /*params*/)
    {
        if (x <= 0)
            return -INFINITY;
        return std::log10(x);
    }

    float logStandard(float x, const std::vector<float> &params)
    {
        if (params.size() != 2)
            throw std::invalid_argument("LogStandard expects 2 parameters [mean, std_dev]");
        float log_x = log(x, {});
        return standard(log_x, params);
    }

    float logTenStandard(float x, const std::vector<float> &params)
    {
        if (params.size() != 4)
            throw std::invalid_argument("LogTenStandard expects 4 parameters [xmin, epsilon, mean, std_dev]");
        float xmin = params[0], epsilon = params[1], mean = params[2], std_dev = params[3];
        float x_shifted = x - xmin + epsilon;
        float log_x = logTen(x_shifted, {});
        return standard(log_x, {mean, std_dev});
    }

    float maxAbsolute(float x, const std::vector<float> &params)
    {
        if (params.size() != 1)
            throw std::invalid_argument("MaxAbsolute expects 1 parameter [max_value]");
        float max_val = params[0];
        if (max_val == 0)
            return x;
        return x / std::abs(max_val);
    }

    float minMax(float x, const std::vector<float> &params)
    {
        if (params.size() != 2)
            throw std::invalid_argument("MinMaxNorm expects 2 parameters [min_val, max_val]");
        float min_val = params[0], max_val = params[1];
        if (min_val == max_val)
            return 0.0; // Avoid division by zero
        return (x - min_val) / (max_val - min_val);
    }

    using TransformFunc = std::function<float(float, const std::vector<float> &)>;

    const std::map<std::string, TransformFunc> TRANSFORMATIONS = {
        {"Standard", standard},
        {"Log", log},
        {"LogTen", logTen},
        {"LogStandard", logStandard},
        {"LogTenStandard", logTenStandard},
        {"MaxAbsolute", maxAbsolute},
        {"MinMaxNorm", minMax}};
}

#endif // CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBFEATURETRANSFORMS_H
