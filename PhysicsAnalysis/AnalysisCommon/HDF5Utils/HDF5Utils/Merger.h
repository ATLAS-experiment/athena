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
       * @brief Make a new group from information in a source group
       * @param targetLocation Where the new group will be created
       * @param source The group to use to create the new group
       */
      H5::Group createFrom(
          H5::H5Location& targetLocation,
          const H5::Group& source);

      /**
       * @brief Make a new dataset from information in a source dataset
       * @param targetLocation Where the new dataset will be created
       * @param source The dataset to use to create the new dataset
       */
      H5::DataSet createFrom(
          H5::H5Location& targetLocation,
          const H5::DataSet& source);

      /**
       * @brief Write all accumulated histogram data to the output.
       * @param dst The root group of the output file.
       */
      void flush(H5::Group& dst);

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
