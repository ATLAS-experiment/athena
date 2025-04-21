// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TAUDQA_PHYSVALTAU_H
#define TAUDQA_PHYSVALTAU_H

// STL includes
#include <memory>
#include <string>
#include <vector>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"
#include "TauAnalysisTools/ITauSelectionTool.h"

// Local includes
#include "TauValidationPlots.h"

class PhysValTau
  : public ManagedMonitorToolBase
{ 

public: 
  /// Constructor with parameters: 
  PhysValTau( const std::string& type,
	      const std::string& name, 
	      const IInterface* parent );

  // Athena algtool's Hooks
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();


private: 
  // properties
  Gaudi::Property<std::string> m_TauJetContainerName{this, "TauContainerName", "TauJets"};
  Gaudi::Property<std::string> m_TruthParticleContainerName{this, "TruthParticleContainerName", "TruthParticles"}; 
  Gaudi::Property<bool> m_isMC{this, "isMC", false};

  // Tool used for truth-matching
  ToolHandle<TauAnalysisTools::ITauTruthMatchingTool> m_truthTool{this, "TauTruthMatchingTool", "TauAnalysisTools::TauTruthMatchingTool/TauTruthMatchingTool"};
  // Tool used to select "primitive" and "nominal" taus
  ToolHandle<TauAnalysisTools::ITauSelectionTool> m_primTauSel{this, "PrimitiveTauSelectionTool", "TauAnalysisTools::TauSelectionTool/PrimitiveTauSelectionTool"};
  ToolHandle<TauAnalysisTools::ITauSelectionTool> m_nomiTauSel{this, "NominalTauSelectionTool", "TauAnalysisTools::TauSelectionTool/NominalTauSelectionTool"};

  //Histograms
  // general tau all prongs plots
  std::unique_ptr<TauValidationPlots> m_oTauValidationPlots;
  
}; 

#endif //> !TAUDQA_PHYSVALTAU_H
