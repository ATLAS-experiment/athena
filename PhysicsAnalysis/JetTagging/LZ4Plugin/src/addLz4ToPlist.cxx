/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LZ4Plugin/addLz4ToPlist.h"

#include "H5Cpp.h"

#include <stdexcept>
#include <array>
#include <mutex>

// Forward declare the filter struct from H5Zlz4.c (C linkage)
extern "C" {
  extern const H5Z_class2_t H5Z_LZ4[1];
}

void addLz4ToPlist(H5::DSetCreatPropList& plist) {

  // Register the filter if it isn't already known to the library.
  // Since LZ4PluginLib is linked into the binary, H5Z_LZ4 is already
  // in memory -- no HDF5_PLUGIN_PATH or symlink directory needed.
  // The registry is global, so registration happens once per process.
  static std::once_flag registered;
  std::call_once(registered, []() {
    if (!H5Zfilter_avail(H5Z_FILTER_LZ4)) {
      herr_t reg = H5Zregister(H5Z_LZ4);
      if (reg < 0) throw std::runtime_error("can't register LZ4 filter");
    }
  });

  // this specifies additional arguments. For LZ4 this can have one
  // entry, the chunk size. Leaving it empty should use the default
  // chunk size.
  std::array<unsigned int, 0> cd_values;

  herr_t status = H5Pset_filter(
    plist.getId(),
    H5Z_FILTER_LZ4,
    H5Z_FLAG_MANDATORY,
    cd_values.size(),
    cd_values.data());

  if (status < 0) throw std::runtime_error("can't set LZ4 filter");

}
