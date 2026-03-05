/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/HistCommon.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace H5Utils::hist::detail {

void chkerr(herr_t code, const std::string& error) {
  if (code < 0) throw std::runtime_error("error setting " + error);
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
                     const std::string& key,
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

void write_axes(H5::Group& hist_grp, const std::vector<Axis>& axes)
{
  H5::Group ref_axes_grp = hist_grp.createGroup("ref_axes");

  for (size_t ax_n = 0; ax_n < axes.size(); ++ax_n) {
    const auto& ax = axes.at(ax_n);
    H5::Group ax_grp = ref_axes_grp.createGroup("axis_" + std::to_string(ax_n));

    if (ax.n_regular_bins) {
      write_str_attr(ax_grp, "type", "regular");
      write_double_attr(ax_grp, "lower",
                        static_cast<double>(ax.edges.front()));
      write_double_attr(ax_grp, "upper",
                        static_cast<double>(ax.edges.back()));
      write_int_attr(ax_grp, "bins", *ax.n_regular_bins);
    } else {
      write_str_attr(ax_grp, "type", "variable");
      hsize_t nedges = ax.edges.size();
      H5::DataSpace edge_space(1, &nedges);
      H5::DSetCreatPropList edge_props;
      edge_props.setChunk(1, &nedges);
      edge_props.setDeflate(7);
      H5::DataSet edge_ds = ax_grp.createDataSet(
        "edges", H5::PredType::NATIVE_FLOAT, edge_space, edge_props);
      edge_ds.write(ax.edges.data(), H5::PredType::NATIVE_FLOAT);
    }

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
