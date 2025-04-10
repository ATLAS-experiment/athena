/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// GeneratorFilters/DecaysFinalStateFilter
//
// picks events with a given number of quarks, leptons and neutrinos from
// decays of a list of specified resonances (e.g. W, Z, ...)
//
// Authors:
// Kerim Suruliz Nov 2014
// Frank Siegert Nov 2014

#ifndef GENERATORFILTERSXAODDECAYSFINALSTATEFILTER_H
#define GENERATORFILTERSXAODDECAYSFINALSTATEFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

class xAODDecaysFinalStateFilter : public GenFilter {

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  // list of allowed resonances from which decay products are counted
  Gaudi::Property<std::vector<int>> m_PDGAllowedParents{this, "PDGAllowedParents", {}};

  // required (exact) number of quarks, electrons, muons, taus,
  // charged leptons (of any flavor), neutrinos and photons from decays
  Gaudi::Property<int> m_NQuarks{this, "NQuarks", -1};
  Gaudi::Property<int> m_NElectrons{this, "NElectrons", -1};
  Gaudi::Property<int> m_NMuons{this, "NMuons", -1};
  Gaudi::Property<int> m_NTaus{this, "NTaus", -1};
  Gaudi::Property<int> m_NChargedLeptons{this, "NChargedLeptons", -1};
  Gaudi::Property<int> m_NNeutrinos{this, "NNeutrinos", -1};
  Gaudi::Property<int> m_NPhotons{this, "NPhotons", -1};

  // required minimal number of quarks, electrons, muons, taus,
  // charged leptons (of any flavor), neutrinos and photons from decays
  Gaudi::Property<int> m_MinNQuarks{this, "MinNQuarks", 0};
  Gaudi::Property<int> m_MinNElectrons{this, "MinNElectrons", 0};
  Gaudi::Property<int> m_MinNMuons{this, "MinNMuons", 0};
  Gaudi::Property<int> m_MinNTaus{this, "MinNTaus", 0};
  Gaudi::Property<int> m_MinNChargedLeptons{this, "MinNChargedLeptons", 0};
  Gaudi::Property<int> m_MinNNeutrinos{this, "MinNNeutrinos", 0};
  Gaudi::Property<int> m_MinNPhotons{this, "MinNPhotons", 0};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

};

#endif
