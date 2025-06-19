//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOGPUOUTPUT_H
#define CALORECGPU_CALOGPUOUTPUT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include <string>
#include <mutex>
#include <atomic>

/**
 * @class CaloGPUOutput
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 30 May 2022
 * @brief Standard tool to output the GPU data representation to the non-standard file format that
 * we have been using for plotting and validation purposes.
 *
 * There are likely more elegant/general/generic/portable solutions,
 * some of which might even avoid Root too, but our workflow was built around this one...
 */

class CaloGPUOutput :
  public extends<AthAlgTool, CaloClusterGPUProcessor>
{
 public:

  CaloGPUOutput(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual ~CaloGPUOutput() = default;

 private:


  /**
   * @brief The path specifying the folder to which the files should be saved.
     Default @p ./saved_clusters
   */
  Gaudi::Property<std::string> m_savePath{this, "SavePath", "./saved_clusters", "Path to where the files should be saved"};

  /**
   * @brief The prefix of the saved files. Empty string by default.
   */
  Gaudi::Property<std::string> m_filePrefix{this, "FilePrefix", "", "Prefix of the saved files"};

  /**
   * @brief The suffix of the saved files. Empty string by default.
   */
  Gaudi::Property<std::string> m_fileSuffix{this, "FileSuffix", "", "Suffix of the saved files"};

  /**
   * @brief The number of digits to reserve for the events. 9 by default.
   */
  Gaudi::Property<unsigned int> m_numWidth{this, "NumberWidth", 9, "The number of digits to reserve for the events"};

  /**
   * @brief If @p true, sort the clusters by transverse energy and compactify the tags to ensure sequentiality.
   */
  Gaudi::Property<bool> m_sortedAndCutClusters {this, "UseSortedAndCutClusters", true, "Sort the clusters by transverse energy, apply a cut and ensure contiguous tags"};

  /**
   * @brief If @p true, only output cell info (useful for reducing disk usage when running the full standalone version of the algorithms).
   */
  Gaudi::Property<bool> m_onlyCellInfo {this, "OnlyOutputCellInfo", false, "Only output cell info"};
  
  /**
   * @brief A flag to signal that the constant data has been adequately saved.
    *  This is required for everything to work properly in a multi-threaded context...
   */
  mutable std::atomic<bool> m_constantDataSaved;

  /** @brief This mutex is locked when saving the constant data on the first event
    * to ensure thread safety. Otherwise, it's unused.
    */
  mutable std::mutex m_mutex;

};

#endif //CALORECGPU_CALOGPUOUTPUT_H
