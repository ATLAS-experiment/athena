/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/Writer.h"

#include <random>

//-------------------------------------------------------------------------
// output data structure
struct out_t
{
  double dtype;
  float ftype;
  bool btype;
};
using consumer_t = H5Utils::Consumers<const out_t&>;

#define ADD(NAME) consumers.add(#NAME, [](const out_t& o){ return o.NAME;}, 0)
#define HADD(NAME) consumers.add(#NAME, [](const out_t& o){ return o.NAME;}, 0, h)


consumer_t getFullConsumers() {
  consumer_t consumers;
  ADD(ftype);
  ADD(dtype);
  ADD(btype);
  return consumers;
}
consumer_t getHalfConsumers() {
  consumer_t consumers;
  auto h = H5Utils::Compression::HALF_PRECISION;
  HADD(ftype);
  HADD(dtype);
  ADD(btype);
  return consumers;
}

//-------------------------------------------------------------------------
// outputs

using mt_t = decltype(std::mt19937());

std::vector<out_t> getOutputs(size_t length, mt_t& rand ) {
  std::vector<out_t> outvec;
  std::uniform_int_distribution<int> exponent(-10, 2);
  std::uniform_real_distribution<float> man(-0.0001, 0.0001);
  std::uniform_int_distribution<short> booldist(0, 1);
  for (size_t n = 0; n < length; n++) {
    out_t out;
    double value = std::exp2(exponent(rand)) * (1 + man(rand));
    out.dtype = value;
    out.ftype = value;
    out.btype = booldist(rand);
    outvec.push_back(out);
  }
  return outvec;
}


//-------------------------------------------------------------------------
// main routine

void fill(H5::Group& out_file, size_t iterations) {

  const int deflate = 7;
  const int max_width = 10;
  std::mt19937 random(42);

  // compare outputs
  using writer_t = H5Utils::Writer<1, consumer_t::input_type>;
  writer_t::configuration_type config;
  config.name = "full";
  config.extent = {max_width};
  config.deflate = deflate;
  writer_t full(out_file, getFullConsumers(), config);
  config.name = "half";
  writer_t half(out_file, getHalfConsumers(), config);
  std::uniform_int_distribution<int> width(0, max_width+1);
  for (size_t n = 0; n < iterations; n++) {
    auto out = getOutputs(width(random), random);
    full.fill(out);
    half.fill(out);
  }
}

int main(int nargs, char* argv[]) {
  H5::H5File out_file("output.h5", H5F_ACC_TRUNC);
  size_t iterations = 1;
  if (nargs > 2) {
    return 1;
  }
  if (nargs > 1) iterations = std::atoi(argv[1]);
  fill(out_file, iterations);
  return 0;
}
