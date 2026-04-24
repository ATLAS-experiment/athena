/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/HistogramMerger.h"
#include "HDF5Utils/IHistogram.h"
#include "HDF5Utils/HistCommon.h"

#include "H5Cpp.h"

#include <cstdint>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// ============================================================
// File-local helpers
// ============================================================

namespace {

using namespace H5Utils::hist::detail;

// ------------------------------------------------------------------
// Attribute read helpers
// ------------------------------------------------------------------
std::string read_str_attr(const H5::H5Object& obj,
                          const std::string& key)
{
  H5::Attribute attr = obj.openAttribute(key);
  std::string val;
  attr.read(attr.getDataType(), val);
  return val;
}

bool read_bool_attr(const H5::H5Object& obj, const std::string& key) {
  H5::Attribute attr = obj.openAttribute(key);
  uint8_t val = 0;
  attr.read(H5::PredType::NATIVE_UINT8, &val);
  return val != 0;
}

double read_double_attr(const H5::H5Object& obj, const std::string& key) {
  H5::Attribute attr = obj.openAttribute(key);
  double val = 0.0;
  attr.read(H5::PredType::NATIVE_DOUBLE, &val);
  return val;
}

int64_t read_int_attr(const H5::H5Object& obj, const std::string& key) {
  H5::Attribute attr = obj.openAttribute(key);
  int64_t val = 0;
  attr.read(H5::PredType::NATIVE_INT64, &val);
  return val;
}

// ------------------------------------------------------------------
// VL-string category reader (RAII-safe)
// ------------------------------------------------------------------
std::vector<std::string> read_string_categories(const H5::DataSet& ds)
{
  H5::StrType   strtype(H5::PredType::C_S1, H5T_VARIABLE);
  H5::DataSpace sp = ds.getSpace();
  hsize_t npts = static_cast<hsize_t>(sp.getSimpleExtentNpoints());
  std::vector<char*> ptrs(npts, nullptr);
  // RAII guard must be created before ds.read so that H5Treclaim is
  // called even if ds.read throws after partial allocation.
  // H5Treclaim on nullptr entries is a no-op, so early construction
  // is safe.
  struct VlGuard {
    hid_t tid, sid;
    void* buf;
    ~VlGuard() { H5Treclaim(tid, sid, H5P_DEFAULT, buf); }
  } guard{strtype.getId(), sp.getId(), ptrs.data()};
  ds.read(ptrs.data(), strtype);
  std::vector<std::string> labels;
  labels.reserve(npts);
  for (char* p : ptrs) {
    labels.emplace_back(p ? p : "");
  }
  return labels;
}

// ------------------------------------------------------------------
// Axis I/O
// ------------------------------------------------------------------
std::vector<Axis> read_axes(const H5::Group& grp) {
  H5::Group ref_axes = grp.openGroup("ref_axes");
  std::vector<Axis> axes;
  for (int n = 0; ; ++n) {
    std::string ax_name = "axis_" + std::to_string(n);
    htri_t exists = H5Lexists(ref_axes.getId(), ax_name.c_str(), H5P_DEFAULT);
    if (exists <= 0) break;

    H5::Group ax_grp = ref_axes.openGroup(ax_name);
    Axis ax;

    std::string type = read_str_attr(ax_grp, "type");
    if (type == "regular") {
      double  lower = read_double_attr(ax_grp, "lower");
      double  upper = read_double_attr(ax_grp, "upper");
      int64_t bins  = read_int_attr   (ax_grp, "bins");
      ax.edges = regular_axis_t{lower, upper, static_cast<size_t>(bins)};
    } else if (type == "variable") {
      H5::DataSet edge_ds = ax_grp.openDataSet("edges");
      hsize_t npts = edge_ds.getSpace().getSimpleExtentNpoints();
      std::vector<double> edges(npts);
      edge_ds.read(edges.data(), H5::PredType::NATIVE_DOUBLE);
      ax.edges = std::move(edges);
    } else if (type == "integer") {
      int64_t start = read_int_attr(ax_grp, "start");
      int64_t stop  = read_int_attr(ax_grp, "stop");
      ax.edges = std::pair<int64_t,int64_t>{start, stop};
    } else if (type == "category") {
      H5::DataSet cat_ds = ax_grp.openDataSet("categories");
      H5::DataType dtype = cat_ds.getDataType();
      hsize_t npts = cat_ds.getSpace().getSimpleExtentNpoints();
      if (dtype.getClass() == H5T_STRING) {
        ax.edges = read_string_categories(cat_ds);
      } else {
        std::vector<int64_t> int_vals(npts);
        cat_ds.read(int_vals.data(), H5::PredType::NATIVE_INT64);
        ax.edges = std::move(int_vals);
      }
    }

    ax.underflow = read_bool_attr(ax_grp, "underflow");
    ax.overflow  = read_bool_attr(ax_grp, "overflow");

    // Read optional axis name from metadata subgroup
    htri_t has_meta = H5Lexists(ax_grp.getId(), "metadata", H5P_DEFAULT);
    if (has_meta > 0) {
      H5::Group meta = ax_grp.openGroup("metadata");
      if (meta.attrExists("metadata")) {
        ax.name = read_str_attr(meta, "metadata");
      }
    }

    axes.push_back(std::move(ax));
  }
  return axes;
}

// ------------------------------------------------------------------
// Storage dimension helpers
// ------------------------------------------------------------------

// Read the shape of the values dataset (authoritative dimensions).
std::vector<hsize_t> read_dims(const H5::Group& grp) {
  H5::DataSet ds = grp.openGroup("storage").openDataSet("values");
  H5::DataSpace sp = ds.getSpace();
  int ndims = sp.getSimpleExtentNdims();
  std::vector<hsize_t> dims(static_cast<size_t>(ndims));
  sp.getSimpleExtentDims(dims.data());
  return dims;
}

size_t flat_size(const std::vector<hsize_t>& dims) {
  return std::accumulate(dims.begin(), dims.end(),
                         hsize_t(1), std::multiplies<hsize_t>{});
}

// ------------------------------------------------------------------
// Dataset read/write helpers (templated)
// ------------------------------------------------------------------

// Returns {data, on-disk HDF5 type}.  The dtype is an independent copy
// (HDF5 calls H5Tcopy internally in getDataType()) so the caller owns it.
template <typename T>
std::pair<std::vector<T>, H5::DataType>
read_dataset(const H5::Group& grp, const std::string& name)
{
  H5::DataSet ds = grp.openDataSet(name);
  H5::DataType dtype = ds.getDataType();
  hsize_t n = ds.getSpace().getSimpleExtentNpoints();
  std::vector<T> data(n);
  ds.read(data.data(), hdf5_t<T>);
  return {std::move(data), dtype};
}

// Returns a checker lambda that validates p.second == expected and returns p.first.
auto get_type_checker(const H5::DataType& expected) {
  return [expected](auto p) {
    if (p.second != expected)
      throw std::runtime_error(
        "histogram datasets have inconsistent HDF5 types");
    return std::move(p.first);
  };
}

// Writes data with memory type T but on-disk type file_dtype.
// HDF5 performs the conversion automatically (e.g. double→float).
template <typename T>
void write_dataset(H5::Group& grp,
                           const std::string& name,
                           const std::vector<T>& data,
                           const std::vector<hsize_t>& dims,
                           const H5::DataType& file_dtype)
{
  H5::DataSpace space(static_cast<int>(dims.size()), dims.data());
  H5::DSetCreatPropList props;
  props.setChunk(static_cast<int>(dims.size()), dims.data());
  props.setDeflate(7);
  chkerr(H5Pset_dset_no_attrs_hint(props.getId(), true), "no attribute hint");
  if (space.getSelectNpoints() != static_cast<hssize_t>(data.size()))
    throw std::runtime_error(
      "write_dataset: dataspace size does not match data size");
  grp.createDataSet(name, file_dtype, space, props)
     .write(data.data(), hdf5_t<T>);
}

// ------------------------------------------------------------------
// Axes consistency check
// ------------------------------------------------------------------
void check_axes_consistent(const std::vector<Axis>& stored,
                                   const std::vector<Axis>& incoming)
{
  if (stored != incoming)
    throw std::runtime_error("histogram axes are inconsistent between input files");
}

// ------------------------------------------------------------------
// Group navigation/creation helper
// ------------------------------------------------------------------
H5::Group open_or_create_group(H5::Group& parent, const std::string& name) {
  htri_t exists = H5Lexists(parent.getId(), name.c_str(), H5P_DEFAULT);
  if (exists > 0) return parent.openGroup(name);
  return parent.createGroup(name);
}

// ==================================================================
// Concrete histogram implementations
// ==================================================================

// ------------------------------------------------------------------
// SimpleHistogram<T>: "double" and "int" storage types
// ------------------------------------------------------------------
template <typename T>
class SimpleHistogram : public H5Utils::hist::IHistogram {
public:
  SimpleHistogram(std::vector<Axis> axes,
                  std::vector<hsize_t> dims)
    : m_axes(std::move(axes))
    , m_dims(std::move(dims))
    , m_values(flat_size(m_dims), T(0))
  {}

