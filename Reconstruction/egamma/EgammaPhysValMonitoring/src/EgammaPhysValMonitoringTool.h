/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EgammaPhysValMonitoringTool.h 
// Header file for class EgammaPhysValMonitoringTool
// Author: 
/////////////////////////////////////////////////////////////////// 
#ifndef EGAMMAPHYSVALMONITORING_EGAMMAPHYVALMONITORINGTOOL_H
#define EGAMMAPHYSVALMONITORING_EGAMMAPHYVALMONITORINGTOOL_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"

// Root includes
#include "ElectronValidationPlots.h"
#include "LRTElectronValidationPlots.h"
#include "PhotonValidationPlots.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaAuxContainer.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "xAODEventInfo/EventInfo.h"

#include "StoreGate/ReadHandleKey.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "CxxUtils/checker_macros.h"

#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"

class IMCTruthClassifier;
namespace EgammaPhysValMonitoring {


class ATLAS_NOT_THREAD_SAFE EgammaPhysValMonitoringTool
  : public ManagedMonitorToolBase
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  EgammaPhysValMonitoringTool( const std::string& type,
		  const std::string& name, 
		  const IInterface* parent );

  /// Destructor: 
  virtual ~EgammaPhysValMonitoringTool() = default; 

  // Athena algtool's Hooks
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms(const EventContext& ctx);
  virtual StatusCode procHistograms();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  EgammaPhysValMonitoringTool() = delete;

  StatusCode fillRecoElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo);
  StatusCode fillRecoFrwdElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo);
  StatusCode fillRecoPhotHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo);
  StatusCode fillLRTElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo);

  static const xAOD::TruthParticle* Match(const xAOD::Egamma* particle, int pdg,
				   const xAOD::TruthParticleContainer* truthParticles) ;


  // Containers
  SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoContainerKey {this,
      "EventInfoContainerName", "EventInfo", "Input event information container"};
  SG::ReadHandleKey<xAOD::PhotonContainer> m_photonContainerKey {this,
      "PhotonContainerName", "Photons", "Input photon container"};
  SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainerKey {this,
      "ElectronContainerName", "Electrons", "Input electron container"};
  SG::ReadHandleKey<xAOD::ElectronContainer> m_lrtelectronContainerKey {this,
      "LRTElectronContainerName", "", "Input LRT electron container"};
  SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainerFrwdKey {this,
      "ElectronContainerFrwdName", "ForwardElectrons", "Input forward electron container"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerKey {this,
      "TruthParticleContainerName", "TruthParticles", "Input global truth particles"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_egammaTruthContainerKey {this,
      "EgammaTruthContainerName", "egammaTruthParticles", "Input egamma truth particles"};

  Gaudi::Property<bool> m_isMC {this, "isMC", false, "Should be set from file peeking"};

  // Hists
  ElectronValidationPlots m_oElectronValidationPlots;
  PhotonValidationPlots m_oPhotonValidationPlots;
  LRTElectronValidationPlots m_oLRTElectronValidationPlots;
  
  ToolHandle<IMCTruthClassifier>  m_truthClassifier {this,
      "MCTruthClassifier", "EMMCTruthClassifier", "Handle of MCTruthClassifier"};

  ToolHandle<IAsgElectronLikelihoodTool> m_Electron_VeryLooseNoPix_LLHTool{
      this,
      "ElectronLHSelectorVeryLooseNoPix", // Name of the configurable argument
      "AsgElectronLikelihoodTool/ElectronLHSelectorVeryLooseNoPix", // Default instance of the tool, of the form ToolClass/ToolName
      "Electron Likelihood Selector VeryLooseNoPix" // Description
  };

  ToolHandle<IAsgElectronLikelihoodTool> m_Electron_LooseNoPix_LLHTool{
      this,
      "ElectronLHSelectorLooseNoPix", // Name of the configurable argument
      "AsgElectronLikelihoodTool/ElectronLHSelectorLooseNoPix", // Default instance of the tool, of the form ToolClass/ToolName
      "Electron Likelihood Selector LooseNoPix" // Description
  };

  ToolHandle<IAsgElectronLikelihoodTool> m_Electron_MediumNoPix_LLHTool{
      this,
      "ElectronLHSelectorMediumNoPix", // Name of the configurable argument
      "AsgElectronLikelihoodTool/ElectronLHSelectorMediumNoPix", // Default instance of the tool, of the form ToolClass/ToolName
      "Electron Likelihood Selector Medium" // Description
  };

  ToolHandle<IAsgElectronLikelihoodTool> m_Electron_TightNoPix_LLHTool{
      this,
      "ElectronLHSelectorTightNoPix", // Name of the configurable argument
      "AsgElectronLikelihoodTool/ElectronLHSelectorTightNoPix", // Default instance of the tool, of the form ToolClass/ToolName
      "Electron Likelihood Selector Tight" // Description
  };

  // Aux accessor
  SG::AuxElement::ConstAccessor<char> m_acc_electronLLH_VeryLooseNoPix; // access LLH decision decorators
  SG::AuxElement::ConstAccessor<char> m_acc_electronLLH_LooseNoPix; // access LLH decision decorators
  SG::AuxElement::ConstAccessor<char> m_acc_electronLLH_MediumNoPix; // access LLH decision decorators
  SG::AuxElement::ConstAccessor<char> m_acc_electronLLH_TightNoPix; // access LLH decision decorators

  Gaudi::Property<bool> m_useOQQuality {this, "useOQQuality", false, "Use OQ quality cuts"};

}; 

}

#endif //> !EGAMMAPHYSVALMONITORING_EGAMMAPHYVALMONITORINGTOOL_H

