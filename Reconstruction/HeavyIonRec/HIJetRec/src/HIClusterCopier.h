/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// HIClusterCopier.h

#ifndef __HIJETREC_HICLUSTERCOPIER_H__
#define __HIJETREC_HICLUSTERCOPIER_H__

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODCaloEvent/CaloClusterAuxContainer.h>
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class CaloCellContainer;

class HIClusterCopier : public AthReentrantAlgorithm
{

public:

  HIClusterCopier(const std::string& name, ISvcLocator* pSvcLocator);
  ~HIClusterCopier() {};

  virtual StatusCode initialize();
  virtual StatusCode execute(const EventContext &ctx) const;
  virtual StatusCode finalize();

private:
  /// \brief Name of input CaloClusterContainer, e.g HIClusters
  SG::ReadHandleKey<xAOD::CaloClusterContainer>   m_inputKey  { this, "InputContainerKey"   , "HIClusters"         , "Input Container Key" };
  /// \brief Name of output CaloClusterContainer, e.g. DFHIClusters
  SG::WriteHandleKey<xAOD::CaloClusterContainer>  m_outputKey { this, "OutputContainerKey"     , "DFHIClusters"       , "Output Container Key"};

};
#endif