  void add(const H5::Group& src) override {
    check_axes_consistent(m_axes, read_axes(src));
    H5::Group storage = src.openGroup("storage");
    auto [v, dtype] = read_dataset<T>(storage, "values");
    m_storage_dtype = dtype;
    if (v.size() != m_values.size())
      throw std::runtime_error(
        "histogram bin count in input file does not match expected size");
    for (size_t i = 0; i < m_values.size(); ++i) m_values[i] += v[i];
  }

  void write(H5::Group& parent, const std::string& name) const override {
    H5::Group grp = parent.createGroup(name);
    write_int_attr(grp, "uhi_schema", 1);
    grp.createGroup("metadata");
    grp.createGroup("writer_info");
    write_axes(grp, m_axes);

    H5::Group storage = grp.createGroup("storage");
    write_str_attr(storage, "type", std::is_integral_v<T> ? "int" : "double");
    write_dataset<T>(storage, "values", m_values, m_dims, m_storage_dtype);
  }

private:
  std::vector<Axis>    m_axes;
  std::vector<hsize_t> m_dims;
  std::vector<T>       m_values;
  H5::DataType         m_storage_dtype;  // captured from "values" in add()
};

// ------------------------------------------------------------------
// WeightedHistogram: "weighted" storage type
// ------------------------------------------------------------------
class WeightedHistogram : public H5Utils::hist::IHistogram {
public:
  WeightedHistogram(std::vector<Axis> axes, std::vector<hsize_t> dims)
    : m_axes(std::move(axes))
    , m_dims(std::move(dims))
    , m_values(flat_size(m_dims), 0.0)
    , m_variances(flat_size(m_dims), 0.0)
  {}

