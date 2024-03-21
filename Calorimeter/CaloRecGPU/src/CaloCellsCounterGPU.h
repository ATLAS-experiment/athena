//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCELLSCOUNTERGPU_H
#define CALORECGPU_CALOCELLSCOUNTERGPU_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include <string>
#include <mutex>
#include <atomic>

/**
 * @class CaloCellsCounterGPU
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 22 July 2022
 * @brief Outputs counts of cells, both by type and cluster presence, with each cluster being identified by its cell with the largest signal-to-noise ratio.
 */

class CaloCellsCounterGPU :
  public AthAlgTool, virtual public CaloClusterGPUProcessor
{
 public:

  CaloCellsCounterGPU(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual ~CaloCellsCounterGPU() = default;

 private:


  /**
   * @brief The path specifying the folder to which the files should be saved.
     Default @p ./cell_counts
   */
  Gaudi::Property<std::string> m_savePath{this, "SavePath", "./cell_counts", "Path to where the files should be saved"};

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

  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_seedThreshold {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};

  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_growThreshold {this, "NeighborThresholdOnEorAbsEinSigma", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};

  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_cellThreshold {this, "CellThresholdOnEorAbsEinSigma", 0., "Cell (terminal) threshold (in units of noise Sigma)"};


};

#endif //CALORECGPU_CALOCELLSCOUNTERGPU_H