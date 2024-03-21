//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOMOMENTSDUMPER_H
#define CALORECGPU_CALOMOMENTSDUMPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include <string>


class CaloCell_ID;

/**
 * @class CaloMomentsDumper
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 05 March 2023
 * @brief Outputs cluster moments (and other assorted properties) to text files.
 */

class CaloMomentsDumper :
  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  CaloMomentsDumper(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual ~CaloMomentsDumper() = default;

 private:


  /**
   * @brief The path specifying the folder to which the files should be saved.
     Default @p ./saved_clusters
   */
  Gaudi::Property<std::string> m_savePath{this, "SavePath", "./cluster_dump", "Path to where the files should be saved"};

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
  
};

#endif //CALORECGPU_CALOMOMENTSDUMPER_H
