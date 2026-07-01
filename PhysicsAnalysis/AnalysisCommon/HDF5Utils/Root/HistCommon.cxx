/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/HistCommon.h"

#include <stdexcept>
#include <variant>
#include <format>

namespace H5Utils::hist::detail {

void chkerr(herr_t code, std::string_view error) {
  if (code < 0) throw std::runtime_error(std::format("error setting {}",error));
}

void write_str_attr(H5::H5Object& obj,
                    const std::string& key,
                    const std::string& val)
{
  H5::StrType strtype(H5::PredType::C_S1, H5T_VARIABLE);
  H5::DataSpace scalar(H5S_SCALAR);
  H5::Attribute attr = obj.createAttribute(key, strtype, scalar);
  const char* cstr = val.c_str();
  attr.write(strtype, &cstr);
}

void write_bool_attr(H5::H5Object& obj,
                     const char * key,
                     bool val)
{
  H5::DataSpace scalar(H5S_SCALAR);
  uint8_t v = val ? 1 : 0;
  H5::Attribute attr = obj.createAttribute(
    key, H5::PredType::NATIVE_UINT8, scalar);
  attr.write(H5::PredType::NATIVE_UINT8, &v);
}

void write_int_attr(H5::H5Object& obj,
                    const std::string& key,
                    int64_t val)
{
  H5::DataSpace scalar(H5S_SCALAR);
  H5::Attribute attr = obj.createAttribute(
    key, H5::PredType::NATIVE_INT64, scalar);
  attr.write(H5::PredType::NATIVE_INT64, &val);
}

void write_double_attr(H5::H5Object& obj,
                       const std::string& key,
                       double val)
{
  H5::DataSpace scalar(H5S_SCALAR);
  H5::Attribute attr = obj.createAttribute(
    key, H5::PredType::NATIVE_DOUBLE, scalar);
  attr.write(H5::PredType::NATIVE_DOUBLE, &val);
}

size_t n_bins(const regular_axis_t& r)                { return r.n_bins; }
size_t n_bins(const std::vector<double>& e)           { return e.size() - 1; }
size_t n_bins(const std::vector<int64_t>& e)          { return e.size(); }
size_t n_bins(const std::vector<std::string>& e)      { return e.size(); }
size_t n_bins(const std::pair<int64_t,int64_t>& r)    { return static_cast<size_t>(r.second - r.first + 1); }

void write_str_dataset(H5::Group& parent,
                       const std::string& name,
                       const std::vector<std::string>& values)
{
  H5::StrType strtype(H5::PredType::C_S1, H5T_VARIABLE);
  hsize_t n = values.size();
  H5::DataSpace space(1, &n);
  H5::DSetCreatPropList props;
  props.setChunk(1, &n);
  props.setDeflate(7);
  std::vector<const char*> cstrs;
  cstrs.reserve(n);
  for (const auto& s : values) cstrs.push_back(s.c_str());
  parent.createDataSet(name, strtype, space, props)
        .write(cstrs.data(), strtype);
}

// Regular float axis — r.lower/r.upper are double, matching write_double_attr.
void write_axis_edges(H5::Group& ax_grp, const regular_axis_t& r)
{
  write_str_attr   (ax_grp, "type",  "regular");
  write_double_attr(ax_grp, "lower", r.lower);
  write_double_attr(ax_grp, "upper", r.upper);
  write_int_attr   (ax_grp, "bins",  r.n_bins);
}

// Variable float axis — stored as explicit edge dataset (float64).
void write_axis_edges(H5::Group& ax_grp, const std::vector<double>& edges)
{
  write_str_attr(ax_grp, "type", "variable");
  hsize_t nedges = edges.size();
  H5::DataSpace edge_space(1, &nedges);
  H5::DSetCreatPropList props;
  props.setChunk(1, &nedges);
  props.setDeflate(7);
  ax_grp.createDataSet("edges",
         H5::PredType::NATIVE_DOUBLE, edge_space, props)
        .write(edges.data(), H5::PredType::NATIVE_DOUBLE);
}

void write_axis_edges(H5::Group& ax_grp, const std::pair<int64_t,int64_t>& range)
{
  write_str_attr(ax_grp, "type", "integer");
  write_int_attr(ax_grp, "start", range.first);
  write_int_attr(ax_grp, "stop",  range.second);
}

void write_axis_edges(H5::Group& ax_grp, const std::vector<int64_t>& vals)
{
  write_str_attr(ax_grp, "type", "category");
  hsize_t n = vals.size();
  H5::DataSpace space(1, &n);
  H5::DSetCreatPropList props;
  props.setChunk(1, &n);
  props.setDeflate(7);
  ax_grp.createDataSet("categories",
         H5::PredType::NATIVE_INT64, space, props)
        .write(vals.data(), H5::PredType::NATIVE_INT64);
}

void write_axis_edges(H5::Group& ax_grp, const std::vector<std::string>& labels)
{
  write_str_attr(ax_grp, "type", "category");
  write_str_dataset(ax_grp, "categories", labels);
}

void write_axes(H5::Group& hist_grp, const std::vector<Axis>& axes)
{
  H5::Group ref_axes_grp = hist_grp.createGroup("ref_axes");
  for (size_t ax_n = 0; ax_n < axes.size(); ++ax_n) {
    const auto& ax = axes.at(ax_n);
    H5::Group ax_grp = ref_axes_grp.createGroup("axis_" + std::to_string(ax_n));

    std::visit([&ax_grp](const auto& e) {
      write_axis_edges(ax_grp, e);
    }, ax.edges);

    write_bool_attr(ax_grp, "underflow", ax.underflow);
    write_bool_attr(ax_grp, "overflow",  ax.overflow);
    write_bool_attr(ax_grp, "circular",  false);

    H5::Group ax_meta = ax_grp.createGroup("metadata");
    if (!ax.name.empty()) {
      write_str_attr(ax_meta, "metadata", ax.name);
    }
    ax_grp.createGroup("writer_info");
  }

  // Write object-reference dataset "axes"
  std::vector<hobj_ref_t> axis_refs(axes.size());
  for (size_t ax_n = 0; ax_n < axes.size(); ++ax_n) {
    std::string ax_path = "ref_axes/axis_" + std::to_string(ax_n);
    chkerr(H5Rcreate(&axis_refs.at(ax_n), hist_grp.getId(),
                     ax_path.c_str(), H5R_OBJECT, -1),
                     "axis object reference");
  }
  hsize_t ref_dim = axes.size();
  H5::DataSpace ref_space(1, &ref_dim);
  hist_grp.createDataSet("axes", H5::PredType::STD_REF_OBJ, ref_space)
          .write(axis_refs.data(), H5::PredType::STD_REF_OBJ);
}

} //> end namespace H5Utils::hist::detail
