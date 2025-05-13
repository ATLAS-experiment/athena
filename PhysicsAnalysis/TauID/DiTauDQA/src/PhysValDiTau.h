// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef DITAUDQA_PHYSVALDITAU_H
#define DITAUDQA_PHYSVALDITAU_H

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

// Local includes
#include "DiTauValidationPlots.h"

#include "xAODTau/DiTauJetContainer.h"

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
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();


private: 
  // properties
  Gaudi::Property<std::string> m_DiTauJetContainerName{this, "DiTauContainerName", "DiTauJets"};
  Gaudi::Property<bool> m_isMC{this, "isMC", false};

  //Histograms
  std::unique_ptr<DiTauValidationPlots> m_oDiTauValidationPlots;
  
}; 

#endif //> !DITAUDQA_PHYSVALDITAU_H
