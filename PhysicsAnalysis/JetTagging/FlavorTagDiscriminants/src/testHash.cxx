#include "FlavorTagInference/GNNOptions.h"

#include <unordered_map>

#include <iostream>

int main(int, char*[]) {
  using namespace FlavorTagInference;
  GNNOptions opts;
  std::unordered_map<GNNOptions, std::string> optsMap;
  optsMap[opts] = "zooz";
  for (const auto& [k, v]: optsMap) {
    std::cout << k.default_output_value << " " << v << std::endl;
  }
  return 0;
}
