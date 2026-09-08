// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef DITAUDQA_PHYSVALDITAU_H
#define DITAUDQA_PHYSVALDITAU_H

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "xAODTau/DiTauJetContainer.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TauAnalysisTools/IDiTauSelectionTool.h"
#include "TauAnalysisTools/IDiTauTruthMatchingTool.h"

// Local includes
#include "DiTauValidationPlots.h"

#include <memory>
#include <string>

class PhysValDiTau
  : public ManagedMonitorToolBase
{ 

public: 
  /// Constructor with parameters: 
  PhysValDiTau( const std::string& type,
	        const std::string& name, 
	        const IInterface* parent );

  // Athena algtool's Hooks
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms(const EventContext& ctx);
  virtual StatusCode procHistograms();


private: 
  // properties
  Gaudi::Property<bool> m_isMC{this, "isMC", false};

  ToolHandle<TauAnalysisTools::IDiTauSelectionTool> m_nomiDiTauSel{this, "NominalDiTauSelectionTool", "TauAnalysisTools::DiTauSelectionTool/NominalDiTauSelectionTool"};
  ToolHandle<TauAnalysisTools::IDiTauTruthMatchingTool> m_truthTool{this, "DiTauTruthMatchingTool", "TauAnalysisTools::DiTauTruthMatchingTool/DiTauTruthMatchingTool"};

  //container name
  SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditauContainerKey{this, "DiTauContainerName", "DiTauJets", "Input ditau container key" }; 

  // decoration name
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer> m_IsTruthHadronicKey{this, "IsTruthHadronicDecorKey", "IsTruthHadronic", "IsTruthHadronic decoration key"};

  //Histograms
  std::unique_ptr<DiTauValidationPlots> m_oDiTauValidationPlots;
  
}; 

#endif //> !DITAUDQA_PHYSVALDITAU_H
