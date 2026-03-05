/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_IHistogram_H
#define HDF5Utils_IHistogram_H

#include "H5Cpp.h"
#include <string>

namespace H5Utils::hist {

  /**
   * @class IHistogram
   * Abstract interface for a mergeable histogram.
   *
   * Concrete classes accumulate bin data from HDF5 UHI groups and can write
   * the merged result back to a new HDF5 group.
   */
  class IHistogram {
  public:
    virtual ~IHistogram() = default;

    /**
     * @brief Accumulate data from a UHI histogram group in a source file.
     * @param src The UHI histogram group (must have uhi_schema attribute).
     */
    virtual void add(const H5::Group& src) = 0;

    /**
     * @brief Write the accumulated histogram as a new UHI group.
     * @param parent The parent group to create the histogram in.
     * @param name   The name for the new histogram group.
     */
    virtual void write(H5::Group& parent, const std::string& name) const = 0;
  };

} //> end namespace H5Utils::hist

#endif //> !HDF5Utils_IHistogram_H
