//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCPUOUTPUT_H
#define CALORECGPU_CALOCPUOUTPUT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include <string>

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

class CaloCell_ID;

/**
 * @class CaloCPUOutput
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 31 May 2022
 * @brief Standard tool to output CPU cluster info to the non-standard file format that
 * we have been using for plotting and validation purposes.
 *
 * There are likely more elegant/general/generic/portable solutions,
 * some of which might even avoid Root too, but our workflow was built around this one...
 */

class CaloCPUOutput :
  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  CaloCPUOutput(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual ~CaloCPUOutput();

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
   * @brief If @c true, also outputs the cell energies, times, gains, qualities and provenances.
   
     As the GPU output contains exactly the same information, this will be redundant and thus it is defaulted to @c false.
   */
  Gaudi::Property<bool> m_saveCellInfo{this, "AlsoOutputCellInfo", false,
                                       "Whether to output cell energies, gains, times, etc., as this information is repeated in the GPU side too."};
  
    /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};
  
  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id {nullptr};

};

#endif //CALORECGPU_CALOCPUOUTPUT_H
