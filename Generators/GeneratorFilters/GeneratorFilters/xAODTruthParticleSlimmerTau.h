/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERTAU_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERTAU_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ToolHandle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

#include <unordered_set>


/// @brief Algorithm to skim the xAOD truth particle container for tau filter 
///
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// keeping just relevant taus from the event.  
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
/// @author Jeff Dandoy <Jeff.Dandoy@cern.ch>
class xAODTruthParticleSlimmerTau : public AthAlgorithm {
public:

  /// Regular algorithm constructor
  xAODTruthParticleSlimmerTau( const std::string& name, ISvcLocator* svcLoc );
//  xAODTruthParticleSlimmerTau( const std::string& t, const std::string& n, const IInterface* p);
  /// Function initialising the algorithm
  virtual StatusCode initialize();
  /// Function executing the algorithm
  virtual StatusCode execute();

  CLHEP::HepLorentzVector sumDaughterNeutrinos( const xAOD::TruthParticle* tau );

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerName
  {this,"xAODTruthParticleContainerName","TruthParticles","Name of Truth Particle container"};
  /// The key for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_xaodTruthTauParticleContainerName
    {this, "xAODTruthTauParticleContainerName","TruthTaus","Name of Truth Taus contatiner from the slimmer"};

  /// Selection values for keeping taus and leptons
  DoubleProperty m_tau_pt_selection{this, "tau_pt_selection", 0.001 * Gaudi::Units::GeV}; //in GeV
  DoubleProperty m_abseta_selection{this, "abseta_selection", 10.};

  /// a flag to force rerunning (useful for rerunning on ESDs)
  BooleanProperty m_forceRerun{this, "ForceRerun", false};

  PublicToolHandle<IMCTruthClassifier> m_classifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};

}; // class xAODTruthParticleSlimmerTau



#endif // GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERTAU_H