  void add(const H5::Group& src) override {
    check_axes_consistent(m_axes, read_axes(src));
    H5::Group storage = src.openGroup("storage");
    auto [v, dtype] = read_dataset<double>(storage, "values");
    m_storage_dtype = dtype;
    auto check = get_type_checker(dtype);
    std::vector<double> w = check(read_dataset<double>(storage, "variances"));
    if (v.size() != m_values.size() || w.size() != m_variances.size())
      throw std::runtime_error(
        "histogram bin count in input file does not match expected size");
    for (size_t i = 0; i < m_values.size(); ++i) {
      m_values[i]    += v[i];
      m_variances[i] += w[i];
    }
  }

  void write(H5::Group& parent, const std::string& name) const override {
    H5::Group grp = parent.createGroup(name);
    write_int_attr(grp, "uhi_schema", 1);
    grp.createGroup("metadata");
    grp.createGroup("writer_info");
    write_axes(grp, m_axes);

    H5::Group storage = grp.createGroup("storage");
    write_str_attr(storage, "type", "weighted");
    write_dataset<double>(storage, "values",    m_values,    m_dims, m_storage_dtype);
    write_dataset<double>(storage, "variances", m_variances, m_dims, m_storage_dtype);
  }

private:
  std::vector<Axis>    m_axes;
  std::vector<hsize_t> m_dims;
  std::vector<double>  m_values;
  std::vector<double>  m_variances;
  H5::DataType         m_storage_dtype;  // captured from "values" in add()
};

// ------------------------------------------------------------------
// MeanHistogram: "mean" storage type
//
// On disk: values = mean, variances = variance, count = n
// Internal: m_sums = sum(count * mean), m_M2 = sum(count * variance) accumulated
//           with Chan's parallel formula for numerically stable variance.
// ------------------------------------------------------------------
class MeanHistogram : public H5Utils::hist::IHistogram {
public:
  MeanHistogram(std::vector<Axis> axes, std::vector<hsize_t> dims)
    : m_axes(std::move(axes))
    , m_dims(std::move(dims))
    , m_n(flat_size(m_dims), 0.0)
    , m_sums(flat_size(m_dims), 0.0)
    , m_M2(flat_size(m_dims), 0.0)
  {}

