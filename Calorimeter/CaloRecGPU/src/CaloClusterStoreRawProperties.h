//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCLUSTERSTORERAWPROPERTIES_H
#define CALORECGPU_CALOCLUSTERSTORERAWPROPERTIES_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"

/**
 * @class CaloClusterStoreRawProperties
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 February 2024
 * @brief Stores the current (calibrated) cluster energies and coordinates
 *        as raw properties.
 * 
 * @remark The standard CPU code does this in the cluster making algorithm,
 *         our GPU code optionally does this in the cluster importing tool,
 *         but instantiating the CPU code in a HybridClusterProcessor
 *         requires doing this if we want to run later stages of the reconstruction...
 */

class CaloClusterStoreRawProperties :
  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  CaloClusterStoreRawProperties(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual ~CaloClusterStoreRawProperties() = default;

};

#endif //CALORECGPU_CALOCPUOUTPUT_H
