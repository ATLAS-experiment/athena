/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_HistCommon_H
#define HDF5Utils_HistCommon_H

#include "HDF5Utils/HistAxis.h"

#include "H5Cpp.h"

#include <cstdint>
#include <string>
#include <vector>

namespace H5Utils::hist::detail {

  // Error check helper
  void chkerr(herr_t code, const std::string& error);

  // Attribute write helpers
  void write_str_attr(H5::H5Object& obj,
                      const std::string& key,
                      const std::string& val);
  void write_bool_attr(H5::H5Object& obj,
                       const std::string& key,
                       bool val);
  void write_int_attr(H5::H5Object& obj,
                      const std::string& key,
                      int64_t val);
  void write_double_attr(H5::H5Object& obj,
                         const std::string& key,
                         double val);

  // Write the ref_axes group and the "axes" object-reference dataset.
  // The histogram group hist_grp must already exist.
  void write_axes(H5::Group& hist_grp, const std::vector<Axis>& axes);

  // n_bins overloads: number of data bins (excluding underflow/overflow) for
  // each variant alternative of Axis::edges_t.
  size_t n_bins(const regular_axis_t& r);                        // r.n_bins
  size_t n_bins(const std::vector<double>& edges);               // edges.size() - 1
  size_t n_bins(const std::vector<int64_t>& vals);               // vals.size()
  size_t n_bins(const std::vector<std::string>& labels);         // labels.size()
  size_t n_bins(const std::pair<int64_t,int64_t>& range);        // range.second - range.first + 1

  // Write a 1-D dataset of variable-length strings.
  void write_str_dataset(H5::Group& parent,
                         const std::string& name,
                         const std::vector<std::string>& values);

  // H5 type mapping — must be inline to avoid ODR violations when this
  // header is included in multiple translation units.
  template <typename T>
  extern const H5::DataType hdf5_t;

  template<> inline const H5::DataType hdf5_t<double>
    = H5::PredType::NATIVE_DOUBLE;
  template<> inline const H5::DataType hdf5_t<float>
    = H5::PredType::NATIVE_FLOAT;
  template<> inline const H5::DataType hdf5_t<int>
    = H5::PredType::NATIVE_INT;
  template<> inline const H5::DataType hdf5_t<long>
    = H5::PredType::NATIVE_LONG;
  template<> inline const H5::DataType hdf5_t<long long>
    = H5::PredType::NATIVE_LLONG;
  template<> inline const H5::DataType hdf5_t<unsigned long>
    = H5::PredType::NATIVE_ULONG;
  template<> inline const H5::DataType hdf5_t<unsigned long long>
    = H5::PredType::NATIVE_ULLONG;
  template<> inline const H5::DataType hdf5_t<unsigned char>
    = H5::PredType::NATIVE_UCHAR;
  template<> inline const H5::DataType hdf5_t<unsigned short>
    = H5::PredType::NATIVE_USHORT;

} //> end namespace H5Utils::hist::detail

#endif //> !HDF5Utils_HistCommon_H