  void add(const H5::Group& src) override {
    check_axes_consistent(m_axes, read_axes(src));
    H5::Group storage = src.openGroup("storage");
    auto [means, dtype] = read_dataset<double>(storage, "values");
    m_storage_dtype = dtype;
    auto check = get_type_checker(dtype);
    std::vector<double> counts = check(read_dataset<double>(storage, "count"));
    // variances may not be present; default to zero
    std::vector<double> variances(m_n.size(), 0.0);
    htri_t has_var = H5Lexists(storage.getId(), "variances", H5P_DEFAULT);
    if (has_var > 0) {
      variances = check(read_dataset<double>(storage, "variances"));
    }

    if (counts.size() != m_n.size() || means.size() != m_n.size())
      throw std::runtime_error(
        "histogram bin count in input file does not match expected size");

    for (size_t i = 0; i < m_n.size(); ++i) {
      double na  = m_n[i];
      double nb  = counts[i];
      double nc  = na + nb;

      if (nc == 0.0) continue;

      double mean_b = means[i];
      double mean_a = (na > 0.0) ? m_sums[i] / na : 0.0;
      double delta  = mean_b - mean_a;

      // Chan's parallel formula for sum of squared deviations.
      // boost::histogram mean accumulator stores Bessel-corrected (sample)
      // variance: var = M2 / (n-1), so M2 = var * (n-1).
      // For n <= 1 the variance is defined as 0, so M2 = 0.
      double M2_b = (nb > 1.0) ? variances[i] * (nb - 1.0) : 0.0;
      m_M2[i]  += M2_b + delta * delta * na * nb / nc;
      m_sums[i] = na * mean_a + nb * mean_b;  // = nc * new_mean
      m_n[i]    = nc;
    }
  }

  void write(H5::Group& parent, const std::string& name) const override {
    H5::Group grp = parent.createGroup(name);
    write_int_attr(grp, "uhi_schema", 1);
    grp.createGroup("metadata");
    grp.createGroup("writer_info");
    write_axes(grp, m_axes);

    H5::Group storage = grp.createGroup("storage");
    write_str_attr(storage, "type", "mean");

    size_t n = m_n.size();
    std::vector<double> values(n);
    std::vector<double> variances(n);
    for (size_t i = 0; i < n; ++i) {
      if (m_n[i] > 0.0) {
        values[i] = m_sums[i] / m_n[i];
        // Write Bessel-corrected (sample) variance to match the input format.
        variances[i] = (m_n[i] > 1.0) ? m_M2[i] / (m_n[i] - 1.0) : 0.0;
      }
    }

    write_dataset<double>(storage, "count",     m_n,       m_dims, m_storage_dtype);
    write_dataset<double>(storage, "values",    values,    m_dims, m_storage_dtype);
    write_dataset<double>(storage, "variances", variances, m_dims, m_storage_dtype);
  }

private:
  std::vector<Axis>    m_axes;
  std::vector<hsize_t> m_dims;
  std::vector<double>  m_n;     // accumulated count per bin
  std::vector<double>  m_sums;  // accumulated sum(count * mean) per bin
  std::vector<double>  m_M2;    // accumulated sum-of-squared-deviations per bin
  H5::DataType         m_storage_dtype;  // captured from "values" in add()
};

// ------------------------------------------------------------------
// WeightedMeanHistogram: "weighted_mean" storage type
//
// On disk: values = weighted mean = sum(w*x)/sum(w),
//          sum_of_weights, sum_of_weights_squared, count
// Internal: accumulate each sum directly; compute values on write.
// ------------------------------------------------------------------
class WeightedMeanHistogram : public H5Utils::hist::IHistogram {
public:
  WeightedMeanHistogram(std::vector<Axis> axes, std::vector<hsize_t> dims)
    : m_axes(std::move(axes))
    , m_dims(std::move(dims))
    , m_sum_w(flat_size(m_dims), 0.0)
    , m_sum_w2(flat_size(m_dims), 0.0)
    , m_counts(flat_size(m_dims), 0.0)
    , m_weighted_sums(flat_size(m_dims), 0.0)
  {}

