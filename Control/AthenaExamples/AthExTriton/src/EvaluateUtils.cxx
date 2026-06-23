// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include "EvaluateUtils.h"

#include <arpa/inet.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <numeric>

namespace EvaluateUtils {
//*******************************************************************
// for reading MNIST images
std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(
    const std::string& full_path) {
  std::ifstream file(full_path.c_str(), std::ios::binary);
  std::int32_t magic_number = 0;
  std::int32_t number_of_images = 0;
  std::int32_t n_rows = 0;
  std::int32_t n_cols = 0;

  file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
  // This file format stores numbers in big-endian (network) ordering
  magic_number = ntohl(magic_number);

  // The "magic number" encodes the data type and number of dimensions.
  // This therefore has to be 0x0803 to be a set of images with unsigned byte
  // pixels
  const std::int32_t required_magic = 0x0803;
  if (magic_number != required_magic) {
    throw std::runtime_error(
        std::format("The MNIST input file at {} has the magic number {:#06x}! "
                    "Expected {:#06x}.",
                    full_path, magic_number, required_magic));
  }

  // The next three fields are the number of images, and the number of rows and
  // columns per image
  file.read(reinterpret_cast<char*>(&number_of_images),
            sizeof(number_of_images));
  number_of_images = ntohl(number_of_images);
  file.read(reinterpret_cast<char*>(&n_rows), sizeof(n_rows));
  n_rows = ntohl(n_rows);
  if (n_rows < 1) {
    throw std::runtime_error(
        std::format("The MNIST input file at {} has images with {} rows",
                    full_path, n_rows));
  }
  file.read(reinterpret_cast<char*>(&n_cols), sizeof(n_cols));
  n_cols = ntohl(n_cols);
  if (n_cols < 1) {
    throw std::runtime_error(
        std::format("The MNIST input file at {} has images with {} columns",
                    full_path, n_cols));
  }

  // Now we can make the vector
  std::vector<std::vector<std::vector<float>>> input_tensor_values;
  input_tensor_values.resize(
      number_of_images,
      std::vector<std::vector<float>>(n_rows, std::vector<float>(n_cols)));
  for (int i = 0; i < number_of_images; ++i) {
    for (int r = 0; r < n_rows; ++r) {
      for (int c = 0; c < n_cols; ++c) {
        std::uint8_t temp = 0;
        file.read((char*)&temp, sizeof(temp));
        input_tensor_values[i][r][c] =
            float(temp) / std::numeric_limits<std::uint8_t>::max();
      }
    }
  }
  return input_tensor_values;
}

std::vector<float> flattenNestedVectors(
    const std::vector<std::vector<float>>& nestedVector) {
  const std::size_t total_size = std::transform_reduce(
      nestedVector.cbegin(), nestedVector.cend(), 0, std::plus<>{},
      [](const std::vector<float>& c) { return c.size(); });

  std::vector<float> result{};
  result.reserve(total_size);

  for (const auto& v : nestedVector) {
    std::ranges::copy(v, std::back_inserter(result));
  }
  return result;
}
}  // namespace EvaluateUtils
