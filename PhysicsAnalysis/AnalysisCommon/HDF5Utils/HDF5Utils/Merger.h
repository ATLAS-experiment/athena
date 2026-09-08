/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_Merger_H
#define HDF5Utils_Merger_H

#include "H5Cpp.h"
namespace H5Utils::hist { class HistogramMerger; }

#include <memory>

/**
 * @file Merger.h
 * @author Jon Burr
 *
 * The default H5 merging implementation
 */

namespace H5Utils {
  /**
   * @class H5 Merger
   */
  class Merger {
    public:
      /**
       * @brief Create the merger
       * @param mergeAxis The axis to merge along
       * @param chunkSize The chunk size to apply. If negative then the value
       * found in the input datasets will be used.
       * @param requireSameFormat Require all input files to have the same
       * groups and datasets.
       * @param bufferSize The maximum size of the buffer to use while merging
       * datasets
       * @param bufferInRows Whether the buffer size is specified in rows or
       * bytes
       */
      Merger(
          hsize_t mergeAxis = 0,
          int chunkSize = -1,
          bool requireSameFormat = true,
          std::size_t bufferSize = -1,
          bool bufferInRows = false);

      ~Merger();

      /**
       * @brief Merge a source group into a target group
       * @param target The group to merge into
       * @param source The group to merge from
       */
      void merge(H5::Group& target, const H5::Group& source);

      /**
       * @brief Merge a source dataset into a target dataset
       * @param target The dataset to merge into
       * @param source The dataset to merge from
       */
      void merge(H5::DataSet& target, const H5::DataSet& source);

      /**
       * @brief Merge a source file/group into a target file/group
       * @param target The file or group to merge into
       * @param source The file or group to merge from
       *
       * Accepts any combination of H5::File and H5::Group. Each argument is
       * converted to the H5::Group to actually operate on via convert() (a
       * no-op for a Group, the root group for a File) before forwarding to
       * the merge(Group&, const Group&) overload above.
       */
      void merge(auto& target, const auto& source)
      {
        H5::Group targetGroup = convert(target);
        H5::Group sourceGroup = convert(source);
        merge(targetGroup, sourceGroup);
      }

      /**
       * @brief Write all accumulated histogram data to the output.
       * @param dst The root group of the output file.
       */
      void flush(H5::Group& dst);

    private:
      /// convert() is a no-op for a Group, used by the merge() template
      static H5::Group convert(const H5::Group& group);
      /// convert() opens the root group of a File, used by the merge()
      /// template
      static H5::Group convert(const H5::H5File& file);

    protected:
      /// The axis to merge along
      hsize_t m_mergeAxis;
      /// The chunk size to apply
      int m_chunkSize;
      /// Whether to require the same group structure
      bool m_requireSameFormat;
      /// The size of the buffer
      std::size_t m_bufferSize;
      /// Whether to measure the buffer in bytes or rows
      bool m_measureBufferInRows;
      /// Accumulator for UHI histogram groups
      std::unique_ptr<H5Utils::hist::HistogramMerger> m_histMerger;
  }; //> end class Merger
} //> end namespace H5Utils

#endif //> !HDF5Utils_Merger_H