  void add(const H5::Group& src) override {
    check_axes_consistent(m_axes, read_axes(src));
    H5::Group storage = src.openGroup("storage");
    auto [values, dtype] = read_dataset<double>(storage, "values");
    m_storage_dtype = dtype;
    auto check = get_type_checker(dtype);
    std::vector<double> sum_w  = check(
      read_dataset<double>(storage, "sum_of_weights"));
    std::vector<double> sum_w2 = check(
      read_dataset<double>(storage, "sum_of_weights_squared"));

    if (values.size() != m_sum_w.size())
      throw std::runtime_error(
        "histogram bin count in input file does not match expected size");

    // count is optional
    std::vector<double> counts(m_sum_w.size(), 0.0);
    htri_t has_count = H5Lexists(storage.getId(), "count", H5P_DEFAULT);
    if (has_count > 0) {
      counts = check(read_dataset<double>(storage, "count"));
    }

    for (size_t i = 0; i < m_sum_w.size(); ++i) {
      m_weighted_sums[i] += sum_w[i] * values[i];
      m_sum_w[i]         += sum_w[i];
      m_sum_w2[i]        += sum_w2[i];
      m_counts[i]        += counts[i];
    }
  }

  void write(H5::Group& parent, const std::string& name) const override {
    H5::Group grp = parent.createGroup(name);
    write_int_attr(grp, "uhi_schema", 1);
    grp.createGroup("metadata");
    grp.createGroup("writer_info");
    write_axes(grp, m_axes);

    H5::Group storage = grp.createGroup("storage");
    write_str_attr(storage, "type", "weighted_mean");

    size_t n = m_sum_w.size();
    std::vector<double> values(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
      if (m_sum_w[i] != 0.0) {
        values[i] = m_weighted_sums[i] / m_sum_w[i];
      }
    }

    write_dataset<double>(storage, "values",                 values,   m_dims, m_storage_dtype);
    write_dataset<double>(storage, "sum_of_weights",         m_sum_w,  m_dims, m_storage_dtype);
    write_dataset<double>(storage, "sum_of_weights_squared", m_sum_w2, m_dims, m_storage_dtype);
    write_dataset<double>(storage, "count",                  m_counts, m_dims, m_storage_dtype);
  }

private:
  std::vector<Axis>    m_axes;
  std::vector<hsize_t> m_dims;
  std::vector<double>  m_sum_w;          // sum of weights
  std::vector<double>  m_sum_w2;         // sum of weights squared
  std::vector<double>  m_counts;         // unweighted fill count
  std::vector<double>  m_weighted_sums;  // sum(weight * value)
  H5::DataType         m_storage_dtype;  // captured from "values" in add()
};

} // anonymous namespace


// ==================================================================
// HistogramMerger implementation
// ==================================================================

namespace H5Utils::hist {

HistogramMerger::~HistogramMerger() = default;

std::unique_ptr<IHistogram>
HistogramMerger::make(const H5::Group& src) {
  std::vector<Axis>    axes = read_axes(src);
  std::vector<hsize_t> dims = read_dims(src);

  H5::Group storage = src.openGroup("storage");
  std::string type = read_str_attr(storage, "type");

  if (type == "double") {
    return std::make_unique<SimpleHistogram<double>>(
      std::move(axes), std::move(dims));
  }
  if (type == "int") {
    return std::make_unique<SimpleHistogram<int64_t>>(
      std::move(axes), std::move(dims));
  }
  if (type == "weighted") {
    return std::make_unique<WeightedHistogram>(
      std::move(axes), std::move(dims));
  }
  if (type == "mean") {
    return std::make_unique<MeanHistogram>(
      std::move(axes), std::move(dims));
  }
  if (type == "weighted_mean") {
    return std::make_unique<WeightedMeanHistogram>(
      std::move(axes), std::move(dims));
  }
  throw std::runtime_error("HistogramMerger: unknown UHI storage type: " + type);
}

void HistogramMerger::add(const std::string& path, const H5::Group& src) {
  if (!m_hists.count(path)) {
    m_hists.emplace(path, make(src));
  }
  m_hists.at(path)->add(src);
}

void HistogramMerger::write(H5::Group& root) const {
  for (const auto& [path, hist] : m_hists) {
    // Split the absolute HDF5 path (e.g. "/a/b/c") into components.
    std::vector<std::string> parts;
    std::istringstream ss(path);
    std::string token;
    while (std::getline(ss, token, '/')) {
      if (!token.empty()) parts.push_back(token);
    }
    if (parts.empty()) continue;

    // Navigate/create intermediate groups, then write the histogram.
    H5::Group parent = root;
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
      parent = open_or_create_group(parent, parts[i]);
    }
    hist->write(parent, parts.back());
  }
}

} //> end namespace H5Utils::hist
