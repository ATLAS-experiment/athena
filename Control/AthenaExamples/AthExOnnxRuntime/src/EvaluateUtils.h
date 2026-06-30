// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef EvaluateUtils_H
#define EvaluateUtils_H

#include <format>
#include <vector>
#include <string>

namespace EvaluateUtils {
   //*******************************************************************
   // for reading MNIST images
   std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(const std::string &full_path);
   inline auto spanToString = [](std::span<const float> s) {
      std::string out;
      for (float v : s) out += std::format("{:.2e} ", v);
      if (!out.empty()) out.pop_back();
      return out;
    };
}

#endif // EvaluateUtils_H