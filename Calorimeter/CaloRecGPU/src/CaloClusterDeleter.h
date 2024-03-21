//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCLUSTERDELETER_H
#define CALORECGPU_CALOCLUSTERDELETER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"

/**
 * @class CaloClusterDeleter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 02 June 2022
 * @brief Deletes clusters from the CPU collection. Used for testing and debugging purposes...
 *
 */

class CaloClusterDeleter :
  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  CaloClusterDeleter(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual ~CaloClusterDeleter() = default;

};

#endif //CALORECGPU_CALOCPUOUTPUT_H
