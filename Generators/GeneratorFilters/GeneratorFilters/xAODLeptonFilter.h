/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// --------------------------------------------------
//
// File:  GeneratorFilters/LeptonFilter.h
// Description: Filters based on presence of charged leptons
//
// Authors:
//         I Hinchliffe:  December 2001
//         A Buckley:     April 2009

#ifndef GENERATORFILTERS_XAODLEPTONFILTER_H
#define GENERATORFILTERS_XAODLEPTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Filter events based on presence of charged leptons
class xAODLeptonFilter : public GenFilter {

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;


private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthElectronContKey{this, "TruthElectronContainerKey", "TruthElectrons"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonContKey{this, "TruthMuonContainerKey", "TruthMuons"};

  // Declare filter variables
  Gaudi::Property<double> m_Ptmin{this,"Ptcut",10000.0,"Minimum pT for a lepton to count"};
  Gaudi::Property<double> m_Ptmax{this,"PtcutMax",1e99,"Maximum pT for a lepton to veto the event"};
  Gaudi::Property<double> m_EtaRange{this,"Etacut",10.0,"Minimum |pseudorapidity| for a lepton to count"};

};

#endif
