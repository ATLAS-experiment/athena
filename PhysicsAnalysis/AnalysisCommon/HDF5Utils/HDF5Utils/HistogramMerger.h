/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_HistogramMerger_H
#define HDF5Utils_HistogramMerger_H

#include "H5Cpp.h"

#include <map>
#include <memory>
#include <string>

namespace H5Utils::hist {

  class IHistogram;  // forward declaration — full type in .cxx

  /**
   * @class HistogramMerger
   *
   * Accumulates UHI histogram groups from multiple source files and writes the
   * merged result once all inputs have been consumed.
   *
   * Usage:
   *   HistogramMerger hm;
   *   for each source file:
   *     hm.add("/path/to/hist", src_group);
   *   hm.write(output_root_group);
   */
  class HistogramMerger {
  public:
    ~HistogramMerger();

    /**
     * @brief Accumulate one UHI histogram group.
     * @param path Full HDF5 object path, e.g. "/jets/mass" (same in all files).
     * @param src  The UHI histogram group from a source file.
     */
    void add(const std::string& path, const H5::Group& src);

    /**
     * @brief Write every accumulated histogram into the output file.
     * @param root The root group of the output file (or any common ancestor).
     */
    void write(H5::Group& root) const;

  private:
    std::map<std::string, std::unique_ptr<IHistogram>> m_hists;

    /// Factory: read the storage type from @p src and return a zero-initialised
    /// concrete IHistogram of the appropriate type.
    std::unique_ptr<IHistogram> make(const H5::Group& src);
  };

} //> end namespace H5Utils::hist

#endif //> !HDF5Utils_HistogramMerger_H
