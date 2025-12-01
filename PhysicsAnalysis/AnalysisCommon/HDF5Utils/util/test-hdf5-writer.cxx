/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/Writer.h"

//-------------------------------------------------------------------------
// output data structure
struct out_t
{
  double dtype;
  float ftype;
  char ctype;
  short stype;
  int itype;
  long ltype;
  long long lltype;
  unsigned char uctype;
  unsigned short ustype;
  unsigned int uitype;
  unsigned long ultype;
  unsigned long long ulltype;
  bool btype;
};
using consumer_t = H5Utils::Consumers<const out_t&>;

consumer_t getConsumers() {
  consumer_t consumers;
#define ADD(NAME) consumers.add(#NAME, [](const out_t& o){ return o.NAME;}, 0)
  ADD(ctype);
  ADD(stype);
  ADD(itype);
  ADD(ltype);
  ADD(lltype);
  ADD(uctype);
  ADD(ustype);
  ADD(uitype);
  ADD(ultype);
  ADD(ulltype);
  ADD(ftype);
  ADD(dtype);
  ADD(btype);
#undef ADD
  auto half = H5Utils::Compression::HALF_PRECISION;
  consumers.add("half" , [](const out_t& o) { return o.ftype; }, 0, half);
  consumers.add("dhalf", [](const out_t& o) { return o.dtype; }, 0, half);
  return consumers;
}

//-------------------------------------------------------------------------
// outputs

std::vector<out_t> getOutputs(int offset, size_t length, float factor) {
  std::vector<out_t> outvec;
  for (size_t n = 0; n < length; n++) {
    out_t out;
    long long int shifted = n + offset;
    double factored = shifted*factor;
    out.dtype = factored;
    out.ftype = factored;
    out.ctype = shifted;
    out.stype = shifted;
    out.itype = shifted;
    out.ltype = shifted;
    out.lltype = shifted;
    out.uctype = shifted;
    out.ustype = shifted;
    out.uitype = shifted;
    out.ultype = shifted;
    out.ulltype = shifted;
    out.btype = n % 2;
    outvec.push_back(out);
  }
  return outvec;
}

template <size_t N>
auto nestOutputs(int offset, size_t length) {
  using ret_t = decltype(
    nestOutputs<N-1>(std::declval<int>(),std::declval<size_t>()));
  std::vector<ret_t> ret;
  for (size_t n = 0; n < length; n++) {
    ret.push_back(nestOutputs<N-1>(n + offset, length + 1));
  }
  return ret;
}
template<>
auto nestOutputs<1>(int offset, size_t length) {
  return getOutputs(offset, length, 0.5);
}

//-------------------------------------------------------------------------
// main routine

void fill(H5::Group& out_file, size_t iterations) {

  const int deflate = 7;

  // scalar output
  using scalar_writer_t = H5Utils::Writer<0, consumer_t::input_type>;
  scalar_writer_t::configuration_type scalar_config{};
  scalar_config.name = "scalar";
  scalar_config.deflate = deflate;
  consumer_t consumers = getConsumers();
  scalar_writer_t scalar(out_file, consumers, scalar_config);
  for (size_t n = 0; n < iterations; n++) {
    scalar.fill(getOutputs(1 + n, 1, 0.5).at(0));
  }

  // 1d output
  using d1_t = H5Utils::Writer<1, consumer_t::input_type>;
  d1_t::configuration_type d1_config{};
  d1_config.name = "1d";
  d1_config.extent = {10};
  d1_config.chunks = {5};
  d1_config.deflate = deflate;
  d1_t d1(out_file, consumers, d1_config);
  for (size_t n = 0; n < iterations; n++) {
    d1.fill(getOutputs(n, 10, 0.5));
  }

  // 4d output
  using d4_t = H5Utils::Writer<4, consumer_t::input_type>;
  d4_t::configuration_type d4_config;
  d4_config.name = "4d";
  d4_config.extent = {2,3,4,5};
  d4_config.chunks = {1,2,1,2};
  d4_config.deflate = deflate;
  d4_t d4(out_file, consumers, d4_config);
  for (size_t n = 0; n < iterations; n++) {
    auto vals = nestOutputs<4>(n, 2);
    // store some specific value to make sure we have the indexing
    // right
    vals.at(1).at(2).at(3).at(4).stype = 86;
    d4.fill(std::move(vals));
  }
}
//coverity[root_function]
int main(int nargs, char* argv[]) {
  H5::H5File out_file("output.h5", H5F_ACC_TRUNC);
  size_t iterations = 1;
  if (nargs > 2) {
    return 1;
  }
  if (nargs > 1) iterations = std::atoi(argv[1]);
  //coverity[TAINTED_SCALAR]
  fill(out_file, iterations);
  return 0;
}
