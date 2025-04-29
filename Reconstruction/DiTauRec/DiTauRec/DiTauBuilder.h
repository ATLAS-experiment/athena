/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_DITAUBUILDER_H
#define DITAUREC_DITAUBUILDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODJet/JetContainer.h"

#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"


class DiTauBuilder: public ::AthReentrantAlgorithm { 
 public: 
  DiTauBuilder( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~DiTauBuilder(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext&) const override;
  virtual StatusCode  finalize() override;

 private: 
  // ditau output container name
  SG::WriteHandleKey<xAOD::DiTauJetContainer> m_diTauContainerName
    { this, "DiTauContainer", "DiTauJets", "" };
  // name for seed jet collection name
  SG::ReadHandleKey<xAOD::JetContainer> m_seedJetName
    { this, "SeedJetName", "AntiKt10LCTopoJets", "" };

  Gaudi::Property<float> m_minPt{this, "minPt", 10000};
  Gaudi::Property<float> m_maxEta{this, "maxEta", 2.5};
  Gaudi::Property<float> m_Rjet{this, "Rjet", 1.0};
  Gaudi::Property<float> m_Rsubjet{this, "Rsubjet", 0.2};
  Gaudi::Property<float> m_Rcore{this, "Rcore", 0.1};

  ToolHandleArray<DiTauToolBase> m_tools{this, "Tools", {}};

}; 

#endif //> !DITAUREC_DITAUBUILDER_H
